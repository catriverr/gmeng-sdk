#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <termios.h>
#include <cstdio>
#include <cstdlib>
#include <sys/ioctl.h>

#include "global.h"
#include "ioctl.h"
#include "log.h"
#include "../src/objects.cpp"

/// default termios state. Used to disable rawmode.
static struct termios orig_termios;

static int FONT_SIZE_SET_IN_GMENG_KITTY_CONFIG = 18;

/// Returns the width and height of a single character
/// using the current font in pixel size.
static Objects::coord get_font_size_values() {
    struct winsize w;
    // Query the terminal window metrics
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        // w.ws_xpixel is total window width in pixels
        // w.ws_col    is total number of character columns
        int charWidth = w.ws_xpixel / w.ws_col;
        int charHeight = w.ws_ypixel / w.ws_row;

        return { charWidth, charHeight };
    } else return {
        /// ioctl returned an error ( literally how )
        /// default to (and assume) 2:1 ratio for the font
        /// this is almost always going to be wrong, especially
        /// for kitty's default font. :):)
        FONT_SIZE_SET_IN_GMENG_KITTY_CONFIG,
        FONT_SIZE_SET_IN_GMENG_KITTY_CONFIG*2
    };
};

static Objects::coord get_terminal_size_in_pixels() {
    Objects::coord font_size_values = get_font_size_values();
    TerminalSize size_row_col = get_terminal_size();

    return {
        size_row_col.width  * font_size_values.x,
        size_row_col.height * font_size_values.y
    };
};

/// Disables SGR-Pixel rawmode by setting the
/// termios to the default state.
///
/// SGR-Pixel returns the pixel position of the mouse
/// within the Terminal, allowing for correct mouse clicks
/// in Gmeng's cubic_render mode, because it halves screen height.
static void sgr_pixel_disable_rawmode() {
    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

/// Enables SGR-Pixel rawmode by setting the
/// termios to the default state.
///
/// SGR-Pixel returns the pixel position of the mouse
/// within the Terminal, allowing for correct mouse clicks
/// in Gmeng's cubic_render mode, because it halves screen height.
static void sgr_pixel_enable_rawmode(bool state = true) {
    if (state == false) {
        sgr_pixel_disable_rawmode();
        return;
    };
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(sgr_pixel_disable_rawmode); // Ensure we reset the terminal on exit

    struct termios raw = orig_termios;
    // Disable echoing of characters and line-by-line buffering
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


/// Enables SGR-Pixel mosue reporting
/// (chcp \033[?1016h escape code.)
static void sgr_pixel_enablemouse() {
    // ?1004h : Focus tracking (Leaves window, enters window)
    // ?1003h : Any-Event Mouse Tracking (Clicks, Releases, Movement)
    // ?1016h : SGR-Pixel Mode (Outputs raw pixels instead of rows/cols)
    std::cout << "\033[?1004h\033[?1003h\033[?1016h" << std::flush;
}

/// Disables SGR-Pixel mosue reporting
/// (chcp \033[?1003l \033[?1016l escape code.)
static void sgr_pixel_disablemouse() {
    // Turn all modes off
    char msg[] = "\033[?1004l\033[?1003l\033[?1016l";
    write(STDOUT_FILENO, msg, sizeof(msg)-1);
    fflush(stdout);
}


/// Checks if the current terminal supports SGR-Pixel rawmode.
/// Requires the user to move their mouse to function. Not recommended to use.
///
/// SGR-Pixel returns the pixel position of the mouse
/// within the Terminal, allowing for correct mouse clicks
/// in Gmeng's cubic_render mode, because it halves screen height.
static bool sgr_pixel_checksupport() {

    char* term = getenv("TERM");
    char* term_prog = getenv("TERM_PROGRAM");

    if ( term      != nullptr && std::string(term)      == "xterm-kitty" ) return true;
    if ( term_prog != nullptr && std::string(term_prog) == "Apple_Terminal" ) return false;

    auto tsize = get_terminal_size();

    printf("\033c");
    std::cout << "\033[H";

    std::string text = (std::string)"\033[1m\x1B[36m(gmeng-engine-calibration)\033[0m\n" +
                       "Gmeng (the engine that runs this game) cannot identify your terminal.\n" +
                       "This is required to adjust specific functions of the engine.\n" +
                       "Otherwise, this game will not function properly.\n\n" +
                       "\033[1mPress the [enter] key on your keyboard to continue.\033[0m\n" +
                       "the input will be analyzed to check support for various\n" +
                       "functions provided by the terminal/shell.";

    auto spl = g_splitStr(text, "\n");

    for ( auto& v : spl )
        centertext(v);

    for ( int i = 0; i < tsize.width; i++ )
        std::cout << "\u2588";

    for ( auto s : spl )
        std::cout << s << ( s.length() > tsize.width ? "\n" : "" );

    for ( int i = 0; i < tsize.width; i++ )
        std::cout << "\u2588";

    // Send the DEC mode request for 1016
    std::cout << "\033[?1016$p" << std::flush;

    char buf[32];
    int i = 0;
    char c;

    while (read(STDIN_FILENO, &c, 1) == 1) {
        buf[i++] = c;
        if (c == 'y') {
            buf[i] = '\0';
            break;
        }
        if (i >= 31) break;
    }

    // Parse response format: \033[?1016;Ps$y
    int mode, status;
    if (std::sscanf(buf, "\033[?%d;%d$y", &mode, &status) == 2) {
        if (mode == 1016) {
            // Status 1 or 2 or 3 means it's supported by the terminal
            return (status > 0);
        }
    }
    return false;
}

static int attempt_set_terminal_sgr_support(bool& checker) {
    gm_log("checking SGR-Pixel support for pixel-based mouse reporting...");
    Gmeng::global.terminal_has_sgr_pixel_support = sgr_pixel_checksupport();

    if ( Gmeng::global.terminal_has_sgr_pixel_support )
        gm_log("terminal has SGR-Pixel support, mouse reporting will be fully accurate");
    else
        gm_log("terminal does not have SGR-Pixel support, mouse reporting will not be accurate!");

    checker = true;
    return 0;
};

