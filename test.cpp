#include <chrono>
#include <iostream>
#include "./lib/bin/gmeng.hpp"
#include "./lib/bin/utils/UIElements.cpp"
#include "lib/bin/def/renderer.cpp"
#include <memory>
#include <ncurses.h>
#include <thread>

using namespace Gmeng;

int main2() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, true);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);

    MEVENT event;
    int ch;
    while ((ch = getch()) != 'q') {
        Renderer::drawpoint window_size = get_window_size(get_window_id());
        mvprintw(3, 0, ("window_size: " + v_str(window_size.x) + "," + v_str(window_size.y) + " | window_id:" + v_str(get_window_id()) + " | mpos: " + v_str(get_mouse_pos().x) + "," + v_str(get_mouse_pos().y) + " | pchar: " + v_str(get_pointed_pos(get_mouse_pos()).x) + "," + v_str(get_pointed_pos(get_mouse_pos()).y) + "     ").c_str());
        if (ch == KEY_MOUSE && getmouse(&event) == OK) {
            // Use event.x and event.y as the mouse coordinates
            mvprintw(0, 0, "Mouse position: %d, %d   ", event.x, event.y);
            refresh();
        }
    }

    // Clean up ncurses
    endwin();

    return 0;
}

int main(int argc, char **argv) {
    bool do_main2 = false;
    for (int i = 0; i < argc; i++)
        if (std::string(argv[i]) == "-main2") do_main2 = true;
    if (do_main2) { main2(); return 0; };
    Gmeng::UI::Screen test;
    test.initialize();
    auto func = [&](UI::ButtonInteraction mb) {
        test.text("hi there!", UI_WHITE, UI_BLACK, { 50, 3 });
    };
    auto button1 = UI::Button({25, 10}, "button1", false, UI_WHITE, UI_BGRED, func);
    button1.background_color_highlight = UI_BGGREEN;
    button1.hovered = false;
    auto button2 = UI::Button({37, 10}, "button2", false, UI_WHITE, UI_BGBLUE, func);
    button2.background_color_highlight = UI_BGCYAN;
    button2.hovered = false;
    bool add1 = test.add_element<UI::Button>(std::make_unique<UI::Button>(button1));
    bool add2 = test.add_element<UI::Button>(std::make_unique<UI::Button>(button2));
    test.recv_mouse();
    return 0;
};
