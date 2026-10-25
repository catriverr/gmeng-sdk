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

#include "scripts.h"

namespace Gmeng {
    enum KEYEVENT {
        gkEv_PRESS   = 1,
        gkEv_REPEAT  = 2,
        gkEv_RELEASE = 3
    };

    enum KEY {
        /// Alphabet
        gk_KEYS_START = 97,
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
        gk_KEYS_END = 122,

        /// Numbers
        gk_NUMBERS_START = 48,
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
        gk_NUMBERS_END = 57,

        /// Special Characters
        gk_SPECIAL_START  =  96,
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
        gk_SPECIAL_END    =  46,

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

        /// ARROW KEYS
        gk_UP    = 57399,
        gk_DOWN  = 57400,
        gk_LEFT  = 57401,
        gk_RIGHT = 57402,

        /// Navigation
        gk_INSERT    = 57396,
        gk_DELETE    = 57397,
        gk_PAGE_UP   = 57398,
        gk_PAGE_DOWN = 57395,
        gk_HOME      = 57412,
        gk_END       = 57414,
    };

    /// Returns the 'pressed' state of a given key. Support is only enabled
    /// for this method if the `Kitty Terminal Keyboard Protocol` is supported
    /// by the terminal. Checked via `gmeng_keyboard_init()` and `gmeng_keyboard_deinit()`.
    inline constexpr const bool& getkey(const KEY& key);
    /// Gmeng internal key states map. Contains ASCII keycode to pressed state
    /// mappings. This map is updated via the function `gmeng_update_keys()`.
    /// Gmeng's `do_event_loop()` functionality automatically handles everything
    /// for tracking individual keys. users can get key states via `Gmeng::getkey()`.
    /// For a list of mappings to key codes, see the `Gmeng::KEY` enum.
    static std::unordered_map<int, bool> key_states;
};

#define FOR_EACH_GMENG_gk_KEYS(vB) for ( unsigned int vB = gk_KEYS_START; vB < gk_KEYS_START; vB++ )

#if defined(_WIN32)

