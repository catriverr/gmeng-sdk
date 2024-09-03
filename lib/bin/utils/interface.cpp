#pragma once
#include "../gmeng.h"
#include "network.cpp"
#include "../types/interface.h"
#include <clocale>
#include <ctime>
#include <cwchar>
#include <functional>
#include <future>
#include <locale.h>
#include <iostream>
#include <optional>
#include <string>
#include <algorithm>
#include <array>
#include <map>
#include <thread>
#include <vector>
#include <ncursesw/ncurses.h>
#include <unordered_map>
#include <regex.h>
#include <wchar.h>
#include <memory>
#include <ApplicationServices/ApplicationServices.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define sleep std::this_thread::sleep_for
#define ms std::chrono::milliseconds

#ifndef BUTTON5_PRESSED
    #define BUTTON5_PRESSED true
#endif /// quick little fix for the 5.7 ncurses version. USE G++ PARAMETER: `pkg-config --libs --cflags ncurses`!!!!!!

std::string _gutil_get_terminal() {
    __functree_call__(__FILE__, __LINE__, _gutil_get_terminal);
    char* term_program = getenv("TERM_PROGRAM");
    if (term_program != nullptr) {
        return std::string(term_program);
    } else {
        return "";
    }
};

/// MOUSE POSITION DATA, SO MANY LOW LEVEL FUNCTION CALLS DEPENDENT ON THE ENVIRONMENT
/// NEEDS BE CHANGED TO SUPPORT OTHER OSes.



using namespace Gmeng;

Renderer::drawpoint get_char_sizes() {
    __functree_call__(__FILE__, __LINE__, get_char_sizes);
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

    int num_cols = size.ws_col; // Number of columns in terminal
    int num_lines = size.ws_row; // Number of lines (rows) in terminal

    // Assuming typical monospaced font, estimate character size
    int char_width = size.ws_xpixel / num_cols;
    int char_height = size.ws_ypixel / num_lines;
    return {char_width,char_height};
};

volatile int wcharsizex = get_char_sizes().x;
volatile int wcharsizey = get_char_sizes().y;
volatile Renderer::drawpoint charsizes = {*(&wcharsizex),*(&wcharsizey)};


CGWindowID get_window_id() {
    CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
    if (windowList == nullptr) {
        std::cerr << "Failed to get window list\n";
        return 0;  // Return an error value
    }

    CFDictionaryRef frontmostWindow = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(windowList, 0));
    if (frontmostWindow == nullptr) {
        std::cerr << "Failed to get frontmost window\n";
        CFRelease(windowList);
        return 0;  // Return an error value
    }

    CFNumberRef windowNumber = static_cast<CFNumberRef>(CFDictionaryGetValue(frontmostWindow, kCGWindowNumber));
    if (windowNumber == nullptr) {
        std::cerr << "Failed to get window number\n";
        CFRelease(windowList);
        return 0;  // Return an error value
    }

    int64_t windowID;
    CFNumberGetValue(windowNumber, kCFNumberLongLongType, &windowID);

    CFRelease(windowList);

    return static_cast<CGWindowID>(windowID);
};

CGDirectDisplayID get_display_for_window(CGWindowID windowID) {
    CFArrayRef windowArray = CFArrayCreate(NULL, (const void **)&windowID, 1, NULL);
    CFArrayRef windowDescriptionArray = CGWindowListCreateDescriptionFromArray(windowArray);
    CFDictionaryRef windowDescription = (CFDictionaryRef)CFArrayGetValueAtIndex(windowDescriptionArray, 0);
    CFDictionaryRef bounds = (CFDictionaryRef)CFDictionaryGetValue(windowDescription, kCGWindowBounds);
    CGRect windowRect;
    CGRectMakeWithDictionaryRepresentation(bounds, &windowRect);
    CGPoint windowOrigin = windowRect.origin;
    CFRelease(windowArray);
    CFRelease(windowDescriptionArray);
    CGDirectDisplayID displayID = kCGNullDirectDisplay;
    CGDisplayCount displayCount;
    CGDirectDisplayID displays[32]; // Assumes no more than 32 displays
    CGGetDisplaysWithPoint(windowOrigin, 32, displays, &displayCount);
    if (displayCount > 0) {
        displayID = displays[0]; // Choose the first display
    }
    return displayID;
};

Renderer::drawpoint get_window_size(CGWindowID windowID) {
    CTFontRef font = CTFontCreateUIFontForLanguage(kCTFontUIFontUser, 0, NULL);
    CGRect bounds = CTFontGetBoundingBox(font);
    CFRelease(font);
    Renderer::drawpoint size;
    auto win_id = get_window_id();
    size.x = (bounds.size.width/2-1) * (COLS);
    size.x = size.x > CGDisplayBounds(get_display_for_window(win_id)).size.width ? CGDisplayBounds(get_display_for_window(win_id)).size.width : size.x;
    size.y = bounds.size.width       * (LINES-3);
    size.y = size.y > CGDisplayBounds(get_display_for_window(win_id)).size.height ? CGDisplayBounds(get_display_for_window(win_id)).size.height : size.y;
    return size;
};

Renderer::drawpoint scale_to_window_size(Renderer::drawpoint mouse_pos) {
    Renderer::drawpoint window_size = get_window_size(get_window_id());
    int _x = mouse_pos.x > window_size.x ? window_size.x : mouse_pos.x;
    int _y = mouse_pos.y > window_size.y ? window_size.y : mouse_pos.y;
    return {
        _x, _y
    };
};

Renderer::drawpoint get_pointed_pos(Renderer::drawpoint mpos) {
    CTFontRef font = CTFontCreateUIFontForLanguage(kCTFontUIFontUser, 0, NULL);
    CGRect bounds = CTFontGetBoundingBox(font);
    CFRelease(font);
    Renderer::drawpoint _charsizes = {wcharsizex,wcharsizey};
    //double _x = ( (double)mpos.x / ( (double)(bounds.size.width-2.7)/2 ) ); // magic number 1 (retarded fonts being all wacky)
    //double _y = ( (double)mpos.y / ( (double)(bounds.size.width)-3.4 )   ); // perfectionism is an illness
    double _x = ( (double)mpos.x / (double)_charsizes.x);
    double _y = ( (double)mpos.y / (double)_charsizes.y);
    return {
        _x < 0 ? 0 : (int)(_x > COLS-1  ? COLS -1 : _x ), // x position divided by the length of each char returns X
        _y < 0 ? 0 : (int)(_y > LINES-1 ? LINES-1 : _y )  // y position divided by the height of each char return  Y
    }; //there is no error here, bounds.width equals the height. bounds.width/2 equals width.
};

Renderer::drawpoint get_mouse_pos() {
    CGRect terminalBounds;
    // Get the bounds of the terminal window
    CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
    CFIndex count = CFArrayGetCount(windowList);
    for (CFIndex i = 0; i < count; i++) {
        CFDictionaryRef windowInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(windowList, i);
        CFStringRef ownerName = (CFStringRef)CFDictionaryGetValue(windowInfo, kCGWindowOwnerName);
        if (CFStringCompare(ownerName, CFSTR("Terminal"), kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
            CGRectMakeWithDictionaryRepresentation((CFDictionaryRef)CFDictionaryGetValue(windowInfo, kCGWindowBounds), &terminalBounds);
            break;
        }
    };
    CFRelease(windowList);
    // Get the global mouse position
    CGPoint globalMousePos = CGEventGetLocation(CGEventCreate(NULL));
    // Calculate the relative mouse position within the terminal window
    CGPoint relativeMousePos = {
        globalMousePos.x - terminalBounds.origin.x,
        terminalBounds.size.height - (globalMousePos.y - terminalBounds.origin.y)
    };
    auto win_id = get_window_id();
    CGDirectDisplayID display_id = get_display_for_window(win_id);
    Renderer::drawpoint window_size = get_window_size(win_id);
    CGRect dbounds = {.origin=NULL,.size={
        .width = v_static_cast<CGFloat>(window_size.x),
        .height = v_static_cast<CGFloat>(window_size.y)
    }};
    int _x = (int)relativeMousePos.x > (int)dbounds.size.width ? (int)dbounds.size.width : (int)relativeMousePos.x;
    int final_x = _x < 0 ? 0 : _x;
    int _y = (int)relativeMousePos.y > (int)dbounds.size.height ? (int)dbounds.size.height : (int)relativeMousePos.y;
    int final_y = _y < 0 ? 0 : _y;
    Renderer::drawpoint fin_size = scale_to_window_size({final_x, (int)dbounds.size.height-final_y});
    return {fin_size.x, (int)dbounds.size.height-final_y};
};

struct transportable_image {
    int width; int height;
    std::vector<Gmeng::Unit> units;
};

std::vector<Unit> harvest_units(const std::vector<Unit>& units, int original_width, const Renderer::viewpoint& view) {
    //__functree_call__(__FILE__, __LINE__, harvest_units);
    std::vector<Unit> harvested_units;
    if (original_width == _vcreate_vp2d_deltax(view)+1) return units;
    int start_x = view.start.x; int start_y = view.start.y;
    int end_x = view.end.x; int end_y = view.end.y;
    for (int i = 0; i < units.size(); ++i) {
        int unit_x = i % original_width; int unit_y = i / original_width;
        if (unit_x >= start_x && unit_x <= end_x &&
            unit_y >= start_y && unit_y <= end_y) {
            harvested_units.push_back(units[i]);
        }
    }

    return harvested_units;
};


// dark magic fuckery
std::vector<Unit> scale_image(const std::vector<Unit>& units, int original_width, int new_width, int new_height) {
    //__functree_call__(__FILE__, __LINE__, scale_image);
    // If original width and new width are the same, return original units
    if (original_width == new_width) return units;
    int original_height = static_cast<int>(units.size()) / original_width;
    float scale_x = static_cast<float>(original_width) / new_width; float scale_y = static_cast<float>(original_height) / new_height;
    std::vector<Unit> scaled_units(new_width * new_height);
    for (int y = 0; y < new_height; ++y) {
        for (int x = 0; x < new_width; ++x) {
            int original_x = static_cast<int>(x * scale_x); int original_y = static_cast<int>(y * scale_y);
            int original_index = original_y * original_width + original_x;
            int scaled_index = y * new_width + x;
            scaled_units[scaled_index] = units[original_index];
        };
    };
    return scaled_units;
}

bool kbhit(char key) {
    fd_set read_fds;
    struct timeval timeout;
    int result;

    // Clear the file descriptor set
    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    // Set the timeout to 0 to make the call non-blocking
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    // Check if there is input available
    result = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout);
    if (result > 0 && FD_ISSET(STDIN_FILENO, &read_fds)) {
        char ch = getchar();
        return ch == key;
    }
    return false;
}
namespace Gmeng {
    namespace UI {
        const wchar_t* wc_unit             = L"\u2588";
	    const wchar_t* wc_outer_unit       = L"\u2584";
        const wchar_t* wc_outer_unit_floor = L"\u2580";
        using namespace UI::Interactions;
        /// Base struct, cannot be used.
        struct Element {
          protected:
            enum Types {
                ELEMENT    = 0,
                BUTTON     = 1,
                ACTIONMENU = 2,
            };
          public:
            Renderer::drawpoint __raw_position;
            bool __any_modifier__ = false;
            int id = g_mkid();
            static Renderer::viewpoint get_viewpoint(UI::Element& elem) {
                //__functree_call__(__FILE__, __LINE__, Gmeng::UI::Element::__static__::get_viewpoint);
                return {
                    elem.position,
                    { elem.position.x + (int)elem.width, elem.position.y + (int)elem.height }
                };
            };

