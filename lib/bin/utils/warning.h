#pragma once

#include <iostream>
#include <functional>
#include <map>
#include <stdexcept>
#include <vector>
#include <string>

#include "../types/color32.h"

#include "tracefunc.h"
#include "functree.h"
#include "ioctl.h"
#include "log.h"

static std::map<std::string, std::function<void()>> gmeng_warnings =
{
    { "bad_terminal", []() {
        /// TMUX slows down input handling
        /// and writing to stdout extremely.
        /// show warning against this.
        /// Do not modify this warning.
        std::cout << Gmeng::colors[Gmeng::YELLOW] << "WARNING!" << Gmeng::resetcolor;
        std::cout << " " << "Gmeng (the engine this game runs on) has identified your terminal program/shell to be incompatible.\n"
                  <<        "While this will not cause crashes, it will almost certainly cause display issues and bad input processing.\n\n";
        std::cout << "\nTMUX is" << Gmeng::boldcolor << " ABSOLUTELY NOT RECOMMENDED " << Gmeng::resetcolor;
        std::cout << "as it slows down input receiver signals, introduces massive input lag,\n";
        std::cout << "is unable to handle multiple keypresses at once";
        std::cout << " and slows down output writing by up to 5 times.\n\n";
        std::cout << "If your terminal depends on tmux for color output,";
        std::cout << " do not run this game with it.\n\n";
        std::cout << "Recommended Terminal Programs:";
        std::cout << "\n- windows: Windows Terminal\n";
        std::cout << "- macOS: Kitty\n";
        std::cout << "- linux / Windows Subsystem for linux: the default tty will suffice.\n";
        std::cout << "\nPress CTRL+C to quit.\n";
        std::cout << "\nPress [enter] to continue anyway.";
        std::cin.get();
    } },
    { "windows", []() {
        std::cout << Gmeng::colors[Gmeng::RED] << "ERROR! " << Gmeng::resetcolor << "Gmeng (the engine this game runs on) has identified your OS as WINDOWS." << '\n';
        std::cout << Gmeng::colors[Gmeng::RED] << "ERROR! " << Gmeng::resetcolor << Gmeng::boldcolor << "This error is raised by the engine itself, not the game." << Gmeng::resetcolor << '\n';
        std::cout << Gmeng::colors[Gmeng::RED] << "ERROR! " << Gmeng::resetcolor << "Currently, Gmeng only supports unix based operating systems. If you must use windows, install WSL. https://aka.ms/WSL" << '\n';
        std::cout << '\n';
        std::cout << Gmeng::colors[Gmeng::BLUE] << "ONGOING EFFORTS: PORTING GMENG TO WINDOWS :: " << Gmeng::colors[Gmeng::CYAN] << Gmeng::boldcolor << "https://github.com/catriverr/gmeng-sdk https://gmeng.org\n";
        std::cout << Gmeng::resetcolor << "Contribute to the project.\n";
    } },

    { "port", []() {
        std::cout << Gmeng::colors[Gmeng::RED] << "FAIL!" << Gmeng::resetcolor;
        std::cout << " "  << "Gmeng (the engine this game runs on) has identified an external error in this game instance.\n"
                  << "\n" << Gmeng::boldcolor << "This game (or the engine) has attempted to run the NOBLE script for client-side server instance handling." << Gmeng::resetcolor
                  << " "  << "However, the system could not bind to a port in the 7388-7488 range.\n"
                  << "\n" << Gmeng::boldcolor << "The port range of 7388 to 7488 must have at least one open port for GMENG to create a server instance.\n" << Gmeng::resetcolor
                  << " "  << "Please empty one of these ports to run a server.\n"
                  << "\n" << "Press [enter] to exit.\n";
        std::cin.get();
    } },
    { "init_from_main", []() {
        std::cout << Gmeng::colors[Gmeng::YELLOW] + "DEVELOPER ERROR!" << Gmeng::resetcolor;
        std::cout << " "  << "Gmeng (the engine this game runs on) has identified a developer error in this game instance.\n"
                  << "\n" << "this error is non-fatal, so it can be ignored, but various functionality may cause issues."
                  << "\n" << "Please reach out to the developers of this game and notify them about gmeng:warning:init_from_main.\n"
                  << "\n" << "note to developers of this game:\n"
                  << "\n" << "when initializing gmeng, run gmeng function 'patch_argv_global' within your c main() function."
                  << "\n" << "If gmeng is not initialized properly, you will break various engine functionality.\n";

        std::cout << "\nPress CTRL+C to exit or [enter] to continue.\n";
        std::cin.get();
    } },
    { "no_truecolor", []() {
        std::cout << Gmeng::colors[Gmeng::YELLOW] + "WARNING! " << Gmeng::resetcolor;
        std::cout << "Gmeng (the engine this game runs on) has identified that you have a non-truecolor shell.\n";
        std::cout << "\nUsual culprit is TMUX. Apple Terminal, iTerm and other ttys usually support truecolor by default, but TMUX overrides it.\n";
        std::cout << "\nIf you wish to use tmux with truecolor (RGB support) see " << Gmeng::colors[Gmeng::BLUE] << "gmeng.org/?doc=truecolor" << Gmeng::resetcolor << ".\n";

        std::cout << "\nPress CTRL+C to exit or [enter] to continue.\n";
        std::cin.get();
    } }
};