inline Gmeng::getkey(KEYS key) {
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

#else

#include "global.h"

inline constexpr const bool& Gmeng::getkey(const Gmeng::KEY& key) {
    return Gmeng::key_states[(int)key];
};

/// Initializes the Kitty Keyboard Protocol (for keyUp and keyDown events.)
/// This allows Gmeng to track individual key states for an actual input processing
/// instead of relying on the tty read stream or accessing the global key states.
///
/// This function returns true if the keyboard protocol is successfully initialized,
/// and 'false' if not. Supported terminals are currently limited to Kitty, but it is
/// being worked on to import different terminals such as iTerm2 which support it as well.
static bool gmeng_keyboard_init() {
    /// keyboard system is disabled explicitly
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

#ifndef GMENG_NO_SOL
    Gmeng::Scripts::gmeng_lua_namespace.new_enum("KEY",
        "gk_KEYS_START", Gmeng::KEY::gk_KEYS_START,
        "gk_A", Gmeng::KEY::gk_A,
        "gk_B", Gmeng::KEY::gk_B,
        "gk_C", Gmeng::KEY::gk_C,
        "gk_D", Gmeng::KEY::gk_D,
        "gk_E", Gmeng::KEY::gk_E,
        "gk_F", Gmeng::KEY::gk_F,
        "gk_G", Gmeng::KEY::gk_G,
        "gk_H", Gmeng::KEY::gk_H,
        "gk_I", Gmeng::KEY::gk_I,
        "gk_J", Gmeng::KEY::gk_J,
        "gk_K", Gmeng::KEY::gk_K,
        "gk_L", Gmeng::KEY::gk_L,
        "gk_M", Gmeng::KEY::gk_M,
        "gk_N", Gmeng::KEY::gk_N,
        "gk_O", Gmeng::KEY::gk_O,
        "gk_P", Gmeng::KEY::gk_P,
        "gk_Q", Gmeng::KEY::gk_Q,
        "gk_R", Gmeng::KEY::gk_R,
        "gk_S", Gmeng::KEY::gk_S,
        "gk_T", Gmeng::KEY::gk_T,
        "gk_U", Gmeng::KEY::gk_U,
        "gk_V", Gmeng::KEY::gk_V,
        "gk_W", Gmeng::KEY::gk_W,
        "gk_X", Gmeng::KEY::gk_X,
        "gk_Y", Gmeng::KEY::gk_Y,
        "gk_Z", Gmeng::KEY::gk_Z,
        "gk_KEYS_END", Gmeng::KEY::gk_KEYS_END,

        "gk_NUMBERS_START", Gmeng::KEY::gk_NUMBERS_START,
        "gk_0", Gmeng::KEY::gk_0,
        "gk_1", Gmeng::KEY::gk_1,
        "gk_2", Gmeng::KEY::gk_2,
        "gk_3", Gmeng::KEY::gk_3,
        "gk_4", Gmeng::KEY::gk_4,
        "gk_5", Gmeng::KEY::gk_5,
        "gk_6", Gmeng::KEY::gk_6,
        "gk_7", Gmeng::KEY::gk_7,
        "gk_8", Gmeng::KEY::gk_8,
        "gk_9", Gmeng::KEY::gk_9,
        "gk_NUMBERS_END", Gmeng::KEY::gk_NUMBERS_END,

        "gk_SPECIAL_START", Gmeng::KEY::gk_SPECIAL_START,
        "gk_BACKQUOTE", Gmeng::KEY::gk_BACKQUOTE,
        "gk_SPACE", Gmeng::KEY::gk_SPACE,
        "gk_TAB", Gmeng::KEY::gk_TAB,
        "gk_BACKSPACE", Gmeng::KEY::gk_BACKSPACE,
        "gk_SLASH", Gmeng::KEY::gk_SLASH,
        "gk_ENTER", Gmeng::KEY::gk_ENTER,
        "gk_BACKSLASH", Gmeng::KEY::gk_BACKSLASH,
        "gk_MINUS", Gmeng::KEY::gk_MINUS,
        "gk_EQUALS", Gmeng::KEY::gk_EQUALS,
        "gk_LSQUAREBRACKET", Gmeng::KEY::gk_LSQUAREBRACKET,
        "gk_RSQUAREBRACKET", Gmeng::KEY::gk_RSQUAREBRACKET,
        "gk_SEMICOLON", Gmeng::KEY::gk_SEMICOLON,
        "gk_APOSTROPHE", Gmeng::KEY::gk_APOSTROPHE,
        "gk_COMMA", Gmeng::KEY::gk_COMMA,
        "gk_DOT", Gmeng::KEY::gk_DOT,
        "gk_SPECIAL_END", Gmeng::KEY::gk_SPECIAL_END,

        "gk_CAPSLOCK", Gmeng::KEY::gk_CAPSLOCK,
        "gk_ESC", Gmeng::KEY::gk_ESC,
        "gk_LSHIFT", Gmeng::KEY::gk_LSHIFT,
        "gk_RSHIFT", Gmeng::KEY::gk_RSHIFT,
        "gk_LCTRL", Gmeng::KEY::gk_LCTRL,
        "gk_RCTRL", Gmeng::KEY::gk_RCTRL,
        "gk_LALT", Gmeng::KEY::gk_LALT,
        "gk_LMETA", Gmeng::KEY::gk_LMETA,
        "gk_RMETA", Gmeng::KEY::gk_RMETA,

        "gk_UP", Gmeng::KEY::gk_UP,
        "gk_DOWN", Gmeng::KEY::gk_DOWN,
        "gk_LEFT", Gmeng::KEY::gk_LEFT,
        "gk_RIGHT", Gmeng::KEY::gk_RIGHT,

        "gk_INSERT", Gmeng::KEY::gk_INSERT,
        "gk_DELETE", Gmeng::KEY::gk_DELETE,
        "gk_PAGE_UP", Gmeng::KEY::gk_PAGE_UP,
        "gk_PAGE_DOWN", Gmeng::KEY::gk_PAGE_DOWN,
        "gk_HOME", Gmeng::KEY::gk_HOME,
        "gk_END", Gmeng::KEY::gk_END
    );

    Gmeng::Scripts::gmeng_lua_namespace.set_function("getkey", &Gmeng::getkey);
#endif

    return true;
};


static void gmeng_keyboard_deinit() {
    printf("\x1b[<u");
    fflush(stdout);
    Gmeng::global.terminal_has_keyboard_support = false;
};

inline void gmeng_parse_kitty_input(const std::string& seq, int& code, Gmeng::KEYEVENT& event_type) {
    if (seq.size() < 3 || seq[0] != '\x1b' || seq[1] != '[') return;

    char suffix = seq.back();
    code = 0;
    event_type = (Gmeng::KEYEVENT)1; // Default to press (1) if not specified

    size_t semi_pos = seq.find(';');
    size_t first_colon = std::string::npos;

    if (semi_pos != std::string::npos) {
        // Only look for event_type colons AFTER the semicolon (modifiers section)
        // This avoids accidentally parsing alternate key codes like \x1b[97:65u
        first_colon = seq.find(':', semi_pos);
    }

    if (first_colon != std::string::npos) {
        size_t second_colon = seq.find(':', first_colon + 1);
        size_t end_pos = (second_colon != std::string::npos) ? second_colon : (seq.size() - 1);
        event_type = (Gmeng::KEYEVENT)std::stoi(seq.substr(first_colon + 1, end_pos - first_colon - 1));
    }

    if (suffix == 'u' || suffix == '~') {
        int base_num = 0;
        if (semi_pos != std::string::npos && semi_pos < seq.size() - 1) {
            base_num = std::stoi(seq.substr(2, semi_pos - 2));
        } else {
            base_num = std::stoi(seq.substr(2, seq.size() - 3));
        }

        if (suffix == 'u') {
            code = base_num;
        } else if (suffix == '~') {
            switch (base_num) {
                case 2: code = 57396; break; // INSERT
                case 3: code = 57397; break; // DELETE
                case 5: code = 57398; break; // PAGE_UP
                case 6: code = 57395; break; // PAGE_DOWN
            }
        }
    } 
    else {
        // Letter suffixes carry the key identity directly in the character itself
        switch (suffix) {
            case 'A': code = 57399; break; // UP
            case 'B': code = 57400; break; // DOWN
            case 'C': code = 57402; break; // RIGHT
            case 'D': code = 57401; break; // LEFT
            case 'H': code = 57412; break; // HOME
            case 'F': code = 57414; break; // END
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

static constexpr char* default_keylocale = "com.apple.keylayout.US";

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
        },
    },
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