            bool hovered; bool clicked; bool hidden;
            std::size_t height; std::size_t width;
            std::string title; Renderer::drawpoint position;

            short foreground_color;
            short background_color;
            short background_color_highlight;
            short foreground_color_highlight;
            short foreground_color_click;
            short background_color_click;

            Element(bool hovered, std::size_t height, std::size_t width, std::string title, Renderer::drawpoint position,
                    Gmeng::uicolor_t foreground_color, Gmeng::uicolor_t background_color,
                    Gmeng::uicolor_t background_color_highlight, Gmeng::uicolor_t foreground_color_highlight)
                : hovered(hovered), height(height), width(width), title(std::move(title)), position(position),
                  foreground_color(foreground_color), background_color(background_color),
                  background_color_highlight(background_color_highlight),
                  foreground_color_highlight(foreground_color_highlight) {};
            Element() : id(g_mkid()) {};
            virtual ~Element() = default;
            /// Hover function. Runs when the element is hovered.
            virtual void hover(UI::Screen* instance, bool state) { /* no predefined behaviour */ };
            /// Click function. Runs when the element is clicked.
            virtual void click(UI::Screen* instance, UI::Interactions::MouseButton button) { /* no predefined behaviour */ };
            /// Refresh function. Draws the Element, controlled by main class.
            virtual void refresh(UI::Screen* instance, UI::Interactions::ButtonInteraction button) = 0;
            /// Text input function. When text is stream to the element, controlled b y main class.
            virtual void text_recv(UI::Screen* instance, char ch) { /* no default behaviour */ };
            /// Scroll function. When the scroll event is called from ncurses, streamed into this class.
            virtual void mouse_scroll(UI::Screen* instance, UI::Interactions::MouseScroll type) { /* default behaviour is to do no thing, override this function */ };
            /// Context Menu function. When right-clicked.
            virtual void context_menu(UI::Screen* instance) { /* no default predefined behaviour */ };
        };
        Screen* scr;
        std::size_t Screen::width = 0;
        std::size_t Screen::height = 0;
        bool Screen::initialized = false;
        void Screen::handle_resize(int sig) {
            __functree_call__(__FILE__, __LINE__, Gmeng::UI::Screen::handle_resize);
            Gmeng::UI::Screen::refresh_width_height();
        }

        void Screen::refresh_width_height() {
            __functree_call__(__FILE__, __LINE__, Gmeng::UI::Screen::refresh_width_height);
            auto sizes = get_char_sizes();
            wcharsizex = sizes.x;
            wcharsizey = sizes.y;
            clear();
            refresh();
        }

        bool Screen::_uhas_space(Renderer::drawpoint position, Renderer::drawpoint size) {
            return position.y + size.y < LINES && position.x + size.x < COLS;
        }


