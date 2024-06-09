#pragma once
#include "../../gmeng.hpp"
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
}

namespace Gmeng::UI {
    class Screen;
    struct Element;
};

namespace Gmeng {
    enum uicolor_t {
        UI_WHITE  = 1, UI_BGWHITE  = 9,
        UI_BLUE   = 2, UI_BGBLUE   = 10,
        UI_GREEN  = 3, UI_BGGREEN  = 11,
        UI_CYAN   = 4, UI_BGCYAN   = 12,
        UI_RED    = 5, UI_BGRED    = 13,
        UI_PINK   = 6, UI_BGPINK   = 14,
        UI_YELLOW = 7, UI_BGYELLOW = 15,
        UI_BLACK  = 8, UI_BGBLACK  = 16
    };
    namespace UI {
        namespace Interactions {
            enum MouseButton {
                LEFT = 1, RIGHT = 2
            };
            enum MouseScroll {
                UP = 1, DOWN = 2
            };
            enum ButtonInteraction {
                NONE = 0, CLICKED = 1,
                CONTEXT_MENU = 2, HOVERED = 3
            };
        };
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
            bool hovered;
            std::size_t height; std::size_t width;
            std::string title; Renderer::drawpoint position;
            short foreground_color; short background_color;
            short background_color_highlight = *(&this->background_color);
            short foreground_color_highlight = *(&this->foreground_color);

            Element(bool hovered, std::size_t height, std::size_t width, std::string title, Renderer::drawpoint position,
                    Gmeng::uicolor_t foreground_color, Gmeng::uicolor_t background_color,
                    Gmeng::uicolor_t background_color_highlight, Gmeng::uicolor_t foreground_color_highlight)
                : hovered(hovered), height(height), width(width), title(std::move(title)), position(position),
                  foreground_color(foreground_color), background_color(background_color),
                  background_color_highlight(background_color_highlight),
                  foreground_color_highlight(foreground_color_highlight) {}
            Element() = default;
            /// Hover function. Runs when the element is hovered.
            inline void hover(UI::Screen& instance, bool state) {};

            /// Click function. Runs when the element is clicked.
            inline void click(UI::Screen& instance, UI::Interactions::MouseButton button);

            /// Refresh function. Draws the Element, controlled by main class.
            inline void refresh(UI::Screen& instance, UI::Interactions::ButtonInteraction button) {
                gm_slog(YELLOW, "GM_UI_UX", "refresh_instance: (UI::Element) " + this->title);
            };
        };

        class Screen {
          private:
            Renderer::drawpoint current_cursor_pos;
            static bool initialized;
            static void handle_resize(int sig) { Screen::refresh_width_height(); };
            static void refresh_width_height() {
                getmaxyx(stdscr, UI::Screen::height, UI::Screen::width);
                clear();
                refresh();
            };
            inline bool _uhas_space(Renderer::drawpoint position, Renderer::drawpoint size) {
                return position.y + size.y < LINES && position.x + size.x < COLS;
            };
          public:
            static std::size_t width; static std::size_t height;
            Renderer::drawpoint mouse_pos = {-1,-1};
            std::vector<UI::Element> elements;
            bool should_report_mouse;
            inline void initialize() {
                if (UI::Screen::initialized) {
                    if (global.debugger) gm_slog(YELLOW, "DEBUGGER", "Screen cannot be initialized, a previous one already exists.");
                    return;
                };
                initscr(); cbreak(); noecho(); keypad(stdscr, TRUE);
                mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
                start_color(); curs_set(0); setlocale(LC_CTYPE, "");
                nodelay(stdscr, false);
                signal(SIGWINCH, UI::Screen::handle_resize);
                refresh_width_height();

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
                init_pair(UI_BGGREEN, COLOR_WHITE, COLOR_GREEN);
                init_pair(UI_BGRED, COLOR_WHITE, COLOR_RED);
                init_pair(UI_BGYELLOW, COLOR_BLACK, COLOR_YELLOW);
                if (_gutil_get_terminal() == "iTerm.app")
                    if (global.debugger) this->text("right-click functionality will not work on your current Terminal! (iTerm / iTerm2). Please switch to your default, OS-Provided Terminal instead.", UI_RED, UI_BLACK, {0,0});                
                UI::Screen::initialized = true;
            };

            /// destroys screen.
            inline void destroy() {
                endwin();
                UI::Screen::initialized = false;
            };

