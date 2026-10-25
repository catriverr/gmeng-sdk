#pragma once
#include "../gmeng.h"
#include "graphics.h"
#include "kitty.h"

#include <cstdint>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>

/// Gmeng Splashscreen logo when the app is ran
/// via Kitty or a terminal that supports the Kitty
/// Graphics protocol. Since 13.5.2. Can be skipped
/// via `--debug-args`.

#define PL_MPEG_IMPLEMENTATION
#include "../../../include/pl_mpeg/pl_mpeg.h"

static Gmeng::term_graphics_renderer GM_SS_renderer;

#include <chrono>
#include <thread>


// The permanent, fixed name. Standard C++ only.
// It directly fills a static persistent buffer to stop heap allocation spam.
// Rigid Base64 helper with fixed name tracking
std::string strict_base64_encode(const std::string& in) {
    static const char lookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((in.size() + 2) / 3) * 4);
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(lookup[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(lookup[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
};

void draw_frame_kitty_gpu(const uint8_t* native_rgba_buffer, 
                          unsigned int current_width, 
                          unsigned int current_height,
                          unsigned int desired_width_px, 
                          unsigned int desired_height_px) 
{
    // 1. DYNAMIC GRID MATRIX CALCULATION
    // Fetch your absolute workspace monospace cell boundaries
    Objects::coord font = get_font_size_values();
    unsigned int cell_width_px = (font.x > 0) ? font.x : 10; 
    unsigned int cell_height_px = (font.y > 0) ? font.y : 20;

    struct winsize ws;
    // Query runtime display state context to guarantee perfect pixel mapping alignment
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_xpixel > 0 && ws.ws_ypixel > 0) {
        cell_width_px = ws.ws_xpixel / ws.ws_col;
        cell_height_px = ws.ws_ypixel / ws.ws_row;
    }

    // Convert requested raw pixel sizing down to matching layout text cells
    unsigned int target_cols = (desired_width_px + (cell_width_px / 2)) / cell_width_px;
    unsigned int target_rows = (desired_height_px + (cell_height_px / 2)) / cell_height_px;

    if (target_cols == 0) target_cols = 1;
    if (target_rows == 0) target_rows = 1;

    // 2. SHM LIFECYCLE MANAGEMENT
    // Unique name tracking handles Kitty's automatic shm_unlink garbage collection safely per frame
    static unsigned long long frame_counter = 0;
    std::string shm_name = "/kitty_vid_" + std::to_string(getpid()) + "_" + std::to_string(frame_counter++);
    
    size_t required_bytes = current_width * current_height * 4;

    int fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR | O_EXCL, 0666);
    if (fd == -1) return;

    if (ftruncate(fd, required_bytes) == -1) {
        close(fd);
        shm_unlink(shm_name.c_str());
        return;
    }

    void* ptr = mmap(nullptr, required_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        close(fd);
        shm_unlink(shm_name.c_str());
        return;
    }

    // Pump uncompressed pixel buffer into kernel shared space instantly via RAM-bus speeds
    std::memcpy(ptr, native_rgba_buffer, required_bytes);

    munmap(ptr, required_bytes);
    close(fd);

    std::string b64_payload = strict_base64_encode(shm_name);

    // 3. EXECUTE SAFE TERMINAL DRAW PLACEMENT
    // a=T -> Display and clear active frame cache instantly
    // t=s -> Shared memory pipeline tracking
    // s,v -> Stream buffer input resolution boundaries (1080p source properties)
    // c,r -> Dynamic hardware target grids (Upscales cleanly to target pixel boundaries on GPU)
    // C=1 -> Static cursor anchor lock layout execution
    // q=2 -> Complete silent mode bypass (Stops TTY data flooding loop feedbacks entirely)
    std::string transmit_header = "\033_Ga=T,t=s,q=2,C=1,f=32,i=1,s=" + std::to_string(current_width) + 
                                  ",v=" + std::to_string(current_height) + 
                                  ",c=" + std::to_string(target_cols) + 
                                  ",r=" + std::to_string(target_rows) + ";";
    std::string transmit_footer = "\033\\";

    // Pin layout to viewport root home to kill line fragmentation or scrolling artifacts
    std::cout << "\033[H" << transmit_header << b64_payload << transmit_footer << std::flush;
};


void on_video_frame(plm_t *mpeg, plm_frame_t *frame, void *user) {
    static std::tuple<int, int> v_size = kitty_get_terminal_size_in_pixels();

    unsigned int width = frame->width;
    unsigned int height = frame->height;

    GM_SS_renderer.width = std::get<0>(v_size);
    GM_SS_renderer.height = std::get<1>(v_size);

    size_t num_pixels = width * height;
    uint8_t* native_rgba_buffer = new uint8_t[width * height * 4]();

    // Decode to RGBA
    plm_frame_to_rgba(frame, native_rgba_buffer, width * 4);

    // Force alpha channel to fully opaque (255) for every pixel
    for (size_t i = 0; i < num_pixels; ++i) {
        native_rgba_buffer[(i * 4) + 3] = 255;
    }

    draw_frame_kitty_gpu( native_rgba_buffer, width, height, GM_SS_renderer.width, GM_SS_renderer.height );
    delete[] native_rgba_buffer;
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
