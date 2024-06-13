#pragma once
#include "../gmeng.hpp"
#include <iostream>
#include <vector>
#include <optional>
#include <ncurses.h>

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
    namespace UI { class Screen; struct Element; };
};

namespace Gmeng::UI::Interactions {
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

class Gmeng::UI::Screen {
  private:
    static bool initialized;
    static void handle_resize(int sig);
    static void refresh_width_height();
    inline bool _uhas_space(Renderer::drawpoint, Renderer::drawpoint);
  public:
    static std::size_t width; static std::size_t height;
    Renderer::drawpoint mouse_pos;
    Renderer::drawpoint current_cursor_pos;
    bool should_report_mouse;
    std::vector<std::unique_ptr<UI::Element>> elements;
    inline void initialize();
    inline void destroy();
    inline void recv_mouse();
    inline Renderer::drawpoint get_current_pos();
    inline void report_mouse_pos(bool);
    inline Renderer::drawpoint get_mouse_pointer_pos();
    inline void set_cursor(Renderer::drawpoint);
    inline void modify_scr(std::string, short, short, bool, Renderer::drawpoint);
    inline void modify_scr(chtype ch, short, short, bool, Renderer::drawpoint);
    inline void text(std::string, short = UI_WHITE, short = UI_BLACK, Renderer::drawpoint = {-1,-1});
    template<typename T>
    inline bool add_element(std::unique_ptr<T>);
    inline void _refresh();
  private:
    void check_hover_states(Renderer::drawpoint);
    void handle_left_click(Renderer::drawpoint);
    void handle_right_click(Renderer::drawpoint);
    void handle_scroll_up(Renderer::drawpoint);
    void handle_scroll_down(Renderer::drawpoint);
};
