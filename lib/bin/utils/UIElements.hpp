#pragma once
#include "../gmeng.hpp"
#include <iostream>
#include <vector>
#include <optional>
#include <ncurses.h>
#include <codecvt>

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
    const uicolor_t stoui_color[] = {UI_WHITE,UI_BLUE,UI_GREEN,UI_CYAN,UI_RED,UI_PINK,UI_YELLOW,UI_BLACK,UI_BGWHITE};
    namespace UI { class Screen; struct Element; };
};

static wchar_t* concat_wstr(const wchar_t* str1, const wchar_t* str2) {
    size_t len1 = wcslen(str1); size_t len2 = wcslen(str2);
    wchar_t* concatenated = new wchar_t[len1 + len2 + 1];
    wcscpy(concatenated, str1); wcscat(concatenated, str2);
    return concatenated;
};

static wchar_t* concat_wstr(const wchar_t* str1, const std::string& str2) {
    // Convert std::string to wide character string
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wstr2 = converter.from_bytes(str2);

    // Allocate memory for the concatenated string
    size_t len1 = wcslen(str1);
    size_t len2 = wstr2.length();
    wchar_t* concatenated = new wchar_t[len1 + len2 + 1];

    // Copy str1 into concatenated
    wcscpy(concatenated, str1);

    // Concatenate wstr2 onto concatenated
    wcscat(concatenated, wstr2.c_str());

    return concatenated;
}

static void repeat(int count, std::function<void()> func) {
    for (int i = 0; i < count; i++) func();
};

static void repeat(int count, std::function<void(int count)> func) {
    for (int i = 0; i < count; i++) func(i);
};

static wchar_t* repeat_wstring(wchar_t* wc, int times) {
    if (times <= 0) {
        return L"";
    };
    wchar_t* str = new wchar_t[times + 1];
    for (int i = 0; i < times; ++i) {
        str[i] = *wc;
    }
    str[times] = L'\0'; // Null-terminate the string

    return str;
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
    Renderer::drawpoint mmpos;
    bool should_report_mouse;
    bool report_status;
    std::function<void(Renderer::drawpoint mouse_pos)> loopfunction = [&](Renderer::drawpoint) -> void {};
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
    inline void modify_scr(const wchar_t* str, short, short, bool, Renderer::drawpoint);
    inline void text(std::string, short = UI_WHITE, short = UI_BLACK, Renderer::drawpoint = {-1,-1});
    template<typename T>
    inline bool add_element(std::unique_ptr<T>);
    inline void _refresh();
    inline void __refresh();
    inline void ___refresh();
  private:
    void check_hover_states(Renderer::drawpoint);
    void handle_left_click(Renderer::drawpoint);
    void handle_right_click(Renderer::drawpoint);
    void handle_scroll_up(Renderer::drawpoint);
    void handle_scroll_down(Renderer::drawpoint);
};
