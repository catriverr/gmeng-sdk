#pragma once
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "log.h"
#include "global.h"
#include "setup_class.h"

inline std::string base64_encode(const std::string& in) {
    static const char b64_table[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string out;
    out.reserve(((in.size() + 2) / 3) * 4);

    int val = 0;
    int valb = -6;

    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(b64_table[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }

    if (valb > -6) {
        out.push_back(b64_table[((val << 8) >> (valb + 8)) & 0x3F]);
    }

    while (out.size() % 4) {
        out.push_back('='); // Padding
    }

    return out;
}

namespace Gmeng {
/// @since Gmeng 13.4.0-d: Terminal Graphics Renderer impementation.
/// Provides functionality to (very efficiently) draw raw framebuffers
/// of RGB pixels to the terminal screen.
///
/// This utility is built around the `Kitty Terminal Graphics Protocol`,
/// which provides the API for drawing pixels within terminals that have
/// implemented the protocol. However, Gmeng prefers the `Kitty` terminal
/// for this functionality as it's the most efficient; and, obviously, the
/// best implementation of the protocol out of the batch that supports it.
class term_graphics_renderer {
  public:
    /// Width of the renderer (in pixels, not units). This value is used for
    /// calculating the Shared Memory Size which contains the framebuffer.
    int width = 0;
    /// Height of the renderer (in pixels, not units). This value is used for
    /// calculating the Shared Memory Size which contains the framebuffer.
    int height = 0;
    /// Frame index, used for selecting the correct SHM file descriptor
    /// from term_renderer::shm_names which is more efficient than using
    /// a single object and constantly mutating it and sending update headers.
    uint64_t frame_index = 0;

    GMENG_INIT_TYPE( width, height, frame_index );

    /// Default constructor
    term_graphics_renderer() = default;
    /// Constructor with the width and height of the framebuffer. If the image's
    /// width and height changes (usually means that the Level::Display::viewpoint
    /// was adjusted), the changes must be reflected to this object's width & height.
    term_graphics_renderer(int w, int h) : width(w), height(h), frame_index(0) {
        gm_log("term_renderer instance created with "$(w)" width and"$(h)" height");
    }

    ~term_graphics_renderer() {
        /// Clean up all SHM objects on exit
        for (int i = 0; i < 10; ++i) {
            if (frame_index >= i) {
                char buf[64];
                snprintf(buf, sizeof(buf), "/gshm_%llu", (unsigned long long)(frame_index - i));
                shm_unlink(buf);
            }
        }
    };

    /// Present a framebuffer to the screen. The framebuffer in this case is
    /// almost always a vector<Unit> containing data for the next frame. Since
    /// a Unit object is much larger in size than a uint8_t object, by default
    /// `get_lvl_view` will convert and add the color parameter of any Unit object
    /// it parses over into `(uint8_t*)Camera::display_map::framebuffer`.
    void present(const uint8_t* framebuffer) {
        /// Don't handle broken width/height values
        if ( width <= 0 || height <= 0 ) return;

        /// Memory size times 4 since that's the size of a uint8_t object
        size_t shm_size = width * height * 4;

        char shm_name[64];
        snprintf(shm_name, sizeof(shm_name), "/gshm_%llu", (unsigned long long)frame_index);
        std::string shm_name_b64 = base64_encode(shm_name);

        if (frame_index >= 10) {
            char old_name[64];
            snprintf(old_name, sizeof(old_name), "/gshm_%llu", (unsigned long long)(frame_index - 10));
            shm_unlink(old_name);
        }

        /// Create shared memory for this framebuffer
        int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0600);
        if (shm_fd == -1) {
            perror("shm_open failed");
            return;
        }

        /// Size, map, write, unmap
        ftruncate(shm_fd, shm_size);
        void* shm_ptr = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

        if (shm_ptr != MAP_FAILED) {
            memcpy(shm_ptr, framebuffer, shm_size);
            munmap(shm_ptr, shm_size);
        }
        close(shm_fd);

        /// tell kitty to render
        char escape_seq[256];

        /// \x1b[H  : Move cursor home so it overwrites the exact same screen area
        /// a=T     : Transmit & Display
        /// i=1     : Target Image ID 1 (replaces the existing image seamlessly)
        /// q=2     : Quiet mode (suppress terminal error text just in case)
        int seq_len = snprintf(escape_seq, sizeof(escape_seq),
            "\x1b[H\x1b_Ga=T,i=1,q=2,f=32,s=%d,v=%d,t=s;%s\x1b\\",
            width, height, shm_name_b64.c_str());

        write(STDOUT_FILENO, escape_seq, seq_len);
        frame_index++;
    }
};
};

#include "../strings/contains.cc"

static void gmeng_terminal_graphics_init() {
    char* TERM = getenv("TERM");
    char* TERM_PROG = getenv("TERM_PROGRAM");

    if ( TERM != nullptr && contains( TERM, "kitty" ) )
        Gmeng::global.terminal_has_kgp_pixel_support = true;
    else Gmeng::global.terminal_has_kgp_pixel_support = false;
};
