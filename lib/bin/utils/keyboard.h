#pragma once

#include <charconv>
#include <iostream>

#include "../strings/contains.cc"

#include <Carbon/Carbon.h>
#include <CoreGraphics/CGEventTypes.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>
#if defined(_WIN32)
    #include <windows.h>
    #define GMENG_KEYDEF(win, mac, lin) win
#elif defined(__APPLE__)
    #define GMENG_KEYDEF(win, mac, lin) mac
#elif defined(__linux__)
    #include <linux/input.h>
    #define GMENG_KEYDEF(win, mac, lin) lin
#endif

namespace Gmeng {
    enum KEYEVENT {
        gkEv_PRESS   = 1,
        gkEv_REPEAT  = 2,
        gkEv_RELEASE = 3
    };
    enum KEY {
        /// Alphabet
        gk_A = 97,
        gk_B = 98,
        gk_C = 99,
        gk_D = 100,
        gk_E = 101,
        gk_F = 102,
        gk_G = 103,
        gk_H = 104,
        gk_I = 105,
        gk_J = 106,
        gk_K = 107,
        gk_L = 108,
        gk_M = 109,
        gk_N = 110,
        gk_O = 111,
        gk_P = 112,
        gk_Q = 113,
        gk_R = 114,
        gk_S = 115,
        gk_T = 116,
        gk_U = 117,
        gk_V = 118,
        gk_W = 119,
        gk_X = 120,
        gk_Y = 121,
        gk_Z = 122,

        /// Numbers
        gk_0 = 48,
        gk_1 = 49,
        gk_2 = 50,
        gk_3 = 51,
        gk_4 = 52,
        gk_5 = 53,
        gk_6 = 54,
        gk_7 = 55,
        gk_8 = 56,
        gk_9 = 57,

        /// Special Characters
        gk_BACKQUOTE      =  96,
        gk_SPACE          =  32,
        gk_TAB            =   9,
        gk_BACKSPACE      = 127,
        gk_SLASH          =  47,
        gk_ENTER          =  13,
        gk_BACKSLASH      =  92,
        gk_MINUS          =  45,
        gk_EQUALS         =  61,
        gk_LSQUAREBRACKET =  91,
        gk_RSQUAREBRACKET =  93,
        gk_SEMICOLON      =  59,
        gk_APOSTROPHE     =  39,
        gk_COMMA          =  44,
        gk_DOT            =  46,

        /// Modifier characters
        gk_CAPSLOCK       = 57358,
        gk_ESC            =    27,
        gk_LSHIFT         = 57441,
        gk_LCTRL          = 57442,
        gk_LALT           = 57443,
        gk_LMETA          = 57444,
        gk_RMETA          = 57450,
        gk_RCTRL          = 57448,
        gk_RSHIFT         = 57447,
    };

    /// Returns the 'pressed' state of a given key. Support is only enabled
    /// for this method if the `Kitty Terminal Keyboard Protocol` is supported
    /// by the terminal. Checked via `gmeng_keyboard_init()` and `gmeng_keyboard_deinit()`.
    inline bool getkey(KEY key);
    /// Gmeng internal key states map. Contains ASCII keycode to pressed state
    /// mappings. This map is updated via the function `gmeng_update_keys()`.
    /// Gmeng's `do_event_loop()` functionality automatically handles everything
    /// for tracking individual keys. users can get key states via `Gmeng::getkey()`.
    /// For a list of mappings to key codes, see the `Gmeng::KEY` enum.
    static std::unordered_map<int, bool> key_states;
};

#if defined(_WIN32)

