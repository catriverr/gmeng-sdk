#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdlib.h>
#include <cwchar>

namespace fs = std::filesystem;

#ifndef _WIN32
  #include <ncursesw/ncurses.h>

    class BasicWindowHelper {
public:
    BasicWindowHelper() {
        initscr();              // Initialize ncurses
        setlocale(LC_ALL, "");
        raw();
        start_color();          // Enable color functionality
        cbreak();               // Line buffering disabled
        noecho();               // Don't print input characters
        keypad(stdscr, TRUE);   // Enable special keys
        curs_set(0);
        this->HALF_X = COLS/2; this->HALF_Y = COLS/2;
    }

    uint32_t HALF_X = COLS/2; uint32_t HALF_Y = LINES/2;

    ~BasicWindowHelper() {
        curs_set(1);
        endwin();               // End ncurses mode
    }

    std::vector<int> Centralize(std::string text) {
        return {
            (COLS - (int)text.length())/2,
            (LINES)/2
        };
    };

    std::vector<int> Centralize(std::wstring text) {
        return {
            (COLS - (int)text.length())/2,
            (LINES)/2
        };
    }

    void Text(const std::string& text, int color_pair) {
        // Check if the color pair is valid
        if (color_pair >= 0 && color_pair < COLOR_PAIRS) {
            attron(COLOR_PAIR(color_pair));
            printw("%s", text.c_str());
            attroff(COLOR_PAIR(color_pair));
        } else {
            printw("Invalid color pair: %d", color_pair);
        }
        this->Refresh();
    };

    void Text(const std::string& text, int color_pair, int x, int y) {
        // Move to the specified position and print text with the specified color pair
        if (color_pair >= 0 && color_pair < COLOR_PAIRS) {
            move(y, x);
            attron(COLOR_PAIR(color_pair));
            printw("%s", text.c_str());
            attroff(COLOR_PAIR(color_pair));
        } else {
            move(y, x);
            printw("Invalid color pair: %d", color_pair);
        }
    }

    void Text(const std::wstring& wtext, int color_pair) {
        // Print wide text with the specified color pair at the current cursor position
        if (color_pair >= 0 && color_pair < COLOR_PAIRS) {
            attron(COLOR_PAIR(color_pair));
            // Print wide string directly using ncursesw functions
            addwstr(wtext.c_str());
            attroff(COLOR_PAIR(color_pair));
        } else {
            addwstr(L"Invalid color pair");
        }
    }

    void Text(const wchar_t* wtext, int color_pair, int x, int y) {
        // Move to the specified position and print wide text with the specified color pair
        if (color_pair >= 0 && color_pair < COLOR_PAIRS) {
            move(y, x);
            attron(COLOR_PAIR(color_pair));
            // Print wide string directly using ncursesw functions
            mvaddnwstr(y,x,wtext,wcslen(wtext));
            attroff(COLOR_PAIR(color_pair));
        } else {
            move(y, x);
            addwstr(L"Invalid color pair");
        }
    }


    void InitColorPair(short pair_number, short fg_color, short bg_color) {
        init_pair(pair_number, fg_color, bg_color);
    }

    void DrawRect(int x1, int y1, int x2, int y2, short color_pair) {
        if (color_pair >= 0 && color_pair < COLOR_PAIRS) {
            // Draw the rectangle with the specified background color
            attron(COLOR_PAIR(color_pair));
            for (int y = y1; y <= y2; ++y) {
                move(y, x1); // Move to the starting x position of the current row
                for (int x = x1; x <= x2; ++x) {
                    addch(' '); // Print a space character to fill the rectangle
                }
            }
            attroff(COLOR_PAIR(color_pair));
        } else {
            printw("Invalid color pair: %d", color_pair);
        }
    };

    void Refresh() {
        refresh();
    };

    void SetBackgroundColor(short color) {
        if (has_colors()) {
            bkgd(COLOR_PAIR(color)); // Set the background color for the entire screen
            clear(); // Clear the screen to apply the background color
        }
    }

private:
    // Private method to initialize colors (you can expand this if needed)
    void SetupColors() {
        if (has_colors()) {
            // Set up some default color pairs
            InitColorPair(1, COLOR_RED, COLOR_BLACK);
            InitColorPair(2, COLOR_GREEN, COLOR_BLACK);
            InitColorPair(3, COLOR_BLUE, COLOR_BLACK);
        }
    }
};
#endif

