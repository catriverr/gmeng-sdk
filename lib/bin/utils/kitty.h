#pragma once

#include "global.h"
#include "log.h"
#include "tracefunc.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>

inline std::string execute_command(const char* cmd) {

    std::array<char, 128> buffer;
    std::string result;
    
    // Open pipe for reading command output
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed to start the process!");
    }

    // Read the output chunk by chunk into the buffer
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    // Close the pipe and check status
    int returnCode = pclose(pipe);
    if (returnCode != 0) {
        // Optional: handle non-zero exit codes if kitty isn't installed or fails
    }

    return result;
}


/// Kitty Terminal : Returns the size of the terminal in pixels.
static std::tuple<int, int> kitty_get_terminal_size_in_pixels() {
    if (!Gmeng::global.window_control) return {3014 , 1886}; 

    std::string result = execute_command("kitty +kitten icat --print-window-size");

    auto str = g_splitStr(result, "x");

    if ( str.size() < 2 ) throw std::runtime_error("kitty did not return expected output");

    return { std::stoi(str[0]), std::stoi(str[1]) };
};

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

/// Kitty Terminal : Checks if the application is focused.
static int kitty_check_focus(bool set_global = true, TRACEFUNC) {
    if (!Gmeng::global.window_control) {
        if ( set_global ) Gmeng::global.app_focused = true;
        return true;
    }
    // Executes the kitten command and filters for the focused state
    // It returns 0 (true in shell) if found, or non-zero if not found.
    int status = system("kitten @ ls | grep -q '\"is_focused\": true'");

    if ( set_global )
        Gmeng::global.app_focused = status == 0;
    return status == 0;
};

/// Kitty Terminal : toggles fullscreen mode.
static void kitty_toggle_fullscreen() {
    std::string stateJson = execute_command("kitten @ ls");
    if (!stateJson.empty() && stateJson.find("\"is_fullscreen\": true") == std::string::npos) {
        if (Gmeng::global.window_control) std::system("kitten @ resize-os-window --action toggle-fullscreen");
    }
};

static void kitty_close_window() {
    if (!Gmeng::global.window_control) return;
    std::system(("kitten @ close-window"));
};

/// Kitty terminal : resizes the window in pixel format.
/// Requires an unknown amount of grace milliseconds.
static void kitty_resize_window_pixels(int width, int height) {
    if (!Gmeng::global.window_control) return;
    std::system(("kitten @ resize-os-window --unit pixels --width " + v_str(width) + " --height " + v_str(height)).c_str());
};

/// Kitty terminal : resizes the window.
/// Requires an unknown amount of grace milliseconds.
static void kitty_resize_window(int width, int height, bool pixels = false) {
    if (!Gmeng::global.window_control) return;
    if ( pixels ) { kitty_resize_window_pixels(width, height); return; };
    std::system(("kitten @ resize-os-window --width " + std::to_string(width) + " --height " + std::to_string(height)).c_str());
};


/// Kitty terminal : increments the window size by X,Y pixels.
/// Requires an unknown amount of grace milliseconds.
static void kitty_incrtl_window( int x, int y ) {
    if ( Gmeng::global.window_control ) std::system( ("kitten @ resize-os-window --incremental --unit pixels --width "$(x)" --height " + v_str(y)).c_str() );
};

/// Kitty terminal : sets font size.
static void kitty_set_font_size(int size) {
    if ( Gmeng::global.window_control) std::system(("kitten @ set-font-size " + std::to_string(size)).c_str());
};

/// Kitty Terminal : Increments window size by Width, Height.
static void kitty_increment_window_size( int width = 0, int height = 0 ) {
    if (!Gmeng::global.window_control) return;
    char* term = getenv("TERM");

    if ( term != nullptr && std::string(term) == "xterm-kitty" ) {
        kitty_incrtl_window( width, height );
    };
};
