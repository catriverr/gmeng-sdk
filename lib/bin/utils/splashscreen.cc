#pragma once
#include "../gmeng.h"
#include "graphics.h"


/// Gmeng Splashscreen logo when the app is ran
/// via Kitty or a terminal that supports the Kitty
/// Graphics protocol. Since 13.5.2. Can be skipped
/// via `--debug-args`.

#define PL_MPEG_IMPLEMENTATION
#include "../../../include/pl_mpeg/pl_mpeg.h"

static Gmeng::term_graphics_renderer GM_SS_renderer;

#include <chrono>
#include <thread>

void on_video_frame(plm_t *mpeg, plm_frame_t *frame, void *user) {
    unsigned int width = frame->width;
    unsigned int height = frame->height;

    GM_SS_renderer.width = width;
    GM_SS_renderer.height = height;

    size_t num_pixels = width * height;
    uint8_t* rgba_buffer = new uint8_t[num_pixels * 4];

    // Decode to RGBA
    plm_frame_to_rgba(frame, rgba_buffer, width * 4);

    // Force alpha channel to fully opaque (255) for every pixel
    for (size_t i = 0; i < num_pixels; ++i) {
        rgba_buffer[(i * 4) + 3] = 255;
    }

    GM_SS_renderer.present(rgba_buffer);
    delete[] rgba_buffer;
}
namespace Gmeng {
    namespace {
        bool splash_screen_shown = false;
    }

    void show_splash_screen(Gmeng::term_graphics_renderer& renderer) {
        GM_SS_renderer = renderer;
        plm_t *plm = plm_create_with_filename("assets/Gmeng.mpg");
        if (!plm) return;

        plm_set_video_decode_callback(plm, on_video_frame, nullptr);

        // Calculate actual frame time from the video metadata
        double fps = plm_get_framerate(plm);
        if (fps <= 0.0) fps = 30.0; // Fallback
        double frame_duration = 1.0 / fps;

        while (!plm->has_ended) {
            auto start_time = std::chrono::steady_clock::now();

            // Decode one frame's worth of time
            plm_decode(plm, frame_duration);

            auto end_time = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = end_time - start_time;

            // Sleep for the remainder of the frame time so it plays in real-time
            if (elapsed.count() < frame_duration) {
                std::this_thread::sleep_for(
                    std::chrono::duration<double>(frame_duration - elapsed.count())
                );
            }
        }

        plm_destroy(plm);
    };
};
