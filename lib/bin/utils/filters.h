#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <stdint.h>
#include "../src/objects.cpp"
#include "subset.h"
#include <omp.h>
#include <vector>

enum SIDE {
    TOP,    // Fades downwards from the top edge
    BOTTOM, // Fades upwards from the bottom edge
    LEFT,   // Fades rightwards from the left edge
    RIGHT,  // Fades leftwards from the right edge
};

/// shenanigans. When used with a backdrop with
/// extend_outwards = true, the filters that
/// follow the method is broken. I don't understand
/// it.
inline void dropshadow(subarray_2d<uint8_t>& img,
                       const Gmeng::Renderer::drawpoint& size,
                       const Gmeng::Renderer::viewpoint& face,
                       uint32_t color = 0x202020,
                       float opacity = 0.5f,
                       size_t offset_x = 1,
                       size_t offset_y = 1) {

    if ( !img.exists() ) return;

    opacity = std::clamp(opacity, 0.0f, 1.0f);

    size_t width = size.x;
    size_t height = size.y;

    int face_width = face.end.x - face.start.x;
    int face_height = face.end.y - face.start.y;

    float opacity_reversed = 1.0f - opacity;

    uint8_t C_R = ( color >> 16 ) & 0xFF,
            C_G = ( color >>  8 ) & 0xFF,
            C_B = ( color       ) & 0xFF;

    float opacity_delta_x = opacity / (float)offset_x;
    float opacity_delta_y = opacity / (float)offset_y;

    #pragma omp parallel for schedule(static)
    for ( int i = 0; i < height+offset_y; i++ ) {

        /// TOP and BOTTOM
        if ( i < offset_y || (height+offset_y) - i <= offset_y) {
            unsigned int depth = i < offset_y ? i : (height+offset_y) - i;

            for ( int j = 0; j < width; j++ ) {
                int pixel_start = (((i + offset_y*3) * width) + j) * 4;
                if ( pixel_start > img.view_size ) continue;

            float d_opacity = std::min(( depth * opacity_delta_y ), opacity);                

                if ( j < offset_x || j + offset_x >= width ) {
                    int d = j < offset_x ? j : width - j;
                    float factorx = std::max(0.0f, std::min(1.0f, d / (float)offset_x));
                    float factory = std::max(0.0f, std::min(1.0f, depth / (float)offset_y));
                    d_opacity = factorx * factory * opacity;
                };

            float d_opacity_rev = (1.0f - d_opacity);

              img[ pixel_start + 0 ] =
                (( C_R * d_opacity ) + ( d_opacity_rev * img[ pixel_start + 0 ] ));
              img[ pixel_start + 1 ] =
                (( C_G * d_opacity ) + ( d_opacity_rev * img[ pixel_start + 1 ] ));
              img[ pixel_start + 2 ] =
                (( C_B * d_opacity ) + ( d_opacity_rev * img[ pixel_start + 2 ] ));
            };
            continue;
        };

        for ( int depth = 0; depth < offset_x; depth++ ) {
            /// linear alpha blending: 
            /// result = a x new_color_channel + ( 1 - a ) x old_color_channel
            do { /// LEFT side
                int pixel_start = (((i + offset_y*3) * width) + depth) * 4;
                if ( pixel_start > img.view_size ) continue;

                const Gmeng::Renderer::drawpoint p = img.point( pixel_start );
                //if ( p.x < face.start.x ) {

                    float d_opacity = std::min(( depth * opacity_delta_x ), opacity);
                    float d_opacity_rev = (1.0f - d_opacity);

                    img[ pixel_start + 0 ] =
                        (( C_R * d_opacity ) + ( d_opacity_rev * img[ pixel_start + 0 ] ));
                    img[ pixel_start + 1 ] =
                        (( C_G * d_opacity ) + ( d_opacity_rev * img[ pixel_start + 1 ] ));
                    img[ pixel_start + 2 ] =
                        (( C_B * d_opacity ) + ( d_opacity_rev * img[ pixel_start + 2 ] ));
                //};

            } while(0);
            do { /// RIGHT side
                int pixel_start = (( (i+(offset_y*3)+1) * width ) - 1 - depth) * 4;
                if ( pixel_start > img.view_size ) continue;

                const Gmeng::Renderer::drawpoint p = img.point( pixel_start );
                //if ( p.x > face.end.x ) {
                    float d_opacity = std::min(( depth * opacity_delta_x ), opacity);                
                    float d_opacity_rev = (1.0f - d_opacity);

                    img[ pixel_start + 0 ] =
                      (( C_R * d_opacity ) + ( d_opacity_rev * img[ pixel_start + 0 ] ));
                    img[ pixel_start + 1 ] =
                      (( C_G * d_opacity ) + ( d_opacity_rev * img[ pixel_start + 1 ] ));
                    img[ pixel_start + 2 ] =
                      (( C_B * d_opacity ) + ( d_opacity_rev * img[ pixel_start + 2 ] ));                
                //};
            } while(0);
        };
    };
};