// internal warning method.
// displays warnings related to functions of the engine.
// do not use this method for your game. implement your own
// warning screen mechanisms.
static void _gmeng_show_warning(std::string warning_, char* filename, int fileline, TRACEFUNC) {
    printf("\033c"); // clear screen
    auto term_size = get_terminal_size();
    /// print warning tabtitle
    std::string warn_tab_text = "gmeng_show_warning: " + warning_ + ", " + get_filename(filename) + ":" + v_str( fileline ) + " @ " + std::string( func_caller );
    while( warn_tab_text.size() < term_size.width ) warn_tab_text += " "; // fill tab width
    std::cout << Gmeng::bgcolors_bright[Gmeng::WHITE] + Gmeng::colors[Gmeng::BLACK] + warn_tab_text << '\n' << Gmeng::resetcolor;

    if (gmeng_warnings.count(warning_) != 0) {
        gmeng_warnings.find(warning_)->second();
    } else {
        /// default warning

        std::cout << Gmeng::colors[Gmeng::RED];
        std::cout << "GMENG WARNING!" << Gmeng::resetcolor;
        std::cout << " <gmeng-default-warning-page>\n";
        std::cout << "[gmeng is the engine that powers this game - this warning is produced by the engine itself]";

        std::cout << "\n\nan internal subsystem of gmeng (" << get_filename(filename) << ':' << fileline << " & " << TRACEFUNC_STR <<  ")\n";
        std::cout << "has invoked a warning for '" << warning_ << "',\n";

        std::cout << "\npress CTRL+C to quit or [enter] to continue anyway.\n";

        std::cin.get();
    };
};

// internal warning method. displays warnings related to functions of the engine.
// do not use this method for your game. implement your own warning screen mechanisms.
#define gmeng_show_warning(x) _gmeng_show_warning( x, __FILE__, __LINE__ )


static void gm_err(const int& type, const std::string& err_title, TRACEFUNC) {
    __functree_call__(gm_err);
    _gm_log(func_caller_file, func_caller_line, func_caller, Gmeng::colors[Gmeng::RED] + "ERR! " + Gmeng::resetcolor + err_title);
    switch (type) {
        case 0: // v_gm_err case 0: continue running program
            gmeng_show_warning( err_title );
            break;
        case 1: // v_gm_err case 1: exception (stop execution)
            throw std::runtime_error(Gmeng::colors[4] + "gm:0 *error >> " + err_title + '\n' + Gmeng::resetcolor);
            break;
    };
    return;
};