        void Screen::initialize() {
            __functree_call__(__FILE__, __LINE__, Gmeng::UI::Screen::initialize);
            if (Gmeng::UI::Screen::initialized) {
                if (global.debugger) gm_slog(YELLOW, "DEBUGGER", "Screen cannot be initialized, a previous one already exists.");
                return;
            };
            this->last_mmpos = {-1,-1};
            UI::scr = this;
            this->handles_input = true;
            this->input_handler = NULL;
            initscr(); cbreak(); noecho(); keypad(stdscr, TRUE);
            mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
            start_color(); curs_set(0);
            setlocale(LC_ALL, "");
            signal(SIGWINCH, Gmeng::UI::Screen::handle_resize);
            Gmeng::UI::Screen::refresh_width_height();
            mouseinterval(0);
            use_default_colors();
            init_pair(UI_WHITE, COLOR_WHITE, COLOR_BLACK);
            init_pair(UI_BLUE, COLOR_BLUE, COLOR_BLACK);
            init_pair(UI_GREEN, COLOR_GREEN, COLOR_BLACK);
            init_pair(UI_CYAN, COLOR_CYAN, COLOR_BLACK);
            init_pair(UI_RED, COLOR_RED, COLOR_BLACK);
            init_pair(UI_PINK, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(UI_YELLOW, COLOR_YELLOW, COLOR_BLACK);
            init_pair(UI_BLACK, COLOR_BLACK, COLOR_BLACK);
            init_pair(UI_BGBLACK, COLOR_WHITE, COLOR_BLACK);
            init_pair(UI_BGPINK, COLOR_WHITE, COLOR_MAGENTA);
            init_pair(UI_BGWHITE, COLOR_BLACK, COLOR_WHITE);
            init_pair(UI_BGBLUE, COLOR_WHITE, COLOR_BLUE);
            init_pair(UI_BGCYAN, COLOR_BLACK, COLOR_CYAN);
            init_pair(UI_BGGREEN, COLOR_BLACK, COLOR_GREEN);
            init_pair(UI_BGRED, COLOR_WHITE, COLOR_RED);
            init_pair(UI_BGYELLOW, COLOR_BLACK, COLOR_YELLOW);
            short orange = 200;
            init_color(orange, 246, 128, 25);
            init_pair(25, orange, COLOR_BLACK); // fg
            init_pair(26, COLOR_BLACK, orange); // bg
            if (_gutil_get_terminal() == "iTerm.app") {
                this->text("WARNING! iTerm/iTerm2 does not support RIGHT-CLICK functionality! Some functionality may not work consistently or expectedly.\n[PRESS ENTER TO CONTINUE]");
                while(true) {
                    int ch = getch();
                    if (ch == 10) break; // 10 == enter
                };
            };
            if (_gutil_get_terminal() != "Terminal.app" && _gutil_get_terminal() != "tmux") {
                endwin();
                std::string __text = "ERR_ENGINE_WILL_NOT_COEXIST, ERR_INCOOPERABLE_TERMINAL" + Gmeng::resetcolor + " Your current Terminal Application (" + _gutil_get_terminal() + ") is not suitable for the Gmeng::UI::Screen utility due to its inconsistent handling of screen manipulations." + Gmeng::resetcolor + Gmeng::colors[YELLOW] + " Switch to your default, OS-Provided Terminal Application or use TMUX Instead, the latter being the best choice as TMUX offers right-click functionality, and run on a fullscreen window with a font size of 21.";
                std::cout << Gmeng::bgcolors[4] << __text << Gmeng::resetcolor << std::endl;
                exit(1);
            };
            nodelay(stdscr, true);
            this->report_status = false;
            this->loopfunction = [&](Renderer::drawpoint) -> void {};
            Gmeng::UI::Screen::initialized = true;
        }

        void Screen::destroy() {
            __functree_call__(__FILE__, __LINE__, Gmeng::UI::Screen::destroy);
            endwin();
            Gmeng::UI::Screen::initialized = false;
        }
        void Screen::recv_mouse() {
            __functree_call__(__FILE__, __LINE__, Gmeng::UI::Screen::recv_mouse);
            MEVENT event;
            bool mpos_show = false;
            while (true) {
                int ch = getch();
                if (!this->handles_input && ch != KEY_MOUSE && ch != ERR) {
                    this->input_handler->text_recv(this, (char)ch);
                    if (ch == 27) { // ESC key
                        this->handles_input = true;
                        this->input_handler->refresh(this, NONE);
                        this->input_handler = NULL;
                    };
                    continue;
                };
                if (ch == 'q') { endwin(); exit(2); };
                if (ch == 27) {
                    for (auto &elem : this->elements) {
                        elem->clicked = false;
                        elem->refresh(this, NONE);
                    };
                };
                if (ch == 'M') mpos_show = !mpos_show, this->modify_scr("           ", UI_BLACK, UI_BGBLACK, false, {0,0});
                if (ch == KEY_MOUSE) {
                    if (getmouse(&event) == OK) {
                        Renderer::drawpoint pos{ event.x, event.y };
                        this->mmpos = pos; // quick & dirty patch to fix overlap when click/hovering
                        if (event.bstate & (BUTTON1_PRESSED)) {
                            handle_left_click(pos);
                        }
                        else if (event.bstate & BUTTON3_PRESSED) {
                            handle_right_click(pos);
                        }
                        else if (event.bstate & BUTTON4_PRESSED) {
                            handle_scroll_up(pos);
                        }
                        else if (event.bstate & BUTTON5_PRESSED) { //ignore error here, ncurses thing. will compile.
                            handle_scroll_down(pos);
                        };
                    };
                    this->_refresh();
                };
                bool viable_window = get_window_id() != 0;
                bool do_check = true;
                if (viable_window) this->mmpos = get_pointed_pos(get_mouse_pos());
                if (viable_window) {
                    if (this->last_mmpos.x == this->mmpos.x && this->last_mmpos.y == this->mmpos.y) do_check = false;
                    else this->last_mmpos = this->mmpos;
                };
                if (viable_window && do_check) this->check_hover_states(mmpos); // checks the hover states of all elements within the screen.
                if (viable_window && report_status) this->loopfunction(mmpos);
                if (viable_window && mpos_show) this->modify_scr("pos: " + v_str(mmpos.x) + "," + v_str(mmpos.y) + "    ", UI_WHITE, UI_BGBLACK, false, {0,0});
            };
        };

        Renderer::drawpoint Screen::get_current_pos() {
            __functree_call__(__FILE__, __LINE__, Gmeng::UI::Screen::get_current_pos);
            int x, y;
            getyx(stdscr, y, x);
            std::cout << x << y;
            this->current_cursor_pos = { x,y };
            return { x, y };
        }

        void Screen::report_mouse_pos(bool state) {
            __functree_call__(__FILE__, __LINE__, Gmeng::UI::Screen::report_mouse_pos);
            this->should_report_mouse = state;
            std::thread t_mousepos = Gmeng::_ucreate_thread([&]() {
                while (this->should_report_mouse) {
                    MEVENT event;
                    int ch = getch();
                    if (ch == KEY_MOUSE) {
                        if (getmouse(&event) == OK) this->mouse_pos = { event.x, event.y };
                    };
                };
            });
        }

        Renderer::drawpoint Screen::get_mouse_pointer_pos() {
            __functree_call__(__FILE__, __LINE__, Gmeng::UI::Screen::get_mouse_pointer_pos);
            return this->mouse_pos;
        };

        void Screen::set_cursor(Renderer::drawpoint dp) {
            //__functree_call__(__FILE__, __LINE__, Gmeng::UI::Screen::set_cursor);
            move(dp.y, dp.x);
            this->current_cursor_pos = dp;
        };

        void Screen::modify_scr(std::string text, short fgcolor_pair, short bgcolor_pair, bool reverse_bg, Renderer::drawpoint pos) {
            auto lines = g_splitStr(text, "\n");
            auto cpos = this->current_cursor_pos;
            this->set_cursor(pos);
            if (reverse_bg) attron(COLOR_PAIR(bgcolor_pair | A_REVERSE));
            else attron(COLOR_PAIR(bgcolor_pair));
            for (std::size_t i = 0; i < lines.size(); i++) {
                this->set_cursor({ pos.x, static_cast<int>(pos.y + i) });
                wprintw(stdscr, lines[i].c_str());
            };
            this->set_cursor(cpos);
        }

        void Screen::modify_scr(chtype ch, short fgcolor_pair, short bgcolor_pair, bool reverse_bg, Renderer::drawpoint pos) {
            auto cpos = this->current_cursor_pos;
            this->set_cursor(pos);
            if (reverse_bg) attron(COLOR_PAIR(bgcolor_pair | A_REVERSE));
            else attron(COLOR_PAIR(bgcolor_pair));
            waddch(stdscr, ch);
            if (reverse_bg) attroff(COLOR_PAIR(bgcolor_pair | A_REVERSE));
            else attroff(COLOR_PAIR(bgcolor_pair));
            this->set_cursor(cpos);
        }

        void Screen::modify_scr(const wchar_t* str, short fgcolor_pair, short bgcolor_pair, bool reverse_bg, Renderer::drawpoint pos) {
            auto cpos = this->current_cursor_pos;
            this->set_cursor(pos);
            if (reverse_bg) attron(COLOR_PAIR(bgcolor_pair | A_REVERSE));
            else attron(COLOR_PAIR(bgcolor_pair));
            mvaddnwstr(pos.y, pos.x, str, wcslen(str));
            if (reverse_bg) attroff(COLOR_PAIR(bgcolor_pair | A_REVERSE));
            else attroff(COLOR_PAIR(bgcolor_pair));
            this->set_cursor(cpos);
        };

        void Screen::text(std::string text, short fgcolor, short bgcolor, Renderer::drawpoint pos) {
            if (pos.x != -1 && pos.y != -1) {
                current_cursor_pos = get_current_pos();
                set_cursor(pos);
            };
            attron(COLOR_PAIR(fgcolor));
            bkgdset(bgcolor);
            wprintw(stdscr, text.c_str());
            attroff(COLOR_PAIR(fgcolor));
            bkgdset(bgcolor);
            if (pos.x != -1 && pos.y != -1) {
                set_cursor(this->current_cursor_pos); // restore old cursor position
            };
        }

        template<typename T>
        bool Screen::add_element(std::unique_ptr<T> __a) {
            __functree_call__(__FILE__, __LINE__, Gmeng::UI::Screen::add_element);
            // Implementation for creating a menu (subclass)
            // Background color, position, and buttons will be used here
            if (!this->_uhas_space(__a->position, { static_cast<int>(__a->width), static_cast<int>(__a->height) })) {
                if (global.debugger) gm_slog(YELLOW, "DEBUGGER", "STDSCR does not have enough space to create subclass.");
                return false;
            }
            else {
                this->elements.push_back(std::move(__a));
                this->_refresh();
                return true;
            };
        }

        void Screen::_refresh() {
            clear();
            auto v = get_current_pos();
            for (auto& elem : this->elements) {
                if (elem->clicked) { elem->refresh(this, CLICKED); continue; };
                elem->refresh(this, elem->hovered ? HOVERED : NONE);
            };
            refresh();
        };

        void Screen::__refresh() { __functree_call__(__FILE__, __LINE__, Gmeng::UI::Screen::__refresh); refresh(); };

        void Screen::check_hover_states(Renderer::drawpoint mpos) {
            //__functree_call__(__FILE__, __LINE__, Gmeng::UI::Screen::check_hover_states);
            for (auto& elem : this->elements) {
                if ( viewpoint_includes_dp(UI::Element::get_viewpoint(*elem), mpos) ) {

                    if (elem->hidden) continue;
                    elem->hover(this, true);
                } else elem->hovered = elem->clicked = false, elem->hover(this, false);
            };
        };

        void Screen::handle_left_click(Renderer::drawpoint pos) {
            __functree_call__(__FILE__, __LINE__, Gmeng::UI::Screen::__private__::handle_left_click);
            if (!this->handles_input && this->input_handler != NULL) {
                if (!viewpoint_includes_dp(UI::Element::get_viewpoint(*this->input_handler), pos)) {
                    this->handles_input = true;
                    this->input_handler->clicked = false;
                    this->input_handler = NULL;
                };
            };
            for (auto& elem : this->elements) {
                bool v = viewpoint_includes_dp({
                    .start = elem->position,
                    .end = { static_cast<int>(elem->position.x + elem->width), static_cast<int>(elem->position.y + elem->height - 1) }
                    }, { pos.x,pos.y });
                if (v) {
                    if (elem->hidden) continue;
                    elem->click(this, LEFT);
                    // break; // commented because i'm not sure if ActionMenus would function properly
                }
                else { elem->hovered = elem->clicked = false; }; // in case the element does not handle it internally
            };
        }

        void Screen::handle_right_click(Renderer::drawpoint pos) {
            __functree_call__(__FILE__, __LINE__, Gmeng::UI::Screen::__private__::handle_right_click);
            for (auto& elem : this->elements) {
                bool v = viewpoint_includes_dp(Element::get_viewpoint(*elem), pos);
                if (v) { elem->context_menu(this); };
            };
        };

        void Screen::handle_scroll_up(Renderer::drawpoint pos) {
            __functree_call__(__FILE__, __LINE__, Gmeng::UI::Screen::__private__::handle_scroll_up);
            for (auto& elem : this->elements) {
                bool v = viewpoint_includes_dp(Element::get_viewpoint(*elem), pos);
                if (v) { elem->mouse_scroll(this, MouseScroll::UP); break; };
            };
        };

        void Screen::handle_scroll_down(Renderer::drawpoint pos) {
            __functree_call__(__FILE__, __LINE__, Gmeng::UI::Screen::__private__::handle_scroll_down);
            for (auto& elem : this->elements) {
                bool v = viewpoint_includes_dp(Element::get_viewpoint(*elem), pos);
                if (v) { elem->mouse_scroll(this, MouseScroll::DOWN); break; };
            };
        };

        struct InfoBox : public UI::Element {
          using UI::Element::Element;
          using dp = Renderer::drawpoint;
          using func = std::function<std::string(InfoBox*)>;
          private:
            func _upd_func; int last_c_event = 0;
          public:
            std::string data = "";
            InfoBox(dp pos, func _func, std::size_t width, std::size_t height) : Element() {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::InfoBox);
                this->position = pos;
                this->width = width;
                this->height = height;
                this->hidden = false;
                this->_upd_func = _func;
                this->_upd_func(this);
            };

            inline void refresh(UI::Screen* instance, ButtonInteraction type) override {
                if (GET_TIME() >= last_c_event+1000) last_c_event = GET_TIME();
                else return;
                this->_upd_func(this);
                std::vector<std::string> texts = g_splitStr(data, "\n");
                for (const auto& str : g_splitStr(data, "\n")) {
                    if (texts.size() >= height-2) break;
                    std::string fin = str;
                    if (fin.length() > width-4) {
                        while(fin.length() > width-7) fin.pop_back();
                        fin += "...";
                    };
                    texts.push_back(fin);
                };
                wchar_t* top = concat_wstr(L"┌", concat_wstr(repeat_wstring(L"─", width-2), L"┐"));
                wchar_t* bottom = concat_wstr(L"└", concat_wstr(repeat_wstring(L"─", width-2), L"┘"));
                instance->modify_scr(top, UI_WHITE, UI_WHITE, false, position);
                int i=0;
                for (const auto& txt : texts) {
                    auto txt1 = txt;
                    while(txt1.length() < width-4) txt1 += " ";
                    if (txt1.length() > width-4) {
                        while(txt1.length() > width-7) txt1.pop_back();
                        txt1 += "...";
                    };
                    wchar_t* wtext = concat_wstr(concat_wstr(L"│ ", txt1), L" │");
                    instance->modify_scr(wtext, UI_WHITE, UI_WHITE, false, {position.x, position.y+i+1});
                    i++;
                };
                instance->modify_scr(bottom, UI_WHITE, UI_WHITE, false, {position.x,position.y+(int)height-1});
            };
        };

