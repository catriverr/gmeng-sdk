#pragma once
#include "../gmeng.hpp"
#include "./UIElements.hpp"
#include <clocale>
#include <ctime>
#include <locale.h>
#include <iostream>
#include <optional>
#include <string>
#include <algorithm>
#include <array>
#include <map>
#include <thread>
#include <vector>
#include <ncurses.h>
#include <unordered_map>
#include <regex.h>
#include <wchar.h>
#include <memory>
#include <ApplicationServices/ApplicationServices.h>

#define sleep std::this_thread::sleep_for
#define ms std::chrono::milliseconds

#ifndef BUTTON5_PRESSED
    #define BUTTON5_PRESSED true
#endif /// quick little fix for the 5.7 ncurses version. USE G++ PARAMETER: `pkg-config --libs --cflags ncurses`!!!!!!

std::string _gutil_get_terminal() {
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
    size.x = (bounds.size.width/2-1) * (COLS);
    size.x = size.x > CGDisplayBounds(get_display_for_window(get_window_id())).size.width ? CGDisplayBounds(get_display_for_window(get_window_id())).size.width : size.x;
    size.y = bounds.size.width       * (LINES-3);
    size.y = size.y > CGDisplayBounds(get_display_for_window(get_window_id())).size.height ? CGDisplayBounds(get_display_for_window(get_window_id())).size.height : size.y;
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
    double _x = ( (double)mpos.x / ( (double)(bounds.size.width-2.7)/2 ) ); // magic number 1 (retarded fonts being all wacky)
    double _y = ( (double)mpos.y / ( (double)(bounds.size.width)-3.4 )   ); // perfectionism is an illness
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
    CGDirectDisplayID display_id = get_display_for_window(get_window_id());
    Renderer::drawpoint window_size = get_window_size(get_window_id());
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



namespace Gmeng {
    namespace UI {
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
            static Renderer::viewpoint get_viewpoint(UI::Element& elem) {
                return {
                    elem.position,
                    { elem.position.x + (int)elem.width, elem.position.y + (int)elem.height }
                };
            };

            bool hovered; bool clicked;
            std::size_t height; std::size_t width;
            std::string title; Renderer::drawpoint position;
            short foreground_color; short background_color;
            short background_color_highlight;
            short foreground_color_highlight;

            inline Element(bool hovered, std::size_t height, std::size_t width, std::string title, Renderer::drawpoint position,
                    Gmeng::uicolor_t foreground_color, Gmeng::uicolor_t background_color,
                    Gmeng::uicolor_t background_color_highlight, Gmeng::uicolor_t foreground_color_highlight)
                : hovered(hovered), height(height), width(width), title(std::move(title)), position(position),
                  foreground_color(foreground_color), background_color(background_color),
                  background_color_highlight(background_color_highlight),
                  foreground_color_highlight(foreground_color_highlight) {};
            inline Element() = default;
            virtual ~Element() = default;
            /// Hover function. Runs when the element is hovered.
            virtual void hover(UI::Screen* instance, bool state) = 0;

            /// Click function. Runs when the element is clicked.
            virtual void click(UI::Screen* instance, UI::Interactions::MouseButton button) = 0;
            /// Refresh function. Draws the Element, controlled by main class.
            virtual void refresh(UI::Screen* instance, UI::Interactions::ButtonInteraction button) = 0;
        };
        std::size_t Screen::width = 0;
        std::size_t Screen::height = 0;
        bool Screen::initialized = false;

        void Screen::handle_resize(int sig) {
            Gmeng::UI::Screen::refresh_width_height();
        }

        void Screen::refresh_width_height() {
            getmaxyx(stdscr, Gmeng::UI::Screen::height, Gmeng::UI::Screen::width);
            clear();
            refresh();
        }

        bool Screen::_uhas_space(Renderer::drawpoint position, Renderer::drawpoint size) {
            return position.y + size.y < LINES && position.x + size.x < COLS;
        }

        void Screen::initialize() {
            if (Gmeng::UI::Screen::initialized) {
                if (global.debugger) gm_slog(YELLOW, "DEBUGGER", "Screen cannot be initialized, a previous one already exists.");
                return;
            };
            initscr(); cbreak(); noecho(); keypad(stdscr, TRUE);
            mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
            start_color(); curs_set(0); setlocale(LC_CTYPE, "");
            nodelay(stdscr, true);
            signal(SIGWINCH, Gmeng::UI::Screen::handle_resize);
            Gmeng::UI::Screen::refresh_width_height();

            init_pair(UI_WHITE, COLOR_WHITE, COLOR_BLACK);
            init_pair(UI_BLUE, COLOR_BLUE, COLOR_BLACK);
            init_pair(UI_GREEN, COLOR_GREEN, COLOR_BLACK);
            init_pair(UI_CYAN, COLOR_CYAN, COLOR_BLACK);
            init_pair(UI_RED, COLOR_RED, COLOR_BLACK);
            init_pair(UI_PINK, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(UI_YELLOW, COLOR_YELLOW, COLOR_BLACK);
            init_pair(UI_BLACK, COLOR_BLACK, COLOR_BLACK);
            init_pair(UI_BGBLACK, COLOR_WHITE, COLOR_BLACK);
            init_pair(UI_BGWHITE, COLOR_BLACK, COLOR_WHITE);
            init_pair(UI_BGBLUE, COLOR_WHITE, COLOR_BLUE);
            init_pair(UI_BGCYAN, COLOR_BLACK, COLOR_CYAN);
            init_pair(UI_BGGREEN, COLOR_BLACK, COLOR_GREEN);
            init_pair(UI_BGRED, COLOR_WHITE, COLOR_RED);
            init_pair(UI_BGYELLOW, COLOR_BLACK, COLOR_YELLOW);
            if (_gutil_get_terminal() == "iTerm.app")
                if (global.debugger) this->text("right-click functionality will not work on your current Terminal! (iTerm / iTerm2). Please switch to your default, OS-Provided Terminal instead.", UI_RED, UI_BLACK, {0,0});
            Gmeng::UI::Screen::initialized = true;
        }

        void Screen::destroy() {
            endwin();
            Gmeng::UI::Screen::initialized = false;
        }

        void Screen::recv_mouse() {
            MEVENT event;
            while (true) {
                int ch = getch();
                if (ch == 'q') exit(0);
                if (ch == KEY_MOUSE) {
                    if (getmouse(&event) == OK) {
                        Renderer::drawpoint pos{ event.x, event.y };
                        if (event.bstate & BUTTON1_CLICKED) {
                            handle_left_click(pos);
                        }
                        else if (event.bstate & BUTTON3_CLICKED) {
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
                this->check_hover_states(get_pointed_pos(get_mouse_pos())); // checks the hover states of all elements within the screen.
                sleep(ms(6)); // no more than 120 refreshes needed per second.
            };
        }

        Renderer::drawpoint Screen::get_current_pos() {
            int x, y;
            getyx(stdscr, y, x);
            std::cout << x << y;
            this->current_cursor_pos = { x,y };
            return { x, y };
        }

        void Screen::report_mouse_pos(bool state) {
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
            return this->mouse_pos;
        };

        void Screen::set_cursor(Renderer::drawpoint dp) {
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
            auto v = get_current_pos();
            for (auto& elem : this->elements) {
                elem->refresh(this, elem->hovered ? HOVERED : NONE);
            };
            refresh();
        };

        void Screen::check_hover_states(Renderer::drawpoint mpos) {
            for (auto& elem : this->elements) {
                if ( viewpoint_includes_dp(UI::Element::get_viewpoint(*elem), mpos) ) {
                    elem->hover(this, true);
                } else elem->hover(this, false);
            };
        };

        void Screen::handle_left_click(Renderer::drawpoint pos) {
            for (auto& elem : this->elements) {
                bool v = viewpoint_includes_dp({
                    .start = elem->position,
                    .end = { static_cast<int>(elem->position.x + elem->width), static_cast<int>(elem->position.y + elem->height - 1) }
                    }, { pos.x,pos.y });
                if ((std::string)(v ? "true" : "false") == "true") {
                    elem->click(this, LEFT);
                }
                else { elem->hovered = false; };
            };
        }

        void Screen::handle_right_click(Renderer::drawpoint pos) {
        };

        void Screen::handle_scroll_up(Renderer::drawpoint pos) {
        };

        void Screen::handle_scroll_down(Renderer::drawpoint pos) {
        };;

        struct Button : public UI::Element {
          protected:
              const UI::Element::Types type = UI::Element::BUTTON;
          private:
            std::function<void(UI::Interactions::ButtonInteraction cb)> click_method;
          public:
            using UI::Element::Element; bool compact = false;
            Button(Renderer::drawpoint pos, std::string title, bool compact, short fg_color, short bg_color, std::function<void(UI::Interactions::ButtonInteraction cb)> fn) : Element() {
                this->position = pos; this->title = title;
                this->foreground_color = fg_color; this->background_color = bg_color;
                this->foreground_color_highlight = *(&this->foreground_color);
                this->background_color_highlight = *(&this->background_color);
                this->click_method = fn;
                this->width = title.length() + 4;
                this->height = compact ? 1 : 3;
                this->compact = compact;
            };
            inline void click(UI::Screen* instance, UI::Interactions::MouseButton button) override {
                switch (button) {
                    case LEFT:
                        this->refresh(instance, CLICKED);
                        this->click_method(CLICKED);
                        break;
                    case RIGHT:
                        this->refresh(instance, CONTEXT_MENU);
                        this->click_method(CONTEXT_MENU);
                        break;
                };
            };

            inline void hover(UI::Screen* instance, bool state) override {
                this->hovered = state;
                if (state) this->refresh(instance, HOVERED);
                else this->refresh(instance, NONE);
            };
            inline void refresh(UI::Screen* instance, UI::Interactions::ButtonInteraction type) override {
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
                gm_log("Refreshing BUTTON instance, of " + this->title + " at pos " + v_str(this->position.x) + "," + v_str(this->position.y));
                if (this->title.length() > 25 || this->title.find("\n", 0) != std::string::npos) {
                    gm_log("could not refresh BUTTON instance, '" + this->title + "' is invalid. Max 25 characters allowed. Newlines are disallowed.");
                    return;
                };
                if (this->compact) {
                    short applied_fg = modifiers.highlight  ? this->foreground_color_highlight : this->foreground_color;
                    short highlighted_bg = modifiers.highlight ? this->background_color_highlight : this->background_color;
                    short applied_bg = highlighted_bg;
                    instance->modify_scr(ACS_RTEE, applied_fg, applied_bg, modifiers.reverse_bg, this->position);
                    instance->modify_scr(ACS_LTEE, applied_fg, applied_bg, modifiers.reverse_bg, { .x=v_static_cast<int>(position.x+title.length()+1),.y=position.y });
                    instance->modify_scr(title, applied_fg, applied_bg, modifiers.reverse_bg, { .x=v_static_cast<int>(position.x+1),.y=position.y });
                } else {
                    short applied_fg = modifiers.highlight  ? this->foreground_color_highlight : this->foreground_color;
                    short highlighted_bg = modifiers.highlight ? this->background_color_highlight : this->background_color;
                    short applied_bg = highlighted_bg;
                    instance->modify_scr(ACS_ULCORNER, applied_fg, applied_bg, modifiers.reverse_bg, this->position);
                    __oloop__: for (int j = 0; j < 2; j++) {
                      __iloop__: for (int i = 0; i < (title.length()+2); i++) {
                          instance->modify_scr(ACS_HLINE, applied_fg, applied_bg, modifiers.reverse_bg, {
                              .x=v_static_cast<int>(this->position.x+1 + i),
                              .y=this->position.y+(j==1 ? j+1 : 0)
                          });
                      };
                    };
                    instance->modify_scr(ACS_URCORNER, applied_fg, applied_bg, modifiers.reverse_bg, { .x= v_static_cast<int>(this->position.x+this->title.length()+3), .y=this->position.y });
                    instance->modify_scr(ACS_VLINE, applied_fg, applied_bg, modifiers.reverse_bg, { .x = this->position.x, .y = v_static_cast<int>(this->position.y+1) });
                    instance->modify_scr(ACS_VLINE, applied_fg, applied_bg, modifiers.reverse_bg, { .x = v_static_cast<int>(this->position.x+3+title.length()), .y = v_static_cast<int>(this->position.y+1) });
                    instance->modify_scr(" " + title + " ", applied_fg, applied_bg, modifiers.reverse_bg, { .x = v_static_cast<int>(this->position.x+1), .y = v_static_cast<int>(this->position.y+1) });
                    instance->modify_scr(ACS_LLCORNER, applied_fg, applied_bg, modifiers.reverse_bg, { .x = this->position.x, .y = v_static_cast<int>(this->position.y+2) });
                    instance->modify_scr(ACS_LRCORNER, applied_fg, applied_bg, modifiers.reverse_bg, { .x = v_static_cast<int>(this->position.x+3+title.length()), .y = v_static_cast<int>(this->position.y+2) });
                };
            };
        };

        template<std::size_t height = 0, std::size_t width = 0>
        struct ActionMenu : public UI::Element {

        };
    };
};

#define GMENG_EDITOR_INIT true
