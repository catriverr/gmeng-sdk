#include <chrono>
#include <iostream>
#include "./lib/bin/gmeng.hpp"
#include "./lib/bin/utils/UIElements.cpp"
#include "lib/bin/def/renderer.cpp"
#include "lib/bin/utils/UIElements.hpp"
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




namespace instance_container { UI::Screen* instance; };
using namespace instance_container;

void data_recv(Renderer::drawpoint mpos) {
    
};

int main(int argc, char **argv) {
    gm::_uread_into_vgm("./envs/models");
    bool do_main2 = false;
    for (int i = 0; i < argc; i++)
        if (std::string(argv[i]) == "-main2") do_main2 = true;
    if (do_main2) { main2(); return 0; };
    Gmeng::UI::Screen test;
    instance_container::instance = &test;
    test.initialize();
    auto b1func = [&](UI::Button* button, UI::ButtonInteraction mb) {
    };
    auto b2func = [&](UI::Button* button, UI::ButtonInteraction mb) {
    };
    auto b3func = [&](UI::Button* button, UI::ButtonInteraction mb) {
    };
    auto button1 = UI::Button({128,3}, "OPEN MAP", false, UI_WHITE, UI_BGBLUE, b1func);
    button1.background_color_highlight = UI_BGWHITE;
    button1.background_color_click = UI_BGCYAN;
    button1.hovered = false;
    auto button2 = UI::Button({128,7}, "SAVE MAP", false, UI_WHITE, UI_BGBLUE, b2func);
    button2.background_color_highlight = UI_BGWHITE;
    button2.background_color_click = UI_BGCYAN;
    auto button3 = UI::Button({127,11}, "CREATE MAP", false, UI_WHITE, UI_BGBLUE, b3func);
    button3.background_color_highlight = UI_BGWHITE;
    button3.background_color_click = UI_BGCYAN;
    auto menu1 = UI::ActionMenu({124,1}, "editor", 20, 14, UI_WHITE, UI_YELLOW);
    menu1.add_member<UI::Button>(std::make_unique<UI::Button>(std::move(button1)));
    menu1.add_member<UI::Button>(std::make_unique<UI::Button>(std::move(button2)));
    menu1.add_member<UI::Button>(std::make_unique<UI::Button>(std::move(button3)));
    bool add3 = test.add_element<UI::ActionMenu>(std::make_unique<UI::ActionMenu>(std::move(menu1)));
    auto vgm_cake  = gm::vd_find_texture(gm::vgm_defaults::vg_textures, "01_cake_txtr");
    auto vgm_allah = gm::vd_find_texture(gm::vgm_defaults::vg_textures, "allah");
    UI::small_render_t render1(vgm_cake);
    UI::small_render_t render2(vgm_allah);
    auto hover1 = UI::Hoverable({4,3}, vgm_cake.name, render1, UI_CYAN, UI_YELLOW);
    auto hover2 = UI::Hoverable({4,5}, vgm_allah.name, render2, UI_CYAN, UI_YELLOW);
    auto menu2 = UI::ActionMenu({2,1}, "objects", 20, 25, UI_WHITE, UI_BLUE);
    menu2.add_member<UI::Hoverable>(std::make_unique<UI::Hoverable>(std::move(hover1)));
    menu2.add_member<UI::Hoverable>(std::make_unique<UI::Hoverable>(std::move(hover2)));
    bool add4 = test.add_element<UI::ActionMenu>(std::make_unique<UI::ActionMenu>(std::move(menu2)));
    test.report_status = true;
    test.loopfunction = data_recv;
    test.recv_mouse();
    return 0;
};