        struct Button : public UI::Element {
          protected:
              const UI::Element::Types type = UI::Element::BUTTON;
          private:
            std::function<void(UI::Button* button, UI::Interactions::ButtonInteraction cb)> click_method;
          public:
            using UI::Element::Element; bool compact = false;
            Button(Renderer::drawpoint pos, std::string title, bool compact, short fg_color, short bg_color, std::function<void(UI::Button* button, UI::Interactions::ButtonInteraction cb)> fn) : Element() {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::Button);
                this->position = pos; this->title = title;
                this->foreground_color = fg_color; this->background_color = bg_color;
                this->foreground_color_highlight = *(&this->foreground_color);
                this->background_color_highlight = *(&this->background_color);
                this->foreground_color_click = *(&this->foreground_color);
                this->background_color_click = *(&this->background_color);
                this->click_method = fn;
                this->width = title.length() + 4;
                this->height = compact ? 1 : 3;
                this->compact = compact;
                this->hidden = false; // SHOW.
            };
            inline void text_recv(UI::Screen* instance, char ch) override {};
            inline void click(UI::Screen* instance, UI::Interactions::MouseButton button) override {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::Button::click);
                this->clicked = button == LEFT ? true : false;
                switch (button) {
                    case LEFT:
                        this->refresh(instance, CLICKED);
                        this->click_method(this, CLICKED);
                        break;
                    case RIGHT:
                        this->refresh(instance, CONTEXT_MENU);
                        this->click_method(this, CONTEXT_MENU);
                        break;
                };
                instance->__refresh();
                auto future_thing = std::async(std::launch::async, [&]() -> void {
                    sleep(ms(125));
                    this->clicked = false;
                });
            };