inline void gaussian_blur(subarray_2d<uint8_t>& img, const Gmeng::Renderer::drawpoint& img_size, 
                          const Objects::coord& position,
                          const Gmeng::Renderer::drawpoint& box_size,
                          float radius = 1.0f) {

    int x_end = position.x + box_size.x;
    int y_end = position.y + box_size.y;
    int x_start = position.x;
    int y_start = position.y;
    int width = img_size.x, height = img_size.y;

    // Boundary safety constraints
    if (x_end > width) x_end = width;
    if (y_end > height) y_end = height;
    if (x_start < 0) x_start = 0;
    if (y_start < 0) y_start = 0;

    int total_elements = width * height * 4;
    
    // Allocate pointer array and raw backing buffer for temporary image
    uint8_t* temp_img = (uint8_t*)malloc(total_elements * sizeof(uint8_t));
    if (temp_img == NULL) {
        free(temp_img);
        return;
    }

    // First pass: Horizontal blur
    #pragma omp parallel for collapse(2) schedule(static)
    for (int y = y_start; y < y_end; y++) {
        for (int x = x_start; x < x_end; x++) {
            int sum_r = 0, sum_g = 0, sum_b = 0;
            int count = 0;

            for (int kx = -radius; kx <= radius; kx++) {
                int nx = x + kx;
                if (nx >= x_start && nx < x_end) {
                    int idx = (y * width + nx) * 4;
                    sum_r += (img)[idx + 0];
                    sum_g += (img)[idx + 1];
                    sum_b += (img)[idx + 2];
                    count++;
                }
            }

            int out_idx = (y * width + x) * 4;
            temp_img[out_idx + 0] = (uint8_t)(sum_r / count);
            temp_img[out_idx + 1] = (uint8_t)(sum_g / count);
            temp_img[out_idx + 2] = (uint8_t)(sum_b / count);
            temp_img[out_idx + 3] = 255; // Keep alpha
        }
    }

    // Second pass: Vertical blur
    #pragma omp parallel for collapse(2) schedule(static)
    for (int y = y_start; y < y_end; y++) {
        for (int x = x_start; x < x_end; x++) {
            int sum_r = 0, sum_g = 0, sum_b = 0;
            int count = 0;

            for (int ky = -radius; ky <= radius; ky++) {
                int ny = y + ky;
                if (ny >= y_start && ny < y_end) {
                    int idx = (ny * width + x) * 4;
                    sum_r += temp_img[idx + 0];
                    sum_g += temp_img[idx + 1];
                    sum_b += temp_img[idx + 2];
                    count++;
                }
            }

            int out_idx = (y * width + x) * 4;
            (img)[out_idx + 0] = (uint8_t)(sum_r / count);
            (img)[out_idx + 1] = (uint8_t)(sum_g / count);
            (img)[out_idx + 2] = (uint8_t)(sum_b / count);
        }
    }

    free(temp_img);
};


inline void blend( subarray_2d<uint8_t>& src1, const uint8_t* src2, int width, int height,
                          SIDE direction, float t, float startPosition) {

    t = std::clamp(t, 0.0f, 1.0f);
    startPosition = std::clamp(startPosition, 0.0f, 1.0f);
    const float feather = 0.15f; 

    // 1. Calculate invariant wavefront once, outside the loops!
    float wavefront = startPosition + (t * (1.0f - startPosition + feather));

    // 2. Parallelize across all CPU cores using collapse(2) and static scheduling
    #pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 4;

            // 3. Calculate normalized coordinate based on direction
            float pos = 0.0f;
            switch (direction) {
                case SIDE::TOP:
                    pos = static_cast<float>(y) / height;
                    break;
                case SIDE::BOTTOM:
                    pos = static_cast<float>(height - 1 - y) / height;
                    break;
                case SIDE::LEFT:
                    pos = static_cast<float>(x) / width;
                    break;
                case SIDE::RIGHT:
                    pos = static_cast<float>(width - 1 - x) / width;
                    break;
            }

            // 4. Compute local blend weight
            float localT = 0.0f;
            if (pos >= wavefront) {
                localT = 0.0f; 
            } else if (pos <= wavefront - feather) {
                localT = 1.0f; 
            } else {
                localT = (wavefront - pos) / feather;
            }

            if (localT <= 0.0f) continue;

            // 5. Direct flat buffer indexing (no pointer arrays)
            float invLocalT = 1.0f - localT;
            for (int c = 0; c < 4; ++c) {
                float blended = (static_cast<float>((src1)[idx + c]) * invLocalT) + 
                                (static_cast<float>(src2[idx + c]) * localT);
                (src1)[idx + c] = static_cast<uint8_t>(blended + 0.5f);
            }
        }
    }
}