std::wstring repeat_wstring(const std::wstring& wstr, size_t count) {
    std::wstring result;
    result.reserve(wstr.size() * count); // Reserve space for efficiency
    for (size_t i = 0; i < count; ++i) {
        result.append(wstr);
    }
    return result;
};


enum input_type {
    STRING,
    NUMBER,
    BOOLEAN,
    CLICK
};

struct selection {
    std::string title;
    std::string value;
    input_type type = STRING;
};

bool conv_int_check(const std::string& str) {
    try {
        std::stoi(str); // Try to convert the string to an integer
        return true;    // Conversion succeeded
    } catch (const std::invalid_argument& e) {
        // Caught when the string does not contain a valid integer
        return false;
    } catch (const std::out_of_range& e) {
        // Caught when the number is out of the range of an int
        return false;
    }
}

bool write_to_file(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) return false;
    file << content;
    if (file.fail()) return false;
    file.close();
    return true;
};

std::string create_text(std::vector<std::vector<selection>> data) {
    std::string final = "";
    for (const auto& sel : data) {
        for (auto& v : sel) {
            if (v.type == CLICK) continue;
            final += v.title + " := " + v.value + "\n";
        };
    };
    return final;
};


int main(int argc, char** argv) {
#if _WIN32
    std::cout << "this utility is not available for Windows systems at this time.\n";
    exit(1);
#endif
    BasicWindowHelper window;
    window.InitColorPair(4, COLOR_BLACK,  COLOR_BLUE);
    window.SetBackgroundColor(4);
    addch(ACS_ULCORNER);
    addch(ACS_HLINE);
    addch(ACS_VLINE);
    addch(ACS_URCORNER);
    addch(ACS_LLCORNER);
    addch(ACS_LRCORNER);
    addch(ACS_LTEE);
    addch(ACS_RTEE);
    std::wstring hitext = L"┌" + repeat_wstring(L"─", COLS-22) + L"┐";
    std::wstring hitext_u = L"└" + repeat_wstring(L"─", COLS-22) + L"┘";
    window.InitColorPair(5, COLOR_WHITE, COLOR_BLACK);
    window.InitColorPair(10, COLOR_RED, COLOR_WHITE);
    window.InitColorPair(11, COLOR_BLACK, COLOR_WHITE);
    attron(COLOR_PAIR(5));
    move(6,11);
    attron(COLOR_PAIR(4));
    wvline(stdscr,L'│',LINES-12);
    move(6,12+(COLS-22));
    wvline(stdscr,L'│',LINES-12);
    attroff(COLOR_PAIR(4));
    window.Text(repeat_wstring(L"\u2584",COLS-20).c_str(),4,11,5);
    window.Text(repeat_wstring(L"\u2580",COLS-20).c_str(),4,11,LINES-6);
    std::wstring title = L"GMENG BUILD CONFIGURATION";
    std::vector<int> pos2 = window.Centralize(title);
    window.DrawRect(12,6,COLS-11,LINES-7,11);
    window.Text(title.c_str(),10,pos2[0]+1,7);
    std::string navtext = "<tab> to navigate, <enter> to select, <esc> to go back, <q> to quit";
    std::vector<int> pos = window.Centralize(navtext);
    window.Text(navtext,11,pos[0]+1,9);
    window.Refresh();
    int ch;
    int position = 0;

    window.InitColorPair(9,COLOR_GREEN,COLOR_BLACK);
    window.InitColorPair(12, COLOR_RED,COLOR_BLACK);
    std::vector<std::vector<selection>> selections = {
      {
        { "USE_NCURSES", "true", BOOLEAN }, { "USE_EXTERNAL", "false", BOOLEAN },
      },
      {
        { "TARGET_NAME", "all", STRING }, { "DEBUG_MODE", "false", BOOLEAN }
      },
      {
        { " <OK>", "", CLICK }, { " <CANCEL>", "", CLICK }
      }
    };

    selection* current = nullptr;

        window.InitColorPair(8, COLOR_BLUE, COLOR_WHITE);
        window.InitColorPair(7, COLOR_MAGENTA, COLOR_WHITE);
        window.InitColorPair(20, COLOR_WHITE, COLOR_BLACK);
        window.InitColorPair(21, COLOR_YELLOW, COLOR_BLACK);
        window.InitColorPair(22, COLOR_BLUE, COLOR_BLACK);

    auto refresh_selections = [&]() {
                int id = 0; int pd = 0;


        attron(COLOR_PAIR(11));
        for (auto& sel : selections) {
            for (auto& p : sel) {
                move(
                        11+(pd*3),
                        25+(id%2==1 ? 30:0)
                    );
                int COLO = position == id ? 21 : 20;
                attron(COLOR_PAIR(COLO));
                addstr(p.title.c_str());
                addch(' ');
                attroff(COLOR_PAIR(COLO));
                switch (p.type) {
                    default:
                    case STRING:
                        attron(COLOR_PAIR(22));
                        addstr((p.value.length() > 0 ? p.value : std::string("")).c_str());
                        attroff(COLOR_PAIR(22));
                        attron(COLOR_PAIR(11));
                        addch(' ');
                        attroff(COLOR_PAIR(11));
                        break;
                    case NUMBER:
                        attron(COLOR_PAIR(7));
                        addstr(p.value.c_str());
                        attroff(COLOR_PAIR(7));
                        break;
                    case BOOLEAN:
                        attron(COLOR_PAIR(p.value=="true" ? 9:12));
                        addstr((p.value + (p.value == "true" ? " " : "")).c_str());
                        attron(COLOR_PAIR(p.value=="true" ? 9:12));
                        break;
                };
                id++;
            };
            pd++;
        };

        window.Text("Keep in mind that this configuration is going to overwrite", 11, 20, 20);
        window.Text("your existent configuration in this directory. This utility does", 11, 20, 21);
        window.Text("not load your existent configuration, make sure that the", 11, 20, 22);
        window.Text("preferences above are correctly set for your program.", 11, 20, 23);

        window.Text("USE_NCURSES: For UI Elements. not required to run gmeng.", 11, 20, 26);
        window.Text("USE_EXTERNAL: For SDL windows. not required to run gmeng.", 11, 20, 27);
        window.Text("TARGET_NAME: MAKE phony name / path to your program.", 11, 20, 28);
        window.Text("DEBUG_MODE: For debugging with LLDB & better logging.", 11, 20, 29);


        attroff((COLOR_PAIR(11)));
    };

    refresh_selections();

    while((ch = getch()) != ERR) {
        if (current != nullptr && ch != 27) {
            attron(22);
            mvaddch(COLS-3,0,ch);
            attroff(22);
            switch (current->type) {
                case STRING:
                    if (ch == KEY_BACKSPACE || ch == 127 || ch == 263) {
                        if (current->value.length() > 0) current->value.pop_back();
                    } else {
                        if (current->value.length() < 17) current->value += ch;
                    };
                    refresh_selections();
                    break;
                case BOOLEAN:
                    if (ch == 10) current->value = current->value == "true" ? "false" : "true";
                    break;
                case NUMBER:
                    std::string dat = "";
                    dat += (char)ch;
                    if (conv_int_check( dat )) current->value += ch;
                    break;
            };
        } else {
            switch(ch) {
                case 'q':
                    curs_set(1);
                    endwin();
                    exit(0);
                    break;
                case 9://tab
                    position++;
                    if (position >= selections.size()*2
                        || position < 0) position = 0;
                    break;
                case 27://esc
                    current = nullptr;
                    break;
                case 10://enter
                    auto& selec = selections[position/2][!(position%2==0)];
                    if (selec.type == BOOLEAN) { selec.value = selec.value == "true" ? "false" : "true"; current = nullptr; };
                    if (selec.type == CLICK) {
                        if (selec.title == " <OK>") {
                            bool check = write_to_file("buildoptions.mk", create_text(selections));
                            if (!check) {
                                curs_set(1);
                                endwin();
                                std::cout << "ERROR! write() failed, please ensure the program has the correct permissions to modify the `buildoptions.mk` file in the same directory as this makefile.\n";
                                exit(1);
                            } else {
                                curs_set(1);
                                endwin();
                                std::cout << "Configuration succeeded! you may now run `make` to build this program.\n";
                                exit(0);
                            };
                        } else if (selec.title == " <CANCEL>") {
                                curs_set(1);
                                endwin();
                                std::cout << "Abort configuration\n";
                                exit(2);
                        };
                    } else if (selec.type != BOOLEAN) current = &selec;
                    break;
            };
        }
        attron(COLOR_PAIR(11));
        mvaddstr(6,12,(std::to_string(ch) + " ").c_str());
        attroff(COLOR_PAIR(11));
        refresh_selections();
    };
    exit(0);
};
