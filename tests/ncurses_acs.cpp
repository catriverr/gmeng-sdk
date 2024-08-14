#include <array>
#include <clocale>
#include <ncursesw/ncurses.h>
#include "lib/bin/gmeng.h"
// Function to draw all ncurses ACS characters
void draw_acs_chars() {
    std::array<std::pair<const char*, chtype>, 42> acs_chars = {{
        {"ACS_ULCORNER", ACS_ULCORNER}, {"ACS_LLCORNER", ACS_LLCORNER},
        {"ACS_URCORNER", ACS_URCORNER}, {"ACS_LRCORNER", ACS_LRCORNER},
        {"ACS_LTEE", ACS_LTEE}, {"ACS_RTEE", ACS_RTEE},
        {"ACS_BTEE", ACS_BTEE}, {"ACS_TTEE", ACS_TTEE},
        {"ACS_HLINE", ACS_HLINE}, {"ACS_VLINE", ACS_VLINE},
        {"ACS_PLUS", ACS_PLUS}, {"ACS_S1", ACS_S1},
        {"ACS_S9", ACS_S9}, {"ACS_DIAMOND", ACS_DIAMOND},
        {"ACS_CKBOARD", ACS_CKBOARD}, {"ACS_DEGREE", ACS_DEGREE},
        {"ACS_PLMINUS", ACS_PLMINUS}, {"ACS_BULLET", ACS_BULLET},
        {"ACS_LARROW", ACS_LARROW}, {"ACS_RARROW", ACS_RARROW},
        {"ACS_DARROW", ACS_DARROW}, {"ACS_UARROW", ACS_UARROW},
        {"ACS_BOARD", ACS_BOARD}, {"ACS_LANTERN", ACS_LANTERN},
        {"ACS_BLOCK", ACS_BLOCK}, {"ACS_S3", ACS_S3},
        {"ACS_S7", ACS_S7}, {"ACS_LEQUAL", ACS_LEQUAL},
        {"ACS_GEQUAL", ACS_GEQUAL}, {"ACS_PI", ACS_PI},
        {"ACS_NEQUAL", ACS_NEQUAL}, {"ACS_STERLING", ACS_STERLING},
        {"ACS_BSSB", ACS_BSSB}, {"ACS_SSBB", ACS_SSBB},
        {"ACS_BBSS", ACS_BBSS}, {"ACS_SBBS", ACS_SBBS},
        {"ACS_SBSS", ACS_SBSS}, {"ACS_SSSB", ACS_SSSB},
        {"ACS_SSBS", ACS_SSBS}, {"ACS_BSBS", ACS_BSBS},
        {"ACS_BSSS", ACS_BSSS}, {"ACS_SSSS", ACS_SSSS}
    }};

    int row = 0;
    for (const auto& acs_char : acs_chars) {
        mvprintw(row, 0, "%s", acs_char.first);
        mvaddch(row, 20, acs_char.second);
        ++row;
    }
    const wchar_t *wstr = L"";
    mvaddnwstr(row+1, 0, L"▼", 1); /// IIII LLOOOOOVVEEE YOUU!!!! BABY
}

int main() {
    // Initialize ncurses
    initscr();
    setlocale(LC_ALL, "");
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // Ensure terminal supports ACS characters
    if (!has_colors()) {
        endwin();
        printf("Your terminal does not support ACS characters\n");
        return 1;
    }

    // Draw all ACS characters
    draw_acs_chars();

    // Refresh the screen to show the characters
    refresh();

    // Wait for user input
    getch();

    // End ncurses mode
    endwin();

    return 0;
}