            inline void recv_mouse() {
                MEVENT event;
                while(true) {
                    this->_refresh();
                    int ch = getch();
                    if (ch == 'q') exit(0);

                    if (ch == KEY_MOUSE) {
                        if (getmouse(&event) == OK) {
                            Gmeng::Renderer::drawpoint pos {event.x, event.y};
                            if (event.bstate & BUTTON1_CLICKED) {
                                handle_left_click(pos);
                            } else if (event.bstate & BUTTON3_CLICKED) {
                                handle_right_click(pos);
                            } else if (event.bstate & BUTTON4_PRESSED) {
                                handle_scroll_up(pos);
                            } else if (event.bstate & BUTTON5_PRESSED) { //ignore error here, ncurses thing. will compile.
                                handle_scroll_down(pos);
                            }
                        }
                    };
                };
            };

            inline Renderer::drawpoint get_current_pos() {
                int x, y;
                getyx(stdscr, y, x);
                std::cout << x << y;
                this->current_cursor_pos = {x,y};
                return {x, y};
            }

            inline void report_mouse_pos(bool state) {
              this->should_report_mouse = state;
              std::thread t_mousepos = Gmeng::_ucreate_thread([&]() {
                while (this->should_report_mouse) {
                    MEVENT event;
                    int ch = getch();
                    if (ch == KEY_MOUSE) {
                        if (getmouse(&event) == OK) this->mouse_pos = {event.x, event.y};
                    };
                };
              });
            };
            inline Renderer::drawpoint get_mouse_pos() {
                return this->mouse_pos;
            };

            inline void set_cursor(Renderer::drawpoint dp) {
                move(dp.y, dp.x);
                this->current_cursor_pos = dp;
            };

            inline void modify_scr(std::string text, short fgcolor_pair, short bgcolor_pair, bool reverse_bg, Renderer::drawpoint pos) {
                auto lines = g_splitStr(text, "\n");
                auto cpos = this->current_cursor_pos;
                this->set_cursor(pos);
                if (reverse_bg) attron(COLOR_PAIR(bgcolor_pair | A_REVERSE));
                else attron(COLOR_PAIR(bgcolor_pair));
                for (std::size_t i = 0; i < lines.size(); i++) {
                    this->set_cursor({pos.x, v_static_cast<int>(pos.y+i)});
                    wprintw(stdscr, lines[i].c_str());
                };
                this->set_cursor(cpos);
            };
            inline void modify_scr(chtype ch, short fgcolor_pair, short bgcolor_pair, bool reverse_bg, Renderer::drawpoint pos) {
                auto cpos = this->current_cursor_pos;
                this->set_cursor(pos);
                if (reverse_bg) attron(COLOR_PAIR(bgcolor_pair | A_REVERSE));
                else attron(COLOR_PAIR(bgcolor_pair));
                waddch(stdscr, ch);
                if (reverse_bg) attroff(COLOR_PAIR(bgcolor_pair | A_REVERSE));
                else attroff(COLOR_PAIR(bgcolor_pair));
                this->set_cursor(cpos);
            };

            inline void text(std::string text, short fgcolor = UI_WHITE, short bgcolor = UI_BLACK, Renderer::drawpoint pos = {-1, -1}) {
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
                    set_cursor(current_cursor_pos); // restore old cursor position
                };
            };

            /// Adds an element to the screen instance.
            inline bool add_element(UI::Element __a) {
                // Implementation for creating a menu (subclass)
                // Background color, position, and buttons will be used here
                if (!this->_uhas_space(__a.position, {(int)__a.width, (int)__a.height})) {
                    if (global.debugger) gm_slog(YELLOW, "DEBUGGER", "STDSCR does not have enough space to create subclass.");
                    return false;
                } else {
                    this->elements.push_back(__a);
                    this->_refresh();
                    __a.refresh(*this, UI::HOVERED);
                    return true;
                };
            };

            /// Refreshes the elements included in the instance.
            inline void _refresh() {
                refresh();
                auto v = get_current_pos();
                for (auto& elem : this->elements) {
                    this->modify_scr(v_str(g_mkid()) + " refreshing " + elem.title, UI_WHITE, UI_BGRED, false, {0,LINES-1});
                    elem.refresh(*this, elem.hovered ? HOVERED : NONE);
                };
            };

          private:
            void handle_left_click(Gmeng::Renderer::drawpoint pos) {
                for (auto& elem : this->elements) {
                    bool v = viewpoint_includes_dp( {
                            .start = elem.position,
                            .end = { static_cast<int>(elem.position.x+elem.width), static_cast<int>(elem.position.y+elem.height-1) }
                       }, {pos.x,pos.y});
                    this->modify_scr((std::string)(v ? "true" : "false") + " for " + elem.title, UI_WHITE, UI_BGBLUE, false, {0,0});
                    if ( (std::string)(v ? "true" : "false") == "true" ) {
                        elem.hover(*this, true);
                    } else elem.hover(*this, false);
                };
            };

