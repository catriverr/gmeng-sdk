#pragma once
#include "../gmeng.hpp"
#include "./UIElements.hpp"
#include <clocale>
#include <ctime>
#include <cwchar>
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
            static Renderer::viewpoint get_viewpoint(UI::Element& elem) {
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
        Screen* scr;
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
            UI::scr = this;
            if (Gmeng::UI::Screen::initialized) {
                if (global.debugger) gm_slog(YELLOW, "DEBUGGER", "Screen cannot be initialized, a previous one already exists.");
                return;
            };
            initscr(); cbreak(); noecho(); keypad(stdscr, TRUE);
            mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
            start_color(); curs_set(0);
            setlocale(LC_ALL, "");
            signal(SIGWINCH, Gmeng::UI::Screen::handle_resize);
            Gmeng::UI::Screen::refresh_width_height();
            mouseinterval(0);
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
            endwin();
            Gmeng::UI::Screen::initialized = false;
        }
        void Screen::recv_mouse() {
            MEVENT event;
            bool mpos_show = false;
            while (true) {
                int ch = getch();
                if (ch == 'q') exit(0);
                if (ch == 'M') mpos_show = !mpos_show, this->modify_scr("           ", UI_BLACK, UI_BGBLACK, false, {0,0});
                if (ch == KEY_MOUSE) {
                    if (getmouse(&event) == OK) {
                        Renderer::drawpoint pos{ event.x, event.y };
                        if (event.bstate & BUTTON1_PRESSED) {
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
                if (viable_window) this->mmpos = get_pointed_pos(get_mouse_pos());
                if (viable_window) this->check_hover_states(mmpos); // checks the hover states of all elements within the screen.
                if (viable_window && report_status) this->loopfunction(mmpos);
                if (viable_window && mpos_show) this->modify_scr("pos: " + v_str(mmpos.x) + "," + v_str(mmpos.y) + "    ", UI_WHITE, UI_BGBLACK, false, {0,0});
                sleep(ms(7)); // no more than 120 refreshes needed per second.
            };
        };

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

        void Screen::__refresh() { refresh(); };

        void Screen::check_hover_states(Renderer::drawpoint mpos) {
            for (auto& elem : this->elements) {
                if ( viewpoint_includes_dp(UI::Element::get_viewpoint(*elem), mpos) ) {
                    if (elem->hidden) continue;
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
                    if (elem->hidden) continue;
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
        };

        struct Button : public UI::Element {
          protected:
              const UI::Element::Types type = UI::Element::BUTTON;
          private:
            std::function<void(UI::Button* button, UI::Interactions::ButtonInteraction cb)> click_method;
          public:
            using UI::Element::Element; bool compact = false;
            Button(Renderer::drawpoint pos, std::string title, bool compact, short fg_color, short bg_color, std::function<void(UI::Button* button, UI::Interactions::ButtonInteraction cb)> fn) : Element() {
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
            inline void click(UI::Screen* instance, UI::Interactions::MouseButton button) override {
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
            small_render_t(Gmeng::texture txtr) : width(txtr.width), height(txtr.height), contents(txtr) {};
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
                            p.push_back({wc_unit,stoui_color[partial.unit.transparent ? UI_BLACK : partial.unit.color]});
                            break;
                    };
                    c++;
                };
                return render;
            };
        }; // should move to renderer.cpp

        struct Hoverable : public UI::Element {
          private:
            bool HAS_ENOUGH_SPACE = true;
            std::size_t MAX_SPACE_X = -1;
            std::size_t MAX_SPACE_Y = -1;
            bool oo_refresh_after_stopped_hovering = false;
          public:
            using UI::Element::Element;
            using dp = Renderer::drawpoint;
            small_render_t render;
            Hoverable(dp pos,std::string title, small_render_t render, uicolor_t color_base, uicolor_t color_hovered) : Element() {
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
                /// NO FUNCTIONALITY;
            };
            inline void hover(UI::Screen* instance, bool state) override {
                if (state == false) {
                    this->hovered = state;
                    this->refresh(instance, NONE);
                } else {
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

        struct ActionMenu : public UI::Element {
          private:
            std::vector<std::unique_ptr<UI::Element>> members;
          public:
            using UI::Element::Element;
            bool extended = false; // show contents, or title of the menu only.
            ActionMenu(Renderer::drawpoint pos, std::string title, std::size_t width, std::size_t height, uicolor_t color, uicolor_t highlight_color) : Element() {
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
            inline void click(UI::Screen* instance, UI::Interactions::MouseButton button) override {
                this->clicked = button == LEFT ? true : false;
                this->refresh(instance, CLICKED);
                instance->__refresh();
                auto future_thing = std::async(std::launch::async, [&]() -> void {
                    this->clicked = false;
                });
            };

            inline void hover(UI::Screen* instance, bool state) override {
                this->hovered = state;
                //if (this->clicked) return; // click does not affect ActionMenu elements.
                if (state) this->refresh(instance, HOVERED);
                else this->refresh(instance, NONE);
            };

            template<typename T>
            inline void add_member(std::unique_ptr<T> elem) {
                this->members.push_back(std::move(elem));
            };

            inline void refresh(UI::Screen* instance, UI::Interactions::ButtonInteraction type) override {
                if (this->hidden) return;
                auto win_id = get_window_id();
                bool viable_window = win_id != 0;
                if (!viable_window) return; // not focused
                Renderer::drawpoint _mpos = {-1,-1};
                if (viable_window) _mpos = get_pointed_pos(get_mouse_pos());
                if ((_mpos.y-1 == this->position.y-1) && this->hovered && type == CLICKED) this->extended = !this->extended; // switch extended status.
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
                    elem->refresh(instance, is_hovered_over_element ? HOVERED : NONE);
                };
            };
        };
    };
};

#define GMENG_EDITOR_INIT true
