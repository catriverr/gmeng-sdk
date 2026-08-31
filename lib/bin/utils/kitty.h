#pragma once

#include "global.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>


/// Creates a Kitty terminal config that is
/// compatible with Gmeng's requirements.
static void kitty_create_config(std::string fname) {
    std::ofstream f(fname);

    if (!f.is_open()) std::cerr << "error writing";
    else {
        f << "allow_remote_control yes\n";
        f << "font_size 18.0\n";

        f.close();
        std::cout << "written to " << fname;
    };

};

/// Kitty terminal : resizes the window in pixel format.
/// Requires an unknown amount of grace milliseconds.
static void kitty_resize_window_pixels(int width, int height) {
    std::system(("kitten @ resize-os-window --unit pixels --width " + v_str(width) + " --height " + v_str(height)).c_str());
};

/// Kitty terminal : resizes the window.
/// Requires an unknown amount of grace milliseconds.
static void kitty_resize_window(int width, int height, bool pixels = false) {
    if ( pixels ) { kitty_resize_window_pixels(width, height); return; };
    std::system(("kitten @ resize-os-window --width " + std::to_string(width) + " --height " + std::to_string(height)).c_str());
};


/// Kitty terminal : increments the window size by X,Y pixels.
/// Requires an unknown amount of grace milliseconds.
static void kitty_incrtl_window( int x, int y ) {
    std::system( ("kitten @ resize-os-window --incremental --unit pixels --width "$(x)" --height " + v_str(y)).c_str() );
};

/// Kitty terminal : sets font size.
static void kitty_set_font_size(int size) {
    std::system(("kitten @ set-font-size " + std::to_string(size)).c_str());
};

/// Kitty Terminal : Increments window size by Width, Height.
static void kitty_increment_window_size( int width = 0, int height = 0 ) {
    char* term = getenv("TERM");

    if ( term != nullptr && std::string(term) == "xterm-kitty" ) {
        kitty_incrtl_window( width, height );
    };
};
