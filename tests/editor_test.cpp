#include <atomic>
#include <sstream>
#define __GMENG_ALLOW_LOG__ true

#include <chrono>
#include <iostream>
#include "../lib/bin/gmeng.h"
#include "../lib/bin/utils/interface.cpp"
#include "../lib/bin/utils/network.cpp"
#include "../lib/bin/src/renderer.cpp"
#include "../lib/bin/types/interface.h"
#include <memory>
#include <ncurses.h>
#include <thread>
#include <chrono>

using namespace Gmeng;


int main4() {
    gmserver_t myserver(7388);
    myserver.create_path(path_type_t::GET, "/test", [&](auto& req, auto& res) {
        res.body += "Hello";
    });
    myserver.run();
    return 0;
};















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

void setInterval(std::function<void()> func, unsigned int interval, std::atomic<bool>& stopFlag) {
    while (!stopFlag.load()) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        std::this_thread::sleep_for(std::chrono::milliseconds(interval) - elapsed);
    }
};

int main3() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    int row_num;
    while (true) {
        if (row_num == LINES-1) { clear(); row_num = 0; };
        mvprintw(row_num,0, "enter char>");
        int ch = getch();
        if (ch == 'Q') { endwin(); exit(0); };
        mvprintw(row_num++,0,std::string(std::string(" charcode is: ") + v_str(ch)).c_str());
    };
    return 0;
};

namespace instance_container { UI::Screen* instance; };
using namespace instance_container;

void data_recv(Renderer::drawpoint mpos) {

};

int main(int argc, char **argv) {
    gm::_uread_into_vgm("./envs/models");
    bool do_main2, do_net, do_main3 = false;
    for (int i = 0; i < argc; i++) {
        if (std::string(argv[i]) == "-main2") do_main2 = true;
        else if (std::string(argv[i]) == "-charcode_test") do_main3 = true;
        else if (std::string(argv[i]) == "-network") do_net = true;
    };
    if (do_main2) { main2(); return 0; };
    if (do_main3) { main3(); return 0; };
    if (do_net)   { main4(); return 0; };
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
    auto vgm_cake  = gm::vd_find_texture(gm::vgm_defaults::vg_textures, "01_cake_txtr");
    auto vgm_allah = gm::vd_find_texture(gm::vgm_defaults::vg_textures, "allah");
    UI::small_render_t render1(vgm_cake);
    UI::small_render_t render2(vgm_allah);
    auto hover1 = UI::Hoverable({4,3}, vgm_cake.name, render1, UI_CYAN, UI_YELLOW);
    auto hover2 = UI::Hoverable({4,5}, vgm_allah.name, render2, UI_CYAN, UI_YELLOW);
    auto menu2 = UI::ActionMenu({2,1}, "objects", 20, 25, UI_WHITE, UI_BLUE);
    auto switch1 = UI::Switch({5,14}, "test", UI_WHITE, UI_CYAN, false);
    menu2.add_member<UI::Switch>(std::make_unique<UI::Switch>(std::move(switch1)));
    Gmeng::texture vgm_house = gm::vd_find_texture(gm::vgm_defaults::vg_textures, "ui_map_txtr");
    UI::small_render_t render3(vgm_house);

    auto image1 = UI::Image({(COLS-(int)render3.width+2)/2,6}, render3, true, UI_BLUE, UI_CYAN);
    bool add5 = test.add_element<UI::Image>(std::make_unique<UI::Image>(std::move(image1)));
    menu2.add_member<UI::Hoverable>(std::make_unique<UI::Hoverable>(std::move(hover1)));
    menu2.add_member<UI::Hoverable>(std::make_unique<UI::Hoverable>(std::move(hover2)));
    /// ADD MENUS
    bool add3 = test.add_element<UI::ActionMenu>(std::make_unique<UI::ActionMenu>(std::move(menu1)));
    bool add4 = test.add_element<UI::ActionMenu>(std::make_unique<UI::ActionMenu>(std::move(menu2)));
    int time = GET_TIME();
    UI::InfoBox::func func1 = [&](UI::InfoBox* box) -> std::string {
        if (GET_TIME() < time+1000) return "";
        time = GET_TIME();
        box->data = "";
        for (int i = 0; i < box->height-2; i++) {
            int pos = Gmeng::func_last.size()-box->height-2+i;
            auto str1 = Gmeng::func_last[pos];
            auto str = str_replace(str_replace(str_replace(str1, "Gmeng::", ""), "::", "."), "__private__.", "");
            box->data += v_str(pos) + " " + g_splitStr(str, ">> ")[1] + "\n";
        };
        return "";
    };
    auto info1 = UI::InfoBox({107, 28}, func1, 36, 10);
    //bool add6 = test.add_element(std::make_unique<UI::InfoBox>(info1));
    //_gremote_server_apl(true, "allahyok");
    test.recv_mouse();
    return 0;
};
