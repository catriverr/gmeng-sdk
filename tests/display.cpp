#include <iostream>
#include "../lib/bin/gmeng.hpp"

using std::endl;

int main() {
    gm::t_display display = {
        .pos = { .x=10, .y=10 },
        .v_cursor = 0,
        .v_width = 15,
        .v_height = 8,
        .v_outline_color = 1,
        .v_textcolor = 2
    };
    std::cout << "init" << endl;
    _udisplay_init(display);
    std::cout << "_udisplay_init() v_success" << endl;
    _utext(display, "hello\x0f"); /// \n and \x0f (Gmeng::d_endl) are line-formatters (x0f_formatter_0 || newline) -> seeks to next pos in v_drawpoints->row(++x)->column(0)
    std::cout << "_utext() v_success" << endl;
    _udraw_display(display);
    std::cout << "\n\n_udraw_display() v_success" << endl;
    int i = 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    display.v_outline_color = 4;
    _utext(display, "vl_uknown");
    _udraw_display(display);
    std::thread([&]() {
    for ( ;; ) {
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        display.v_textcolor = i % 8;
        _udraw_display(display);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        i++;
    };
    }).join();
    return 0;
};