            void handle_right_click(Gmeng::Renderer::drawpoint pos) {
                this->text("Right Click from " + v_str(pos.x) + "," + v_str(pos.y) +"!", UI_CYAN, UI_WHITE);
            };

            void handle_scroll_up(Gmeng::Renderer::drawpoint pos) {
                this->text("Scroll Up from " + v_str(pos.x) + "," + v_str(pos.y) +"!", UI_CYAN, UI_WHITE);
            };

            void handle_scroll_down(Gmeng::Renderer::drawpoint pos) {
                this->text("Scroll Down from " + v_str(pos.x) + "," + v_str(pos.y) +"!", UI_CYAN, UI_WHITE);
            };
        };

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
            inline void click(UI::Screen& instance, UI::Interactions::MouseButton button) {
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

            inline void hover(UI::Screen& instance, bool state) {
                instance.modify_scr(v_str(g_mkid()) + " hover for " + this->title + ": " + (state ? "1" : "0"), UI_WHITE, UI_BGGREEN, false, {40, LINES-1});
                if (state) this->refresh(instance, HOVERED);
                else this->refresh(instance, NONE);
            };
            inline void refresh(UI::Screen& instance, UI::Interactions::ButtonInteraction type) {
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
                instance.modify_scr(v_str(g_mkid()) + ", DATA for " + this->title + ": HOVERED: " + (modifiers.highlight ? "1" : "0") +", CLICKED: " + (modifiers.reverse_bg ? "1" : "0"), UI_WHITE, UI_BGBLUE, false, {0,5});
                if (this->compact) {
                    short applied_fg = modifiers.highlight  ? this->foreground_color_highlight : this->foreground_color;
                    short highlighted_bg = modifiers.highlight ? this->background_color_highlight : this->background_color;
                    short applied_bg = highlighted_bg;
                    instance.modify_scr(ACS_RTEE, applied_fg, applied_bg, modifiers.reverse_bg, this->position);
                    instance.modify_scr(ACS_LTEE, applied_fg, applied_bg, modifiers.reverse_bg, { .x=v_static_cast<int>(position.x+title.length()+1),.y=position.y });
                    instance.modify_scr(title, applied_fg, applied_bg, modifiers.reverse_bg, { .x=v_static_cast<int>(position.x+1),.y=position.y });
                } else {
                    short applied_fg = modifiers.highlight  ? this->foreground_color_highlight : this->foreground_color;
                    short highlighted_bg = modifiers.highlight ? this->background_color_highlight : this->background_color;
                    short applied_bg = highlighted_bg;
                    instance.modify_scr(ACS_ULCORNER, applied_fg, applied_bg, modifiers.reverse_bg, this->position);
                    __oloop__: for (int j = 0; j < 2; j++) {
                      __iloop__: for (int i = 0; i < (title.length()+2); i++) {
                          instance.modify_scr(ACS_HLINE, applied_fg, applied_bg, modifiers.reverse_bg, {
                              .x=v_static_cast<int>(this->position.x+1 + i),
                              .y=this->position.y+(j==1 ? j+1 : 0)
                          });
                      };
                    };
                    instance.modify_scr(ACS_URCORNER, applied_fg, applied_bg, modifiers.reverse_bg, { .x= v_static_cast<int>(this->position.x+this->title.length()+3), .y=this->position.y });
                    instance.modify_scr(ACS_VLINE, applied_fg, applied_bg, modifiers.reverse_bg, { .x = this->position.x, .y = v_static_cast<int>(this->position.y+1) });
                    instance.modify_scr(ACS_VLINE, applied_fg, applied_bg, modifiers.reverse_bg, { .x = v_static_cast<int>(this->position.x+3+title.length()), .y = v_static_cast<int>(this->position.y+1) });
                    instance.modify_scr(" " + title + " ", applied_fg, applied_bg, modifiers.reverse_bg, { .x = v_static_cast<int>(this->position.x+1), .y = v_static_cast<int>(this->position.y+1) });
                    instance.modify_scr(ACS_LLCORNER, applied_fg, applied_bg, modifiers.reverse_bg, { .x = this->position.x, .y = v_static_cast<int>(this->position.y+2) });
                    instance.modify_scr(ACS_LRCORNER, applied_fg, applied_bg, modifiers.reverse_bg, { .x = v_static_cast<int>(this->position.x+3+title.length()), .y = v_static_cast<int>(this->position.y+2) });
                };
            };
        };

        template<std::size_t height = 0, std::size_t width = 0>
        struct ActionMenu : public UI::Element {

        };

        std::size_t UI::Screen::width = 0; std::size_t UI::Screen::height = 0;
        bool UI::Screen::initialized = false;
    };
};

#define GMENG_EDITOR_INIT true
