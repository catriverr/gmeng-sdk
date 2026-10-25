#pragma once

#include "global.h"
#include <string>
#include <vector>

/// Replaces all occurences of a substring within
/// a given string to a given substring.
static std::string str_replace(const std::string& str, const std::string& from, const std::string& to) {
    // __functree_call__(str_replace);
    std::string result = str; std::size_t start_pos = 0;
    while ((start_pos = result.find(from, start_pos)) != std::string::npos) {
        result.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return result;
};

/// foregroundcolor text identifiers.
/// TODO: convert to `char` array.
static std::vector<std::string> colornames = {
    "w", "b", "g", "c", "r", "p", "y", "0", "_", "o", "1"
    // white blue green cyan red pink yellow black
};

/// backgroundcolor text identifiers.
static std::vector<std::string> bgcolornames = {
    "Bw", "Bb", "Bg", "Bc", "Br", "Bp", "By", "B0", "Bw", "B1"
    // white blue green cyan red pink yellow black
};

#include "../types/color32.h"

/// Formats a text with colorcodes. Converts `~<colorname>~` indexes
/// into ANSI escape code sequences.
/// Example:
/// ```
/// std::string f_string = "~g~green text ~n~normal text ~b~blue text ...";
/// std::string formatted = colorformat(f_string);
/// ```
static std::string colorformat(const std::string& data, bool append_end_format = true) {
    std::string formatted = data;
    for (int i = 0; i < (sizeof(Gmeng::colors)/sizeof(*Gmeng::colors)); i++) {
        formatted = str_replace(formatted, "~" + colornames[i] + "~", Gmeng::colors[i]);
    };
    for (int i = 0; i < (sizeof(Gmeng::bgcolors)/sizeof(*Gmeng::bgcolors)); i++) {
        formatted = str_replace(formatted, "~" + bgcolornames[i] + "~", Gmeng::bgcolors[i]);
    };

    formatted = str_replace(formatted, "~h~", "\033[1m");
    formatted = str_replace(formatted, "~n~", "\033[0m");
    formatted = str_replace(formatted, "~i~", "\033[3m");
    formatted = str_replace(formatted, "~u~", "\033[4m");
    formatted = str_replace(formatted, "~st~", "\033[9m");
    return formatted + (append_end_format ? "\033[0m" : "");
};

/// Writes to the standard output with color formatting.
/// See `colorformat()` to see ANSI escape code sequencing.
#define SAY std::cout << colorformat
/// Writes to the standard output with information formatting.
/// See `colorformat()` to see ANSI escape code sequencing.
#define INF std::cout << "\033[1m" + Gmeng::colors[Gmeng::BLUE] + "(i) >> \033[0m" + Gmeng::colors[Gmeng::WHITE] + colorformat


#include "../strings/replace_all.cc"

#include "../src/version.h"

/// Parses string variables (VERSION and BUILD).
/// str vars are accessed with the `$!__VARIABLE_NAME` syntax.
static std::string parse_str_vars(const std::string& a) {
    std::string res = a;
    replace_all(res, "$!__VERSION", Gmeng::version);
    replace_all(res, "$!__BUILD", GMENG_BUILD_NO);
    return res;
};

/// Writes a parsed string to the standard output.
/// Parsed strings include string variables. See `colorformat()` for
/// ANSI escape sequencing and highlighting functionality.
#define WRITE_PARSED(x) std::cout << colorformat(parse_str_vars(x))
