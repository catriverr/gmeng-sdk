#pragma once

#include "../gmeng.h"
#include <map>
#include <ncursesw/ncurses.h>
#include <locale.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <cstring>
#include <optional>
#include <utility>

namespace curseswutil {
    namespace internal {
        static struct termios orig_term;
        static bool mouse_enabled = false;
        static int last_mouse_x = -1;
        static int last_mouse_y = -1;

        static void enable_raw_mode() {
            tcgetattr(STDIN_FILENO, &orig_term);
            struct termios raw = orig_term;
            raw.c_lflag &= ~(ECHO | ICANON);
            raw.c_cc[VMIN] = 0;
            raw.c_cc[VTIME] = 1;
            tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        }

        static void disable_raw_mode() {
            tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
        }

        static void enable_mouse_tracking() {
            // Xterm "any motion" tracking
            write(STDOUT_FILENO, "\033[?1003h", 8);
            mouse_enabled = true;
        }

        static void disable_mouse_tracking() {
            write(STDOUT_FILENO, "\033[?1003l", 8);
            mouse_enabled = false;
        }

        static bool read_mouse_position() {
            unsigned char buf[6];
            int n = read(STDIN_FILENO, buf, sizeof(buf));
            if (n == 6 && buf[0] == 27 && buf[1] == '[' && buf[2] == 'M') {
                int cb = buf[3] - 32;
                int cx = buf[4] - 32;
                int cy = buf[5] - 32;
                last_mouse_x = cx - 1;
                last_mouse_y = cy - 1;
                return true;
            }
            return false;
        }
    }

    inline void init() {
        setlocale(LC_ALL, "");
        initscr();
        noecho();
        curs_set(0);
        keypad(stdscr, TRUE);
        start_color();
        mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, nullptr);
        mouseinterval(0);