inline Gmeng::getkey(KEYS key) {
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

#else

#include "global.h"

inline bool Gmeng::getkey(Gmeng::KEY key) {
    return Gmeng::key_states[(int)key];
};

/// Initializes the Kitty Keyboard Protocol (for keyUp and keyDown events.)
/// This allows Gmeng to track individual key states for an actual input processing
/// instead of relying on the tty read stream or accessing the global key states.
///
/// This function returns true if the keyboard protocol is successfully initialized,
/// and 'false' if not. Supported terminals are currently limited to Kitty, but it is
/// being worked on to import different terminals such as iTerm2 which supports the protocol as well.
static bool gmeng_keyboard_init() {
    /// keyboard system is disabled implicitly
    if ( Gmeng::global.no_advanced_keyboard ) return false;
    const char* term = std::getenv("TERM");
    const char* term_prog = std::getenv("TERM_PROGRAM");

    std::string terms = term != nullptr ? term : "";
    std::string termps = term_prog != nullptr ? term_prog : "";

    if ( !contains(terms,  "kitty") &&
         !contains(termps, "iTerm") ) return false;

    printf("\x1b[>31u");
    fflush(stdout);
    Gmeng::global.terminal_has_keyboard_support = true;
    return true;
};


static void gmeng_keyboard_deinit() {
    printf("\x1b[<u");
    fflush(stdout);
    Gmeng::global.terminal_has_keyboard_support = false;
};

inline void gmeng_parse_kitty_input(const std::string& seq, int& code, Gmeng::KEYEVENT& event_type) {
    // Kitty sequences match: ESC [ key ; modifiers : event_type : text u
    // Examples: \x1b[97u, \x1b[97;2:3u (Shift + Release)
    if (seq.size() > 2 && seq[0] == '\x1b' && seq[1] == '[') {
        size_t u_pos = seq.find('u');
        if (u_pos != std::string::npos) {
            size_t semi_pos = seq.find(';');

            code = 0;
            event_type = (Gmeng::KEYEVENT)1; // Default to press (1) if not specified

            if (semi_pos != std::string::npos) {
                // Extract key code (everything between '[' and ';')
                code = std::stoi(seq.substr(2, semi_pos - 2));

                // Find the first colon after the semicolon
                size_t first_colon = seq.find(':', semi_pos);
                if (first_colon != std::string::npos && first_colon < u_pos) {
                    // Find if there is a second colon (which would terminate event_type if 'text' exists)
                    size_t second_colon = seq.find(':', first_colon + 1);
                    size_t end_pos = (second_colon != std::string::npos && second_colon < u_pos) ? second_colon : u_pos;

                    event_type = (Gmeng::KEYEVENT)std::stoi(seq.substr(first_colon + 1, end_pos - first_colon - 1));
                }
            } else {
                // No semicolon: only key code exists (e.g., \x1b[97u)
                code = std::stoi(seq.substr(2, u_pos - 2));
            }
        }
    }
}

inline void gmeng_update_keys(char sequence[64], int& code, Gmeng::KEYEVENT& event) {
    /// `code`  == charcode
    /// `event` == { 1 = press, 2 = repeat, 3 = release }
    gmeng_parse_kitty_input(sequence, code, event);

    Gmeng::key_states[ code ] = event != 3;
};

#include "../types/event.h"

/// Gmeng : Converts a keyboard event to a Game Event.
constexpr inline Gmeng::Event gmeng_keyevent( const Gmeng::KEYEVENT& kev ) {
    switch ( kev ) {
        case Gmeng::KEYEVENT::gkEv_PRESS:
            return Gmeng::KEYPRESS;
            break;
        case Gmeng::KEYEVENT::gkEv_REPEAT:
            return Gmeng::KEYREPEAT;
        case Gmeng::KEYEVENT::gkEv_RELEASE:
            return Gmeng::KEYRELEASE;
    };
};

static char* default_keylocale = "com.apple.keylayout.US";

static std::unordered_map<std::string, std::unordered_map<char, char>> SHIFTED_KEYLOCALES = {
    { default_keylocale,
        {
            { '1', '!' },
            { '2', '@' },
            { '3', '#' },
            { '4', '$' },
            { '5', '%' },
            { '6', '^' },
            { '7', '&' },
            { '8', '*' },
            { '9', '(' },
            { '0', ')' },
            { '-', '_' },
            { '=', '+' },
            { '`', '~' },
            { ',', '<' },
            { '.', '>' },
            { '/', '?' },
            { ';', ':' },
            { '\'', '"'},
            { '[', '{' },
            { ']', '}' },
            { '\\', '|' }
        }
    }
};

static char* current_keylocale = default_keylocale;
/// TODO: get_keylocale()

inline char get_shifted_key(const char& keycode) {
    if ( !SHIFTED_KEYLOCALES.contains(current_keylocale) ) {
        if (!SHIFTED_KEYLOCALES[default_keylocale].contains(keycode)) return keycode;
        else return SHIFTED_KEYLOCALES[default_keylocale][keycode];
    } else {
        if (!SHIFTED_KEYLOCALES[current_keylocale].contains(keycode)) return keycode;
        else return SHIFTED_KEYLOCALES[default_keylocale][keycode];
    };
};


/// Gmeng : converts a keystroke into the expected
/// character from the keyboard.
inline char gmeng_toinput(const char& keycode) {
    auto shift = Gmeng::getkey( Gmeng::gk_LSHIFT ) || Gmeng::getkey( Gmeng::gk_RSHIFT );

    if ( shift ) {
        if ( toupper(keycode) == keycode ) {
            return get_shifted_key(keycode);
        } else return toupper(keycode);
    } else return keycode;
};

#endif
