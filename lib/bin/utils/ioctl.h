#pragma once

#include "ansi.h"
#include <string>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
#endif

#include <unistd.h>

struct TerminalSize {
    int width; int height;

    bool operator==( const TerminalSize& other ) const = default;

};

/// LMAO cross-platform as if this engine will run on windows any time soon
static TerminalSize get_terminal_size() {
    TerminalSize size{0, 0};

#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        size.width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        size.height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        size.width = w.ws_col;
        size.height = w.ws_row;
    }
#endif

    return size;
}

static void centertext(std::string& text, TerminalSize sz = get_terminal_size()) {
    while ( strip_ansi(text).length() < sz.width ) text = " " + text + " ";
    while ( strip_ansi(text).length() > sz.width ) text = text.substr(0, text.length()-1);
};
