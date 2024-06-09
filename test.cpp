#include <chrono>
#include <iostream>
#include "./lib/bin/gmeng.hpp"
#include "./lib/bin/utils/envs/editor.cpp"
#include "lib/bin/def/renderer.cpp"
#include <ncurses.h>
#include <thread>

using namespace Gmeng;

int main() {
    Gmeng::UI::Screen test;
    test.initialize();
    auto func = [&](UI::ButtonInteraction mb) {
        test.text("hi there!", UI_WHITE, UI_BLACK, { 50, 3 });
    };
    auto button1 = UI::Button({25, 10}, "button1", false, UI_WHITE, UI_BGRED, func);
    button1.background_color_highlight = UI_BGYELLOW;
    button1.hovered = false;
    button1.refresh(test, UI::NONE);
    bool add = test.add_element(button1);
    if (!add) test.text("something went wrong.", UI_RED, UI_BLACK, { 20, 20 });
    test.recv_mouse();
    return 0;
};
