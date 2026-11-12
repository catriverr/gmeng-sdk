#pragma once

#include "global.h"
#include "sgr_pixel.h"
#include <sys/ioctl.h>
#include <fcntl.h>

/// Terminal Utility for raw mode, text drawing, mouse tracking, etc
namespace Gmeng::TerminalUtil {
    inline void enable_mouse_tracking()  {
        if ( Gmeng::global.terminal_has_sgr_pixel_support )
            sgr_pixel_enablemouse();
        else // default fallback for 1006h mode (no pixels, inaccurate)
            std::cout << "\033[?1006h\033[?1003h\n" << std::flush;
    };
    inline void disable_mouse_tracking() {
        if ( Gmeng::global.terminal_has_sgr_pixel_support )
            sgr_pixel_disablemouse();
        else { // default fallback for 1006h mode (no pixels, inaccurate)
            char sequence[] = "\033[?1006l\033[?1003l\n";
            write(STDOUT_FILENO, sequence, sizeof(sequence)-1);
            fflush(stdout);
        };
    };

    inline struct termios orig_termios;
    inline bool already_set = false;

    inline void cleanup() {
        disable_mouse_tracking();
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    };

    inline void set_raw_mode(bool state) {
        if ( Gmeng::global.terminal_has_sgr_pixel_support ) {
            sgr_pixel_enable_rawmode( state );
            return;
        };

        if (state) {
            std::atexit(cleanup);
            if (already_set) return;
            already_set = true;
            struct termios t;
            tcgetattr(STDIN_FILENO, &t);
            orig_termios = t;
            t.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &t);
        } else {
            already_set = false;
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        };
    }

    // not a function that can be disabled.
    // I tried.
    inline void set_non_blocking(bool state) {
        if (state) {
            int flags = fcntl(STDIN_FILENO, F_GETFL);
            flags |= O_NONBLOCK;
            fcntl(STDIN_FILENO, F_SETFL, flags);
        } else {
            int flags = fcntl(STDIN_FILENO, F_GETFL);
            flags &= ~O_NONBLOCK;
            fcntl(STDIN_FILENO, F_SETFL, flags);
        };
    };
};
