#pragma once
/// FOR GMENG_SDL.



#include <SDL2/SDL.h>
#if GMENG_SDLIMAGE
#include <SDL2/SDL_image.h>
#endif
#include <cstdint>

#include "../gmeng.h"
#include "../src/renderer.cpp"
#include "SDL_rect.h"
#include "SDL_ttf.h"

namespace Gmeng {
    /// (Gmeng) SDL Image
    struct sImage {
        int width, height;
        std::vector<uint32_t> content;
    };


    /// SDL Colors
    static SDL_Color scolors[9] = {
        {255,255,255  ,255},
        {131, 165, 152,255},
        {184, 187, 38 ,255},
        {134, 180, 117,255},
        {244, 73, 52  ,255},
        {211, 134, 155,255},
        {249, 188, 47 ,255},
        {40, 40, 40   ,255},
        {249, 128, 25 ,255},
    };
};

using Gmeng::sImage;

/// Streches an sImage object to a target width*height.
static sImage stretchImage(const sImage& src, int targetWidth, int targetHeight) {
    sImage result;
    result.width = targetWidth;
    result.height = targetHeight;
    result.content.resize(static_cast<size_t>(targetWidth) * targetHeight);

    if (src.width <= 0 || src.height <= 0 || src.content.size() < src.width * src.height) {
        // Invalid source image, return empty result
        return result;
    }

    for (int y = 0; y < targetHeight; ++y) {
        int srcY = y * src.height / targetHeight;
        if (srcY >= src.height) srcY = src.height - 1;

        for (int x = 0; x < targetWidth; ++x) {
            int srcX = x * src.width / targetWidth;
            if (srcX >= src.width) srcX = src.width - 1;

            size_t dstIdx = static_cast<size_t>(y) * targetWidth + x;
            size_t srcIdx = static_cast<size_t>(srcY) * src.width + srcX;

            result.content[dstIdx] = src.content[srcIdx];
        }
    }

    return result;
};

/// Returns a drawpoint of a source image given
/// a drawpoint from a stretched version of that image,
/// given the width & height values of both images.
static Gmeng::Renderer::drawpoint unscale_drawpoint(
    const Gmeng::Renderer::drawpoint& stretchedPoint,
    int sourceWidth, int sourceHeight,
    int targetWidth, int targetHeight)
{
    Gmeng::Renderer::drawpoint srcPoint;

    srcPoint.x = stretchedPoint.x * sourceWidth / targetWidth;
    if (srcPoint.x >= sourceWidth) srcPoint.x = sourceWidth - 1;

    srcPoint.y = stretchedPoint.y * sourceHeight / targetHeight;
    if (srcPoint.y >= sourceHeight) srcPoint.y = sourceHeight - 1;

    return srcPoint;
};

static uint32_t color_to_uint32(const SDL_Color& color) {
    // Pack as RGBA with R in highest byte, matching masks below
    return (color.r << 24) | (color.g << 16) | (color.b << 8) | (color.a);
};

Gmeng::color_t get_dominant_color(const Gmeng::color32_t& input_color) {
    using namespace Gmeng;
    static const struct { int r, g, b; } palette[8] = {
        { 255, 255, 255 }, // 0: WHITE
        { 0,   0,   255 }, // 1: BLUE
        { 0,   255, 0   }, // 2: GREEN
        { 0,   255, 255 }, // 3: CYAN
        { 255, 0,   0   }, // 4: RED
        { 255, 0,   255 }, // 5: PINK (Magenta)
        { 255, 255, 0   }, // 6: YELLOW
        { 0,   0,   0   }  // 7: BLACK
    };

    color_t closest_color = WHITE;

    // Initialize with a value larger than the maximum possible squared distance
    // (255^2 * 3 = 195075)
    int min_dist_sq = 200000;

    for (int i = 0; i < 8; ++i) {
        int dr = input_color.r - palette[i].r;
        int dg = input_color.g - palette[i].g;
        int db = input_color.b - palette[i].b;

        int dist_sq = (dr * dr) + (dg * dg) + (db * db);

        if (dist_sq < min_dist_sq) {
            min_dist_sq = dist_sq;
            closest_color = static_cast<color_t>(i);
        }
    }

    return closest_color;
}
/// Creates an SDL_Texture from Gmeng::sImage using proper packed pixel buffer
static SDL_Texture* make_texture(SDL_Renderer* renderer, const Gmeng::sImage& image) {
    uint32_t width = image.width;
    uint32_t height = image.height;

    std::vector<uint32_t> pixels;
    pixels.reserve(width * height);

    // Convert color indices to packed pixels
    for (auto colorIndex : image.content) {
        int filtered_index = 8; //orange
        if ( colorIndex > 8 ) filtered_index = get_dominant_color( Gmeng::color32_t( colorIndex ) );
        else filtered_index = colorIndex;

        SDL_Color color = Gmeng::scolors[ filtered_index ];
        pixels.push_back(color_to_uint32(color));
    }

    if (pixels.size() != width * height) {
        std::cerr << "Pixel count mismatch!" << std::endl;
        return nullptr;
    }

    // Create surface with masks matching the packing in color_to_uint32
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
        pixels.data(),
        width,
        height,
        32,                     // bits per pixel
        width * sizeof(uint32_t), // pitch
        0xFF000000,  // red mask (highest byte)
        0x00FF0000,  // green mask
        0x0000FF00,  // blue mask
        0x000000FF   // alpha mask (lowest byte)
    );

    if (!surface) {
        std::cerr << "Error creating surface: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        std::cerr << "Error creating texture: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    return texture;
}

#if GMENG_SDLIMAGE
/// (Gmeng) returns an SDL Texture from `.png` files.
static SDL_Texture* from_png(SDL_Renderer* renderer, const char* file_path) {
    __functree_call__(gmeng_external::__optional_utils__::libsdl2::from_png);
    // Load the PNG file into an SDL_Surface
    SDL_Surface* surface = IMG_Load(file_path);
    if (!surface) {
        std::cerr << "Error loading image: " << IMG_GetError() << std::endl;
        return nullptr;
    }

    // Create an SDL_Texture from the SDL_Surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Free the surface after creating the texture
    if (!texture) {
        std::cerr << "Error creating texture: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    return texture;
}

#endif

#define GMENG_SDL_ENABLED true