        internal::enable_raw_mode();
        internal::enable_mouse_tracking();
    }

    inline void shutdown() {
        internal::disable_mouse_tracking();
        internal::disable_raw_mode();
        endwin();
    }
    // -- UI Drawing Utilities --
    inline void draw_text(int y, int x, const std::wstring& text) {
        mvaddwstr(y, x, text.c_str());
    }

    inline void draw_colored_text(int y, int x, const std::wstring& text, int color_pair) {
        attron(COLOR_PAIR(color_pair));
        mvaddwstr(y, x, text.c_str());
        attroff(COLOR_PAIR(color_pair));
    }

    inline void init_color_pair(short index, short fg, short bg = -1) {
        init_pair(index, fg, bg);
    }

    inline void draw_hline(int y, int x, int length) {
        mvhline(y, x, ACS_HLINE, length);
    }

    inline void draw_vline(int y, int x, int length) {
        mvvline(y, x, ACS_VLINE, length);
    }

    inline void draw_box(int y, int x, int height, int width) {
        mvhline(y, x, ACS_HLINE, width);
        mvhline(y + height - 1, x, ACS_HLINE, width);
        mvvline(y, x, ACS_VLINE, height);
        mvvline(y, x + width - 1, ACS_VLINE, height);
        mvaddch(y, x, ACS_ULCORNER);
        mvaddch(y, x + width - 1, ACS_URCORNER);
        mvaddch(y + height - 1, x, ACS_LLCORNER);
        mvaddch(y + height - 1, x + width - 1, ACS_LRCORNER);
    }

    /// Gmeng NCURSES color pair table for
    /// indexes. use with get_color_pair_index(fg, bg).
    static std::map<int, vector<short> > gmeng_to_curses_color_lookup = {
        {   0, { COLOR_WHITE,   COLOR_WHITE   } }, /// fg 0 bg 0
        { 256, { COLOR_BLUE,    COLOR_WHITE   } }, /// fg 1 bg 0
        { 512, { COLOR_GREEN,   COLOR_WHITE   } }, /// fg 2 bg 0
        { 768, { COLOR_CYAN,    COLOR_WHITE   } }, /// fg 3 bg 0
        {1024, { COLOR_RED,     COLOR_WHITE   } }, /// fg 4 bg 0
        {1280, { COLOR_MAGENTA, COLOR_WHITE   } }, /// fg 5 bg 0
        {1536, { COLOR_YELLOW,  COLOR_WHITE   } }, /// fg 6 bg 0
        {1792, { COLOR_BLACK,   COLOR_WHITE   } }, /// fg 7 bg 0

        {   1, { COLOR_WHITE,   COLOR_BLUE    } }, /// fg 0 bg 1
        { 257, { COLOR_BLUE,    COLOR_BLUE    } }, /// fg 1 bg 1
        { 513, { COLOR_GREEN,   COLOR_BLUE    } }, /// fg 2 bg 1
        { 769, { COLOR_CYAN,    COLOR_BLUE    } }, /// fg 3 bg 1
        {1025, { COLOR_RED,     COLOR_BLUE    } }, /// fg 4 bg 1
        {1281, { COLOR_MAGENTA, COLOR_BLUE    } }, /// fg 5 bg 1
        {1537, { COLOR_YELLOW,  COLOR_BLUE    } }, /// fg 6 bg 1
        {1793, { COLOR_BLACK,   COLOR_BLUE    } }, /// fg 7 bg 1

        {   2, { COLOR_WHITE,   COLOR_GREEN   } }, /// fg 0 bg 2
        { 258, { COLOR_BLUE,    COLOR_GREEN   } }, /// fg 1 bg 2
        { 514, { COLOR_GREEN,   COLOR_GREEN   } }, /// fg 2 bg 2
        { 770, { COLOR_CYAN,    COLOR_GREEN   } }, /// fg 3 bg 2
        {1026, { COLOR_RED,     COLOR_GREEN   } }, /// fg 4 bg 2
        {1282, { COLOR_MAGENTA, COLOR_GREEN   } }, /// fg 5 bg 2
        {1538, { COLOR_YELLOW,  COLOR_GREEN   } }, /// fg 6 bg 2
        {1794, { COLOR_BLACK,   COLOR_GREEN   } }, /// fg 7 bg 2

        {   3, { COLOR_WHITE,   COLOR_CYAN    } }, /// fg 0 bg 3
        { 259, { COLOR_BLUE,    COLOR_CYAN    } }, /// fg 1 bg 3
        { 515, { COLOR_GREEN,   COLOR_CYAN    } }, /// fg 2 bg 3
        { 771, { COLOR_CYAN,    COLOR_CYAN    } }, /// fg 3 bg 3
        {1027, { COLOR_RED,     COLOR_CYAN    } }, /// fg 4 bg 3
        {1283, { COLOR_MAGENTA, COLOR_CYAN    } }, /// fg 5 bg 3
        {1539, { COLOR_YELLOW,  COLOR_CYAN    } }, /// fg 6 bg 3
        {1795, { COLOR_BLACK,   COLOR_CYAN    } }, /// fg 7 bg 3

        {   4, { COLOR_WHITE,   COLOR_RED     } }, /// fg 0 bg 4
        { 260, { COLOR_BLUE,    COLOR_RED     } }, /// fg 1 bg 4
        { 516, { COLOR_GREEN,   COLOR_RED     } }, /// fg 2 bg 4
        { 772, { COLOR_CYAN,    COLOR_RED     } }, /// fg 3 bg 4
        {1028, { COLOR_RED,     COLOR_RED     } }, /// fg 4 bg 4
        {1284, { COLOR_MAGENTA, COLOR_RED     } }, /// fg 5 bg 4
        {1540, { COLOR_YELLOW,  COLOR_RED     } }, /// fg 6 bg 4
        {1796, { COLOR_BLACK,   COLOR_RED     } }, /// fg 7 bg 4

        {   5, { COLOR_WHITE,   COLOR_MAGENTA } }, /// fg 0 bg 5
        { 261, { COLOR_BLUE,    COLOR_MAGENTA } }, /// fg 1 bg 5
        { 517, { COLOR_GREEN,   COLOR_MAGENTA } }, /// fg 2 bg 5
        { 773, { COLOR_CYAN,    COLOR_MAGENTA } }, /// fg 3 bg 5
        {1029, { COLOR_RED,     COLOR_MAGENTA } }, /// fg 4 bg 5
        {1285, { COLOR_MAGENTA, COLOR_MAGENTA } }, /// fg 5 bg 5
        {1541, { COLOR_YELLOW,  COLOR_MAGENTA } }, /// fg 6 bg 5
        {1797, { COLOR_BLACK,   COLOR_MAGENTA } }, /// fg 7 bg 5

        {   6, { COLOR_WHITE,   COLOR_YELLOW  } }, /// fg 0 bg 6
        { 262, { COLOR_BLUE,    COLOR_YELLOW  } }, /// fg 1 bg 6
        { 518, { COLOR_GREEN,   COLOR_YELLOW  } }, /// fg 2 bg 6
        { 774, { COLOR_CYAN,    COLOR_YELLOW  } }, /// fg 3 bg 6
        {1030, { COLOR_RED,     COLOR_YELLOW  } }, /// fg 4 bg 6
        {1286, { COLOR_MAGENTA, COLOR_YELLOW  } }, /// fg 5 bg 6
        {1542, { COLOR_YELLOW,  COLOR_YELLOW  } }, /// fg 6 bg 6
        {1798, { COLOR_BLACK,   COLOR_YELLOW  } }, /// fg 7 bg 6

        {   7, { COLOR_WHITE,   COLOR_BLACK   } }, /// fg 0 bg 7
        { 263, { COLOR_BLUE,    COLOR_BLACK   } }, /// fg 1 bg 7
        { 519, { COLOR_GREEN,   COLOR_BLACK   } }, /// fg 2 bg 7
        { 775, { COLOR_CYAN,    COLOR_BLACK   } }, /// fg 3 bg 7
        {1031, { COLOR_RED,     COLOR_BLACK   } }, /// fg 4 bg 7
        {1287, { COLOR_MAGENTA, COLOR_BLACK   } }, /// fg 5 bg 7
        {1543, { COLOR_YELLOW,  COLOR_BLACK   } }, /// fg 6 bg 7
        {1799, { COLOR_BLACK,   COLOR_BLACK   } }  /// fg 7 bg 7
    };

    /// returns an index for a color pair of foreground and background.
    /// individual to each combination with regard to number order,
    /// so fg=2 bg=1 != fg=1 bg=2.
    ///
    /// This supports individual indexing for up to 256 colors.
    inline int get_color_pair_index(int fg, int bg) {
        return fg * 256 + bg; // 256 is more than the available colors as of
                              // gmeng 12.0.0
    };

    inline void draw_unit(int y, int x, Gmeng::Unit u, Gmeng::Unit nu) {
        auto index = get_color_pair_index(u.color, nu.color);
        auto pair = gmeng_to_curses_color_lookup.at(index);
        curseswutil::init_color_pair(
            0,
            pair.at(0), // fg
            pair.at(1)  // bg
        );
        attron(A_BOLD);
        draw_colored_text(y, x, L"\u2584", 0);
        attroff(A_BOLD);
    };

    inline void draw_texture(int y, int x, Gmeng::texture _texture) {
        int _x,_y = 0;
        for (int i = 0; i < _texture.width * _texture.height; i++) {
            if (_x % _texture.width == 0) y++, _x = 0;
            draw_unit(y + (_y/2), x + _x, _texture.units.at(i), _texture.units.at(i));
            _x++;
        };
    };

    // -- Mouse Tracking --
    struct MousePosition {
        int row;
        int col;
    };
    static MousePosition last_mousepos = { -1, -1 };

    inline std::optional<MousePosition> get_mouse_pos() {
        if (internal::read_mouse_position()) {
            last_mousepos = MousePosition{
                .row = internal::last_mouse_y,
                .col = internal::last_mouse_x
            };
            return last_mousepos;
        }
        return last_mousepos;
    };
}