            inline void hover(UI::Screen* instance, bool state) override {
                if (state && !this->hovered) __functree_call__(__FILE__, __LINE__, Gmeng::UI::Button::hover);
                this->hovered = state;
                if (this->clicked) return; // if clicked, ignore hover color.
                if (state) this->refresh(instance, HOVERED);
                else this->refresh(instance, NONE);
            };
            inline void refresh(UI::Screen* instance, UI::Interactions::ButtonInteraction type) override {
                if (this->hidden) return;
                struct {
                    bool highlight = false;
                    bool reverse_bg = false;
                } modifiers;
                switch (type) {
                    default:
                    case NONE:
                        break;
                    case HOVERED:
                        modifiers.highlight = true;
                        break;
                    case CLICKED: case CONTEXT_MENU:
                        modifiers.reverse_bg = true;
                        break;
                };
                if (this->title.length() > 25 || this->title.find("\n", 0) != std::string::npos) {
                    gm_log("could not refresh BUTTON instance, '" + this->title + "' is invalid. Max 25 characters allowed. Newlines are disallowed.");
                    return;
                };
                short applied_fg = modifiers.highlight  ? this->foreground_color_highlight : this->foreground_color;
                short highlighted_bg = modifiers.highlight ? this->background_color_highlight : this->background_color;
                short applied_bg = highlighted_bg;
                if (this->compact) {
                    wchar_t* wcompact = concat_wstr(concat_wstr(L"┤",this->title), L"├");
                    instance->modify_scr(wcompact, modifiers.reverse_bg ? this->foreground_color_click : applied_fg, modifiers.reverse_bg ? this->background_color_click : applied_bg, false, this->position);
                } else {
                    wchar_t* wtop    = concat_wstr(concat_wstr(L"┌", repeat_wstring(L"─",(int)this->width-2)), L"┐");
                    wchar_t* wtitle  = concat_wstr(concat_wstr(L"│ ", this->title), L" │");
                    wchar_t* wbottom = concat_wstr(concat_wstr(L"└", repeat_wstring(L"─",(int)this->width-2)), L"┘");
                    instance->modify_scr(wtop, modifiers.reverse_bg ? this->foreground_color_click : applied_fg, modifiers.reverse_bg ? this->background_color_click : applied_bg, false, this->position);
                    instance->modify_scr(wtitle, modifiers.reverse_bg ? this->foreground_color_click : applied_fg, modifiers.reverse_bg ? this->background_color_click : applied_bg, false, {this->position.x,this->position.y+1});
                    instance->modify_scr(wbottom, modifiers.reverse_bg ? this->foreground_color_click : applied_fg, modifiers.reverse_bg ? this->background_color_click : applied_bg, false, {this->position.x,this->position.y+2});
                };
            };
        };

        class small_render_t {
          public:
            std::size_t width = 2; std::size_t height = 2;
            Gmeng::texture contents = Renderer::generate_empty_texture(this->width, this->height);
            small_render_t(Gmeng::texture txtr) : width(txtr.width), height(txtr.height), contents(txtr) { __functree_call__(__FILE__, __LINE__, Gmeng::UI::small_render_t); };
            small_render_t() = default;
            enum partial_type {
                END_OF_HLINE = 0, // end of line, skip to next line
                CONTINUE     = 1, // unit
                UCORNER      = 2, // upper corner
                LCORNER      = 3, // lower corner
                VLINE        = 4  // vertical line
            };
            struct partial_render_obj {
                enum partial_type  type;
                struct Gmeng::Unit unit = Gmeng::Unit { .transparent=true };
                struct Renderer::drawpoint image_size;
            };
            std::vector<partial_render_obj> get() {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::small_render_t::get);
                std::vector<partial_render_obj> v_partials;
                for (int i = 0; i < this->contents.height; i++) {
                    for (int j = 0; j < this->contents.width; j++) {
                        v_partials.push_back({
                            CONTINUE,
                            this->contents.units[(i*this->width)+j],
                            {(int)this->width,(int)this->height}
                        });
                    };
                    v_partials.push_back({ END_OF_HLINE });
                };
                return v_partials;
            };
            std::vector<partial_render_obj> with_frame() {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::small_render_t::with_frame);
                std::vector<partial_render_obj> v_partials;
                repeat(this->width+2, [&]() { v_partials.push_back({ UCORNER }); });
                v_partials.push_back({ END_OF_HLINE });
                for (int i = 0; i < this->contents.height; i++) {
                    v_partials.push_back({ VLINE });
                    for (int j = 0; j < this->contents.width; j++) {
                        v_partials.push_back({
                            CONTINUE,
                            this->contents.units[(i*this->width)+j],
                            {(int)this->width,(int)this->height}
                        });
                    };
                    v_partials.push_back({ VLINE });
                    v_partials.push_back({ END_OF_HLINE });
                };
                repeat(this->width+2, [&]() { v_partials.push_back({ LCORNER }); });
                v_partials.push_back({ END_OF_HLINE });
                return v_partials;
            };
            struct viable_unit_transform_t {
                const wchar_t* ch;
                uicolor_t color;
            };
            typedef struct {
                std::size_t MAX_WIDTH; std::size_t MAX_HEIGHT;
            } max_values;
            static std::vector<std::vector<viable_unit_transform_t>> do_render(std::vector<partial_render_obj> image, max_values max = {0, 0}) {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::small_render_t::__static__::do_render);
                std::vector<std::vector<viable_unit_transform_t>> render;
                std::vector<viable_unit_transform_t> p;
                int c;
                bool v_check_maxvalue = max.MAX_HEIGHT != 0 && max.MAX_WIDTH != 0;
                for (const auto& partial : image) {
                    switch (partial.type) {
                        case UCORNER:
                            p.push_back({wc_outer_unit,UI_CYAN});
                            break;
                        case LCORNER:
                            p.push_back({wc_outer_unit_floor,UI_CYAN});
                            break;
                        case VLINE:
                            p.push_back({wc_unit,UI_CYAN});
                            break;
                        case END_OF_HLINE:
                            render.push_back(p);
                            p.clear();
                            break;
                        default:
                        case CONTINUE:
                            p.push_back({wc_unit,stoui_color[partial.unit.transparent ? BLACK : partial.unit.color]});
                            break;
                    };
                    c++;
                };
                return render;
            };
        }; // should move to renderer.cpp

        struct Switch : public UI::Element {
          using UI::Element::Element;
          using dp = Renderer::drawpoint;
          public:
            bool enabled = false;
            std::function<void(Switch*)> change_func = [](Switch*) -> void {};
            operator bool() const { return this->enabled; };
            operator int() const { return (int)this->enabled; };
            operator std::string() const { return this->enabled ? "YES" : "NO"; };
            Switch(dp pos, std::string name, uicolor_t textcolor = UI_WHITE, uicolor_t textcolor_highlight = UI_CYAN, bool enabled = false) : Element() {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::Switch);
                this->position = pos;
                this->title = name + " ";
                this->hidden = false;
                this->enabled = enabled;
                this->width = (int)this->title.length() + 3;
                this->height = 1;
                this->background_color = textcolor;
                this->background_color_highlight = textcolor_highlight;
                this->background_color_click = UI_WHITE;
                this->foreground_color = this->foreground_color_highlight = this->foreground_color_click = UI_BLACK;
            };
            inline void hover(UI::Screen* instance, bool state) override {
                if (state && !this->hovered) __functree_call__(__FILE__, __LINE__, Gmeng::UI::Switch::hover);
                this->hovered = state; this->refresh(instance, this->hovered ? HOVERED : NONE);
            };
            inline void click(UI::Screen* instance, MouseButton) override {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::Switch::click);
                this->enabled = !this->enabled;
                this->refresh(instance, HOVERED);
                this->change_func(this);
            };
            inline void text_recv(UI::Screen*, char) override {};
            inline void refresh(UI::Screen* instance, ButtonInteraction type) override {
                short hv_color = type == HOVERED ? this->background_color_highlight : this->background_color;
                if (!this->enabled) {
                    instance->modify_scr(this->title, UI_BLACK, hv_color, false, this->position);
                    instance->modify_scr(wc_unit, UI_BLACK, UI_WHITE, false, {this->position.x+(int)this->title.length(), this->position.y});
                    instance->modify_scr(L"🀰🀰", UI_BLACK, UI_RED, false, {this->position.x+(int)this->title.length()+1, this->position.y});
                } else {
                    instance->modify_scr(this->title, UI_BLACK, hv_color, false, this->position);
                    instance->modify_scr(L"🀰🀰", UI_BLACK, UI_GREEN, false, {this->position.x+(int)this->title.length(), this->position.y});
                    instance->modify_scr(wc_unit, UI_BLACK, UI_WHITE, false, {this->position.x+(int)this->title.length()+2, this->position.y});
                };
            };
        };

        struct Hoverable : public UI::Element {
            using dp = Renderer::drawpoint;
          private:
            bool HAS_ENOUGH_SPACE = true;
            std::size_t MAX_SPACE_X = -1;
            std::size_t MAX_SPACE_Y = -1;
            bool oo_refresh_after_stopped_hovering = false;
          public:
            using UI::Element::Element;
            small_render_t render;
            Hoverable(dp pos,std::string title, small_render_t render, uicolor_t color_base, uicolor_t color_hovered) : Element() {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::Hoverable);
                this->render = render;
                this->position = pos;
                this->hidden = false;
                this->title = title;
                this->width = title.length();
                this->height = 1;
                this->foreground_color = color_base;
                this->background_color = color_base;
                this->foreground_color_highlight = color_hovered;
                this->background_color_highlight = color_hovered;
                this->foreground_color_click = color_hovered;
                this->background_color_click = color_hovered;
            };
            Hoverable() = default;
            inline void click(UI::Screen* instance, Interactions::MouseButton button) override {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::Hoverable::click);
                /// NO FUNCTIONALITY;
            };
            inline void text_recv(UI::Screen* instance, char ch) override {};
            inline void hover(UI::Screen* instance, bool state) override {
                if (state == false) {
                    this->hovered = state;
                    this->refresh(instance, NONE);
                } else {
                    __functree_call__(__FILE__, __LINE__, Gmeng::UI::Hoverable::hover);
                    this->hovered = state;
                    this->refresh(instance, HOVERED);
                };
            };

            inline void refresh(UI::Screen* instance, Interactions::ButtonInteraction type) override {
                if (this->hidden || type == CLICKED) return;
                if (type != HOVERED && this->oo_refresh_after_stopped_hovering) {
                    clear();
                    this->hidden = true;
                    instance->_refresh();
                    this->hidden = false;
                    this->oo_refresh_after_stopped_hovering = false;
                };
                if (type != HOVERED) {
                    instance->modify_scr("[" + this->title + "]", this->foreground_color, this->background_color, false, this->position);
                    return;
                };
                clear();
                this->hidden = true;
                instance->_refresh();
                this->hidden = false;
                instance->modify_scr("[" + this->title + "]", this->foreground_color_highlight, this->background_color_highlight, false, this->position);
                //// ^^ set the element's color before showing the image
                std::size_t* render_width  = &this->render.width;
                std::size_t* render_height = &this->render.height;
                Renderer::drawpoint mouse_pos = instance->mmpos;
                int cursor_x = mouse_pos.x;
                int cursor_y = mouse_pos.y;
                int space_below = LINES - cursor_y;
                int space_above = cursor_y;
                int ABOVE_OR_BELOW = 0; // 0 for below, 1 for above.
                if (space_below >= v_static_cast<int>(*render_height)) { //dereference height before casting
                    HAS_ENOUGH_SPACE = true;
                    MAX_SPACE_X = -1;
                    MAX_SPACE_Y = -1;
                } else if (space_above >= v_static_cast<int>(*render_height)) {
                    HAS_ENOUGH_SPACE = true;
                    MAX_SPACE_X = -1;
                    MAX_SPACE_Y = -1;
                    ABOVE_OR_BELOW = 1;
                } else {
                    HAS_ENOUGH_SPACE = false;
                    MAX_SPACE_X = COLS - cursor_x;
                    MAX_SPACE_Y = std::max(space_above, space_below);
                };
                std::vector<std::vector<small_render_t::viable_unit_transform_t>> render_output;
                if (!HAS_ENOUGH_SPACE) {
                    std::vector _NOSPACE = small_render_t::do_render(this->render.with_frame(), {MAX_SPACE_X, MAX_SPACE_Y});
                    render_output = _NOSPACE;
                } else {
                    std::vector _SPACE   = small_render_t::do_render(this->render.with_frame());
                    render_output = _SPACE;
                };
                /// DRAW IMAGE (according to ABOVE_OR_BELOW)
                if (ABOVE_OR_BELOW == 1) {
                    // ABOVE
                    int margin_Y = (cursor_y-render_output.size());
                    int margin_X = (cursor_x);                      // asuming each line is the same length.
                    for (int i = render_output.size(); i < 0; i--) {
                        /// Y values
                        int absolute_Y = (render_output.size()-i);          // value of how long we moved Y
                        if (cursor_y-absolute_Y-margin_Y <= 0) break;       // stop if we have hit 0th Y coord
                        auto vec_units = render_output[i];
                        for (int j = 0; j < vec_units.size(); i++) {
                            /// X values
                            int absolute_X = j;                               // value of how long we moved X
                            if (cursor_x+absolute_X > (COLS-cursor_x)) break; // stop if we hit MAXth X coord
                            auto unit = vec_units[j];
                            attron(COLOR_PAIR(unit.color));
                            auto addv = mvaddnwstr(margin_Y+i,margin_X+j,unit.ch,wcslen(unit.ch));
                            attroff(COLOR_PAIR(unit.color));
                        };
                    };
                } else {
                    // BELOW
                    int margin_Y = (cursor_y);
                    int margin_X = (cursor_x);
                    for (int i = 0; i < render_output.size(); i++) {
                        /// Y values
                        int absolute_Y = i;
                        if (cursor_y+i > (LINES)) break;    // stop if we hit MAXth Y coord
                        auto vec_units = render_output[i];
                        for (int j = 0; j < vec_units.size(); j++) {
                            /// X values
                            int absolute_X = j;
                            if (cursor_x+absolute_X > (COLS-cursor_x)) break; // stop if we hit MAXth X coord
                            auto unit = vec_units[j];
                            attron(COLOR_PAIR(unit.color));
                            auto addv = mvaddnwstr(margin_Y+i,margin_X+j,unit.ch,wcslen(unit.ch));
                            attroff(COLOR_PAIR(unit.color));
                        };
                    };
                };
                this->oo_refresh_after_stopped_hovering = true; /// clear current image in the next frame
            };
        };

        struct TextController : public UI::Element {
          using UI::Element::Element;
          using dp = Renderer::drawpoint;
          public:
            std::string* ptr; bool locked = false;
            TextController(dp pos, std::string* _string, std::size_t max_width = -1, uicolor_t color = UI_WHITE, uicolor_t highlight_color = UI_BLUE) : Element() {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::TextController);
                this->hidden = false;
                this->width = max_width == -1 ? _string->length() : max_width;
                this->clicked = false;
                this->background_color = color;
                this->background_color_highlight = color;
                this->ptr = _string;
            };
            inline void hover(UI::Screen*, bool) override {};
            inline void click(UI::Screen* instance, MouseButton button) override {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::TextController::click);
                if (this->clicked || this->locked) return;
                if (instance->input_handler != NULL) {
                    instance->input_handler->clicked = false;
                    instance->input_handler = NULL;
                };
                this->clicked = true;
                instance->handles_input = false;
                instance->input_handler = std::shared_ptr<UI::Element>(this, [](UI::Element*) {});
                this->refresh(instance, CLICKED);
            };
            inline void text_recv(UI::Screen* instance, char ch) override {
                if (this->locked) return;
                if (ch == 27) { /// ESC key
                    this->clicked = false;
                    instance->handles_input = true;
                    instance->input_handler = nullptr;
                    this->refresh(instance, NONE);
                    return;
                } else {
                    if (this->ptr->length() == this->width && ch != 7 && ch != 10) return;
                    switch (ch) {
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                            break;
                        case 9: /// TAB
                            this->ptr->push_back(' ');
                            this->refresh(instance, CLICKED);
                            break;
                        case 10: /// Enter
                            this->refresh(instance, CLICKED);
                            break;
                        case 82:
                        case 83:
                            break;
                        case 74: /// DEL key
                        case 7: /// backspace
                            if (this->ptr->length() == 0) break;
                            this->ptr->pop_back();
                            this->refresh(instance, CLICKED);
                            break;
                        default:
                            this->ptr->push_back(ch);
                            this->refresh(instance, CLICKED);
                            break;
                    };
                };
            };
            inline void refresh(UI::Screen* instance, Interactions::ButtonInteraction type) override {
                wchar_t* w_lnU = repeat_wstring(wc_outer_unit, this->width);
                if (type != CLICKED) {
                    std::string w_ln = *this->ptr + repeatString(" ", (int)this->width-this->ptr->length());
                    instance->modify_scr(w_ln, UI_WHITE, this->background_color, false, this->position);
                } else {
                    wchar_t* w_ln = concat_wstr(concat_wstr(concat_wstr(L"", *this->ptr), this->ptr->length() == this->width ? L"" : wc_unit), repeatString(" ", this->width-this->ptr->length()-1));
                    instance->modify_scr(w_ln, UI_WHITE, this->background_color_highlight, false, this->position);
                };
            };
        };

        // increments and decrements integer
        struct NumButton : public UI::Element {
          using UI::Element::Element;
          using dp = Renderer::drawpoint;
          enum type_t { INC = 0, DEC = 1 };
          public:
            int* ptr = nullptr; type_t type; bool locked = false;
            NumButton(dp pos, int* ptr, type_t type, uicolor_t color = UI_CYAN, uicolor_t color_highlight= UI_WHITE) : Element() {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::NumButton);
                this->hidden = false;
                this->width = this->height = 1;
                this->ptr = ptr;
                this->position = pos;
                this->type = type;
                this->title = "NUM" + std::string(type?"INC":"DEC")+"BUTTON_" + v_str(g_mkid());
                this->background_color = color;
                this->background_color_highlight = color_highlight;
            };
        };

        struct NavButton : public UI::Element {
          using UI::Element::Element;
          using dp = Renderer::drawpoint;
          using func = std::function<void(UI::NavButton*)>;
          enum direction_t { LEFT = 0, RIGHT = 1 };
          public:
            direction_t direction = LEFT; func click_func;
            NavButton(dp pos, direction_t direction, uicolor_t color, uicolor_t color_highlight, uicolor_t color_click, func click_func) : Element() {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::NavButton);
                this->position = pos;
                this->width = 1;
                this->height = 1;
                this->title = "NAV" + std::string(direction == 0 ? "LEFT" : "RIGHT") + "BUTTON_" + v_str(g_mkid());
                this->direction = direction;
                this->background_color = color;
                this->background_color_highlight = color_highlight;
                this->background_color_click = color_click;
                this->click_func = click_func;
            };
            inline void text_recv(UI::Screen*, char) override {};
            inline void hover(UI::Screen* instance, bool state) override {
                if (state && !this->hovered) __functree_call__(__FILE__, __LINE__, Gmeng::UI::NavButton::hover);
                this->hovered = state;
                this->refresh(instance, HOVERED);
            };
            inline void click(UI::Screen* instance, MouseButton button) override {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::NavButton::click);
                instance->modify_scr(v_str(g_mkid()) + "YO VAR", UI_WHITE, UI_GREEN, false, {0,LINES-5});
                this->click_func(this);
            };
            inline void refresh(UI::Screen* instance, ButtonInteraction type) override {
                short _color = background_color;
                if (type == HOVERED) _color = background_color_highlight;
                instance->modify_scr(direction == LEFT ? ACS_LARROW : ACS_RARROW, UI_WHITE, _color, false, this->position);
            };
        };

        struct SettingsMenu : public UI::Element {
          using UI::Element::Element;
          using dp = Renderer::drawpoint;
          private:
          std::vector<std::vector<std::unique_ptr<UI::Element>>> self_elements;
          UI::NavButton nav_l; UI::NavButton nav_r;
          public:
            void hndl_l(NavButton*) {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::SettingsMenu::page_left);
                if (this->mpage-1 < 0) { this->mpage = this->self_elements.size()-1; return; };
                this->mpage--;
            };
            void hndl_r(NavButton*) {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::SettingsMenu::page_right);
                if (this->mpage+1 >= this->self_elements.size()) { this->mpage = 0; return; };
                this->mpage++;
            };
            int mpage = 0; int vid = g_mkid();
            SettingsMenu(dp pos, std::string title, std::size_t width, std::size_t height) : Element() {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::SettingsMenu);
                this->position = pos;
                this->title = title;
                this->width = width;
                this->height = height;
                self_elements.push_back({}); /// page 0
                auto navbutton_l = UI::NavButton({2,(int)this->height-3}, NavButton::LEFT, UI_CYAN, UI_WHITE, UI_BLUE, std::bind(&SettingsMenu::hndl_l, this, std::placeholders::_1));
                auto navbutton_r = UI::NavButton({(int)this->width-3,(int)this->height-3}, NavButton::RIGHT, UI_CYAN, UI_WHITE, UI_BLUE, std::bind(&SettingsMenu::hndl_r, this, std::placeholders::_1));

                nav_l = navbutton_l; nav_r = navbutton_r;
            };
            inline void add_element(int page, std::unique_ptr<UI::Element> element) {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::SettingsMenu::add_element);
                if (page >= this->self_elements.size()) this->self_elements.push_back({});
                this->self_elements[page].push_back(std::move(element));
            };
            inline void hover(UI::Screen* instance, bool state) override {
                if (state && !this->hovered) __functree_call__(__FILE__, __LINE__, Gmeng::UI::SettingsMenu::hover);
                this->hovered = state;
                this->refresh(instance, HOVERED);
            };
            inline void click(UI::Screen* instance, MouseButton button) override {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::SettingsMenu::click);
                this->refresh(instance, CLICKED);
            };
            inline void refresh(UI::Screen* instance, ButtonInteraction type) override {
                if (this->hidden) return;
                int c1 = (int)(this->width-this->title.length()-2)/2;
                int c2 = c1;
                while (c1 + c2 != this->width - this->title.length() - 2) c1++;
                instance->modify_scr(concat_wstr(concat_wstr(concat_wstr(L"┌", repeat_wstring(L"─", c1)), this->title), concat_wstr(repeat_wstring(L"─", c2), L"┐")), UI_WHITE, UI_WHITE, false, this->position);
                for (int i = 0; i < this->height-2; i++) {
                    instance->modify_scr(concat_wstr(concat_wstr(L"│", repeatString(" ", this->width-2)), L"│"), UI_WHITE, UI_WHITE, false, { this->position.x,this->position.y+1+i });
                };
                instance->modify_scr(concat_wstr(concat_wstr(L"└", repeat_wstring(L"─", this->width-2)), L"┘"), UI_WHITE, UI_WHITE, false, {this->position.x,this->position.y+(int)this->height-2});

                if (!nav_l.__any_modifier__) {
                    nav_l.__raw_position = nav_l.position;
                    nav_l.__any_modifier__ = true;
                };
                nav_l.position = { this->position.x+nav_l.__raw_position.x,this->position.y+nav_l.__raw_position.y };
                nav_l.click_func = std::bind(&SettingsMenu::hndl_l, this, std::placeholders::_1);
                if (!nav_r.__any_modifier__) {
                    nav_r.__raw_position = nav_r.position;
                    nav_r.__any_modifier__ = true;
                };
                nav_r.click_func = std::bind(&SettingsMenu::hndl_r, this, std::placeholders::_1);
                nav_r.position = { this->position.x+nav_r.__raw_position.x,this->position.y+nav_r.__raw_position.y };
                if (instance->mmpos.x == nav_l.position.x && instance->mmpos.y == nav_l.position.y) {
                    if (type == CLICKED) nav_l.click(instance, LEFT), nav_l.refresh(instance, CLICKED);
                    if (type == HOVERED) nav_l.hover(instance, true);
                } else nav_l.hovered = false, nav_l.refresh(instance, NONE);

                if (instance->mmpos.x == nav_r.position.x && instance->mmpos.y == nav_r.position.y) {
                    if (type == CLICKED) nav_r.click(instance, LEFT), nav_r.refresh(instance, CLICKED);
                    if (type == HOVERED) nav_r.hover(instance, true);
                } else nav_r.hovered = false, nav_r.refresh(instance, NONE);
                std::string _page_text = "[ " + v_str(this->mpage+1) + " // " + v_str(this->self_elements.size()) + " ]";
                int _page_pos = (this->width-_page_text.length())/2;
                instance->modify_scr(_page_text, UI_WHITE, nav_l.background_color_click, false, { this->position.x+(int)((this->width-_page_text.length())/2),nav_l.position.y });                

                for (auto& elem : this->self_elements[this->mpage]) {
                    if (!elem->__any_modifier__) {
                        elem->__raw_position = elem->position;
                        elem->__any_modifier__ = true;
                    };
                    elem->position = {
                        elem->__raw_position.x+this->position.x,
                        elem->__raw_position.y+this->position.y
                    };
                    if (viewpoint_includes_dp(Element::get_viewpoint(*elem), instance->mmpos)) {
                        if (type == CLICKED) elem->click(instance, LEFT);
                        if (type == HOVERED) elem->hover(instance, true);
                        elem->refresh(instance, type);
                    } else elem->hover(instance, false);
                };
            };

            inline void text_recv(UI::Screen*, char) override {};
        };

        struct draginfo_t {
            using dp = Renderer::drawpoint;
            dp startposition = {-15,-15};
            dp endposition = {-15,-15};
        };

        struct Image : public UI::Element {
          using UI::Element::Element;
          using dp = Renderer::drawpoint;
          using vp = Renderer::viewpoint;
          public:
            bool active_context = false; int context_id; bool do_spin = false; bool jabelahe = false;
            bool dragging; bool with_frame = false; vp viewspace = { {0,0}, {0,0} }; bool show_info = true;
            draginfo_t drag_info; small_render_t image; dp camera = { 0,0 }; bool allow_zoom = true; bool follow_cursor = false;
            Image(dp pos, small_render_t _image, bool with_frame = false, uicolor_t frame_color = UI_CYAN, uicolor_t frame_color_click = UI_BLUE) : Element() {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::Image);
                this->position = pos;
                this->hidden = this->clicked = false;
                this->image = _image;
                this->width = _image.width + (with_frame ? 2 : 0);
                this->height= _image.height + (with_frame ? 2 : 0);
                this->with_frame = with_frame;
                this->background_color = frame_color;
                this->background_color_highlight = *(&this->background_color);
                this->background_color_click = frame_color_click;
                this->foreground_color = this->foreground_color_highlight = this->foreground_color_click = UI_BLACK;
                this->camera = {(int)_image.width-1,(int)_image.height-1};
                this->viewspace = { {0,0}, {(int)_image.width-1,(int)_image.height-1} };
            };
            inline void drag(UI::Screen* instance) {
                /// scale with camera
                int MARGINX_CAM = (this->image.width-1  - (this->camera.x))/2;
                int MARGINY_CAM = (this->image.height-1 - (this->camera.y))/2;
                if (this->drag_info.startposition.x == -15) return this->refresh(instance, AUTO);
                if (MARGINX_CAM != 0 && MARGINY_CAM != 0) this->viewspace = {
                    { 0+MARGINX_CAM  ,  0+MARGINY_CAM },
                    { (int)image.width -MARGINX_CAM,
                      (int)image.height-MARGINY_CAM }
                }; else this->viewspace = { {0,0}, {(int)this->image.width-1,(int)this->image.height-1} };
                /// X COORDINATE
                if (this->drag_info.startposition.x >= this->drag_info.endposition.x) {
                    /// user went right
                    int max_distance = this->image.width;
                    if (this->viewspace.end.x < max_distance) this->viewspace.start.x++, this->viewspace.end.x++;
                } else {
                    /// user went left
                    if (this->viewspace.start.x > 0) this->viewspace.start.x--, this->viewspace.end.x--;
                };
                /// Y COORDINATE
                if (this->drag_info.startposition.y >= this->drag_info.endposition.y) {
                    /// user went right
                    int max_distance = this->image.width-_vcreate_vp2d_deltay(this->viewspace);
                    if (this->viewspace.start.y < max_distance) this->viewspace.start.y++;
                } else {
                    /// user went left
                    if (this->viewspace.start.y > 0) this->viewspace.start.y--;
                };
                this->refresh(instance, CONTEXT_MENU);
            };
            inline void hover(UI::Screen* instance, bool state) override {
                if (state && !this->hovered) __functree_call__(__FILE__, __LINE__, Gmeng::UI::Image::hover);
                this->hovered = state;
                if (!state) { this->clicked = false; this->hovered = false; return; };
                if (state && this->clicked) { // clicked AND hovering
                    if (this->drag_info.startposition.x == -15) { /// -15 is the default NO_INFO value
                        this->drag_info = { instance->mmpos, instance->mmpos };
                    } else { /// was already dragging before the function was called
                        this->drag_info = { this->drag_info.startposition, instance->mmpos };
                        this->drag(instance); /// only after the user drags the screen for at least 1 unit
                    };
                };
                if (this->active_context) {
                    for (auto& elem : instance->elements) {
                        if (elem->id == this->context_id &&
                            viewpoint_includes_dp(UI::Element::get_viewpoint(*elem), instance->mmpos)) { elem->refresh(instance, HOVERED); break;}
                        if (elem->id == this->context_id) elem->hovered = false, elem->hidden = true, this->jabelahe = false;
                    };
                };
            };
            inline void click(UI::Screen* instance, MouseButton button) override {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::Image::click);
                if (this->jabelahe) return this->clicked = true, this->refresh(instance, CONTEXT_MENU);
                this->clicked = !this->clicked; /// switch the status, only needed here because other elements handle click() differently
                if (!this->clicked) { //previously clicked
                    this->drag_info = { {-15,-15}, {-15,-15} }; /// reset value
                };
                if (!this->__any_modifier__) {
                    this->__any_modifier__ = true;
                    this->follow_cursor = true;
                };
                this->refresh(instance, CLICKED);
            };
            bool disallow_spin_refresh = false;
            inline void refresh(UI::Screen* instance, ButtonInteraction type) override {
                if ( (this->clicked || type == CLICKED || type == HOVERED) && !viewpoint_includes_dp(Element::get_viewpoint(*this), instance->mmpos) ) return;
                if (this->active_context && type == CLICKED && this->jabelahe) {
                    for (auto& elem : instance->elements) {
                        if (elem->id == this->context_id && !elem->hidden) {
                            if (viewpoint_includes_dp(UI::Element::get_viewpoint(*elem), instance->mmpos)) {
                                elem->click(instance, LEFT);
                            };
                            break;
                        };
                    };
                };
                if (this->disallow_spin_refresh) return;
                if (this->do_spin && !disallow_spin_refresh) {
                    disallow_spin_refresh = true;
                    std::thread([&]() {
                        Renderer::drawpoint mpos;
                        while(this->do_spin) {
                            for (int i = 0; i < this->image.height*this->image.width;i++) {
                                int j = i / this->image.width;
                                int k = i % this->image.width;
                                std::string obj = "";
                                obj.push_back( v_str(g_mkid())[0] );
                                int rand_pos = std::stoi(obj);
                                uicolor_t color = stoui_color[rand_pos];
                                attron(COLOR_PAIR(color));
                                mvaddnwstr(this->position.y+(this->with_frame ? 1 : 0)+j,this->position.x+(this->with_frame ? 1 : 0)+k,wc_unit,1);
                                attroff(COLOR_PAIR(color));
                            };
                            sleep(ms(150));
                        };
                    disallow_spin_refresh = false;
                    }).detach();

                    return;
                };
                ButtonInteraction _find_state = this->clicked || type == CONTEXT_MENU ? CLICKED : NONE;
                ButtonInteraction state = (type == AUTO ? _find_state : type == CONTEXT_MENU ? CLICKED : type);
                std::string thetext = "follow_cursor: " + v_str((int)this->follow_cursor) + " | allow_zoom: " + v_str((int)this->allow_zoom) + " | do_spin: " + v_str((int)this->do_spin) + " | camera: " + v_str(camera.x) + "," + v_str(camera.y)  + " | viewspace: " + v_str(this->viewspace.start.x) + "," + v_str(this->viewspace.start.y) + " : " + v_str(this->viewspace.end.x) + "," + v_str(this->viewspace.end.y);
                if (this->clicked && this->show_info) instance->modify_scr(thetext, UI_BLACK, UI_WHITE, false, {((COLS-((int)thetext.length()+2))/2)+3,0});
                std::vector<Unit> cunits = this->image.contents.units;
                if (follow_cursor) (&cunits[((instance->mmpos.y-this->position.y-2))*this->image.width+(instance->mmpos.x-this->position.x-1)])->color = CYAN;
                std::vector<Unit> raw_units = harvest_units(cunits, this->image.width, this->viewspace);
                std::vector<Unit> units = scale_image(raw_units, _vcreate_vp2d_deltax(this->viewspace)+1, this->image.width, this->image.height);
                int marginx, marginy = 0;
                if (with_frame) {
                    short bcolor = state == CLICKED ? this->background_color_click : this->background_color;
                    instance->modify_scr(repeat_wstring(wc_outer_unit,       this->image.width+2), UI_BLACK, bcolor, false, this->position);
                    instance->modify_scr(repeat_wstring(wc_outer_unit_floor, this->image.width+2), UI_BLACK, bcolor, false, {this->position.x,this->position.y+1+(int)this->image.height});
                    attron(COLOR_PAIR(bcolor));
                    for (int i = 0; i < this->image.height; i++) {
                        mvaddnwstr(i+this->position.y+1,this->position.x,wc_unit, 1);
                        mvaddnwstr(i+this->position.y+1,this->position.x+1+this->image.width,wc_unit, 1);
                    };
                    attroff(COLOR_PAIR(bcolor));
                    marginx = marginy = 1;
                };
                for (int y = 0; y < this->image.height; y++) {
                    for (int x = 0; x < this->image.width; x++) {
                        auto unit = units[y*this->image.width+x];
                        short bcolor = (short)stoui_color[unit.transparent ? BLACK : unit.color];
                        attron(COLOR_PAIR(bcolor));
                        mvaddnwstr(y+this->position.y+marginy,x+this->position.x+marginx,wc_unit,1);
                        attroff(COLOR_PAIR(bcolor));
                    };
                };
            };
            inline void mouse_scroll(UI::Screen* instance, MouseScroll type) override {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::Image::mouse_scroll);
                if (!this->allow_zoom) return;
                this->clicked = true;
                /// for increasing and decreasing viewpoint
                switch(type) {
                    case DOWN:
                        if ( this->camera.x < (this->image.width-1)  ) this->camera.x++;
                        if ( this->camera.y < (this->image.height-2) ) this->camera.y++;
                        break;
                    case UP:
                        if ( this->camera.x > 0 ) this->camera.x--;
                        if ( this->camera.y > 0 ) this->camera.y--;
                        break;
                };
                this->drag(instance);
                this->refresh(instance, AUTO);
            };
            inline void text_recv(UI::Screen* instance, char ch) override {};
            inline void context_menu(UI::Screen* instance) override {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::Image::context_menu);
                this->clicked = true;
                if (this->active_context) {
                    this->jabelahe = true;
                    for (auto &elem : instance->elements) {
                        if (elem->id == this->context_id) {
                            elem->hidden = false;
                            elem->position = instance->mmpos;
                            elem->refresh(instance, HOVERED);
                            this->context_id = elem->id;
                        };
                    };
                    return;
                };
                this->jabelahe = true;
                this->active_context = true;
                auto menu = SettingsMenu(instance->mmpos, this->image.contents.name,28,13);
                this->context_id = menu.id;
                auto do_spinning_switch = UI::Switch({ 5,2 }, "colortest64    ", UI_WHITE, UI_CYAN, this->do_spin);
                do_spinning_switch.change_func = [&](Switch* data) {
                    this->do_spin = (bool)*data;
                    this->jabelahe = false;
                };
                auto show_info_switch = UI::Switch({ 5,4 }, "debug_info     ", UI_WHITE, UI_CYAN, this->show_info);
                show_info_switch.change_func = [&](Switch* data) {
                    this->show_info = (bool)*data;
                };
                auto zoom_switch = UI::Switch({ 5,6 }, "allow_zoom     ", UI_WHITE, UI_CYAN, this->allow_zoom);
                zoom_switch.change_func = [&](Switch* data) {
                    this->allow_zoom = (bool)*data;
                };
                auto follow_switch = UI::Switch({ 5,8 }, "follow_cursor  ", UI_WHITE, UI_CYAN, this->follow_cursor);
                follow_switch.change_func = [&](Switch* data) {
                    this->follow_cursor = (bool)*data;
                };
                menu.add_element(0, std::make_unique<Switch>(std::move(do_spinning_switch)));
                menu.add_element(0, std::make_unique<Switch>(std::move(show_info_switch)));
                menu.add_element(1, std::make_unique<Switch>(std::move(zoom_switch)));
                menu.add_element(1, std::make_unique<Switch>(std::move(follow_switch)));
                instance->add_element<SettingsMenu>(std::make_unique<SettingsMenu>(std::move(menu)));
                instance->_refresh();
            };
        };

        struct LineTextBox : public UI::Element {
          using UI::Element::Element;
          using dp = Renderer::drawpoint;
          using tfunc = std::function<void(std::string, UI::LineTextBox*)>;
          protected:
            std::string _text = ""; tfunc enter_func;
          public:
            const std::string* input = &_text; bool compact = false;
            LineTextBox(dp pos, std::size_t max_size, uicolor_t color, uicolor_t highlighted_color, bool compact, tfunc enter_func) : Element() {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::LineTextBox);
                this->position = pos;
                this->hidden = false;
                this->width = max_size;
                this->compact = compact;
                this->height = compact ? 1 : 2; // cannot be taller than 2c
                this->enter_func = enter_func;
                this->foreground_color = UI_WHITE;
                this->background_color = color;
                this->background_color_highlight = color;
                this->background_color_click = highlighted_color;
            };
            inline void hover(UI::Screen* instance, bool state) override {
                /// NO FUNCTIONALITY;
            };
            inline void click(UI::Screen* instance, Interactions::MouseButton button) override {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::LineTextBox::click);
                if (this->clicked) return;
                if (instance->input_handler != NULL) {
                    instance->input_handler->clicked = false;
                    instance->input_handler = NULL;
                };
                this->clicked = true;
                instance->handles_input = false;
                instance->input_handler = std::shared_ptr<UI::Element>(this, [](UI::Element*) {});
                this->refresh(instance, CLICKED);
            };
            inline void text_recv(UI::Screen* instance, char ch) override {
                if (ch == 27) { /// ESC key
                    this->clicked = false;
                    this->refresh(instance, NONE);
                    return;
                } else {
                    if (this->_text.length() == this->width && ch != 7 && ch != 10) return;
                    switch (ch) {
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                            break;
                        case 9: /// TAB
                            this->_text.push_back(' ');
                            this->refresh(instance, CLICKED);
                            break;
                        case 10: /// Enter
                            this->enter_func(this->_text, this);
                            this->_text = "";
                            this->refresh(instance, CLICKED);
                            break;
                        case 82:
                        case 83:
                            break;
                        case 74: /// DEL key
                        case 7: /// backspace
                            if (this->_text.length() == 0) break;
                            this->_text.pop_back();
                            this->refresh(instance, CLICKED);
                            break;
                        default:
                            this->_text.push_back(ch);
                            this->refresh(instance, CLICKED);
                            break;
                    };
                };
            };
            inline void refresh(UI::Screen* instance, Interactions::ButtonInteraction type) override {
                wchar_t* w_lnU = repeat_wstring(wc_outer_unit, this->width);
                if (!compact) instance->modify_scr(w_lnU, UI_WHITE, conv_bgcolor(clicked ? (uicolor_t)this->background_color_click : (uicolor_t)this->background_color), false, this->position);
                if (type != CLICKED) {
                    std::string w_ln = this->_text + repeatString(" ", (int)this->width-this->_text.length());
                    instance->modify_scr(w_ln, UI_WHITE, this->background_color, false, {this->position.x,this->position.y+(int)!compact});
                } else {
                    wchar_t* w_ln = concat_wstr(concat_wstr(concat_wstr(L"", this->_text), this->_text.length() == this->width ? L"" : wc_unit), repeatString(" ", this->width-this->_text.length()-1));
                    instance->modify_scr(w_ln, UI_WHITE, this->background_color_click, false, {this->position.x,this->position.y+(int)!compact});
                };
            };
        };

        struct ActionMenu : public UI::Element {
          private:
            std::vector<std::unique_ptr<UI::Element>> members;
          public:
            using UI::Element::Element;
            bool extended = false; // show contents, or title of the menu only.
            ActionMenu(Renderer::drawpoint pos, std::string title, std::size_t width, std::size_t height, uicolor_t color, uicolor_t highlight_color) : Element() {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::ActionMenu);
                this->position = pos;
                this->hidden = false;
                this->title = title;
                this->width = width;
                this->height = height;
                this->foreground_color = color;
                this->foreground_color_click = color;
                this->foreground_color_highlight = highlight_color;
                this->background_color = this->background_color_click = this->background_color_highlight = UI_WHITE;
            };
            inline void text_recv(UI::Screen* instance, char ch) override {};
            inline void click(UI::Screen* instance, UI::Interactions::MouseButton button) override {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::ActionMenu::click);
                this->clicked = button == LEFT ? true : false;
                this->refresh(instance, CLICKED);
                instance->__refresh();
                auto future_thing = std::async(std::launch::async, [&]() -> void {
                    this->clicked = false;
                });
            };

            inline void hover(UI::Screen* instance, bool state) override {
                if (state && !this->hovered) __functree_call__(__FILE__, __LINE__, Gmeng::UI::ActionMenu::hover);
                this->hovered = state;
                //if (this->clicked) return; // click does not affect ActionMenu elements.
                if (state) this->refresh(instance, HOVERED);
                else this->refresh(instance, NONE);
            };

            template<typename T>
            inline void add_member(std::unique_ptr<T> elem) {
                __functree_call__(__FILE__, __LINE__, Gmeng::UI::ActionMenu::add_member);
                this->members.push_back(std::move(elem));
            };

            inline void refresh(UI::Screen* instance, UI::Interactions::ButtonInteraction type) override {
                if (this->hidden) return;
                auto win_id = get_window_id();
                bool viable_window = win_id != 0;
                if (!viable_window) return; // not focused
                Renderer::drawpoint _mpos = {-1,-1};
                if (viable_window) _mpos = instance->mmpos;
                if ((_mpos.y == this->position.y) && this->hovered && type == CLICKED) this->extended = !this->extended; // switch extended status.
                wchar_t* wline_str_1 = concat_wstr(L"┌┤▼ ",this->title);
                wchar_t* wline_str_1_e = concat_wstr(L"┌┤▲ ",this->title);
                wchar_t* wline_str_2 = concat_wstr(L"├",repeat_wstring(L"─",(int)this->width-this->title.length()-6));
                wchar_t* wline_str_3 = concat_wstr(wline_str_2,"┐");
                wchar_t* wline_str   = extended ? concat_wstr(wline_str_1,wline_str_3) : concat_wstr(wline_str_1_e,wline_str_3);
                wchar_t* wline_str_down = concat_wstr(concat_wstr(L"└", repeat_wstring(L"─",(int)this->width-2)), L"┘");
                bool hovered = type == HOVERED || type == CLICKED;
                short applied_color = hovered ? this->foreground_color_highlight : this->foreground_color;

                /// extended menu? ( menus can be clicked to hide their contents )
                if (!this->extended) {
                    instance->modify_scr(wline_str, UI_BLACK, applied_color, false, this->position);
                    instance->modify_scr(wline_str_down, UI_BLACK, applied_color, false, {this->position.x,this->position.y+1});
                    return;
                };
                /// REFRESH THE COVER
                instance->modify_scr(wline_str, UI_BLACK, applied_color, false, this->position);
                int c = 1;
                wchar_t* wvln = concat_wstr(concat_wstr(L"│", repeatString(" ",(int)this->width-2)), L"│");
                for ( ;; ) { // once again, dangerous and risky!
                    if (c == (int)this->height) break;
                    instance->modify_scr(wvln, UI_BLACK, applied_color, false, {this->position.x, this->position.y+c});
                    c++;
                }
                instance->modify_scr(wline_str_down, UI_BLACK, applied_color, false, {this->position.x,this->position.y+c});
                /// REFRESH MEMBERS
                for ( auto& elem : this->members ) {
                    elem->hovered = false;
                    bool is_hovered_over_element = false;
                    if (viewpoint_includes_dp(UI::Element::get_viewpoint(*elem), _mpos)) is_hovered_over_element = true;
                    if (_mpos.x != -1 && viable_window && this->clicked && type == CLICKED && is_hovered_over_element) { elem->click(instance, LEFT); continue; };
                    if (!elem->clicked) elem->refresh(instance, is_hovered_over_element ? HOVERED : NONE);
                    else elem->refresh(instance, CLICKED);
                };
            };
        };
    };
};

#define GMENG_CAN_USE_NCURSES true
#define GMENG_UIELEMENTS_INIT true
