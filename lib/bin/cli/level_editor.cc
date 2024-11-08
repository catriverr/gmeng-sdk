#pragma once
#include <cstddef>
#include <cstring>
#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <string>
#include "../gmeng.h"  // Replace with your actual header
#include "../utils/serialization.cpp"
#include "../cli/index.h"

using namespace Gmeng;

static std::string filename;
static Gmeng::Level level;

void getinput(const char *format, void *value, const char* autoval = "") {
    int ch;
    char buffer[100];  // Buffer to store user input
    int pos = 0;       // Position of the current character in the buffer

    // Initialize the input buffer based on the format type
    if (strcmp(format, "%d") == 0) {
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer
    } else if (strcmp(format, "%s") == 0) {
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer
    }


    // Display instructions
    refresh();

    while (true) {
        ch = getch();  // Capture a single character input

        if (ch == KEY_BACKSPACE || ch == 127) { // Handle backspace
            if (pos > 0) {
                buffer[--pos] = '\0'; // Remove the last character
                int x, y;
                getyx(stdscr, y, x);
                ::move(y, x-1);
                printw(" ");
                ::move(y, x-1);
            }
        } else if (ch == '\n') {  // When Enter is pressed, break the loop
            break;
        } else if (ch >= 32 && ch <= 126) {  // If it's a printable character, add to the buffer
            if (pos < sizeof(buffer) - 1) {
                buffer[pos++] = (char)ch;
                buffer[pos] = '\0'; // Null-terminate the string
                printw("%c", (char)ch);
            }
        }

        refresh();
    };

    // Parse the final input based on the format
    if (strcmp(format, "%d") == 0) {
        *(int*)value = atoi(buffer);  // Convert the buffer to an integer
    } else if (strcmp(format, "%s") == 0) {
        strcpy((char*)value, buffer);  // Copy the buffer to the provided string
    }
};

void display_header(WINDOW* win, const std::string& filename, const Gmeng::Level& level) {
    mvwprintw(win, 1, 0, ("[ Gmeng "+ (Gmeng::version) +" - build "+(GMENG_BUILD_NO)+" ]").c_str());
    mvwprintw(win, 2, 0, "[ Level File Inspector - %s - %s %s | %zu x %zu ]",
              filename.c_str(), level.name.c_str(), level.desc.c_str(),
              level.base.width, level.base.height);
    mvwprintw(win, 3, 0, "[ press 'q' to quit, LEFT/RIGHT to navigate chunks, UP/DOWN to navigate options, ENTER to select an option. ]");
};

void list_chunks(WINDOW* win, const vector<Gmeng::chunk> chunks, int selected_chunk) {
    int pos = 0, chunk_number = 0;
    for (auto& chunk : chunks) {
        if (chunk_number == selected_chunk) attron(A_REVERSE);
        mvwprintw(win, 5, pos, "Chunk %d", chunk_number + 1);
        attroff(A_REVERSE);
        pos += std::string("Chunk " + v_str(chunk_number)).length()+2;
        chunk_number++;
    };
};

// Function to display chunk details
void show_chunk_details(WINDOW* win, const Gmeng::chunk& chunk, int chunk_number, int selected_model) {
    mvwprintw(win, 9, 0, "---- objects ----");
    for (size_t i = 0; i < chunk.models.size(); ++i) {
        const auto& model = chunk.models[i];
        if (i == selected_model) attron(A_REVERSE);
        mvwprintw(win, 10 + i, 0, ("%d: (%d) - " + model.texture.name + " [%zu x %zu] @(%d, %d)").c_str(),
                  (int)i + 1, model.id, model.texture.width,
                  model.texture.height, model.position.x, model.position.y);
        attroff(A_REVERSE);
    };
};

void edit_model(WINDOW* win, vector<Gmeng::chunk>& chunks, int selected_chunk, int selected_model) {
    clear();
    bool editing_done = false;
    bool editing_param = false;
    int selected_param = 0;

    Renderer::Model mdl = chunks.at(selected_chunk).models.at(selected_model);

    while (!editing_done) {
        if (!editing_param) {
        display_header(win, filename, level);
        list_chunks(win, chunks, selected_chunk);
        mvwprintw(win, 10, 0, "editing model %d - press `ESC` to return to main menu.", selected_model);

            for (int lndx = 0; lndx < 3; lndx++) {
                if (selected_param == lndx) attron(A_REVERSE);
                switch (lndx) {
                    case 0: {
                                mvwprintw(win, 11, 0, "name: %s", mdl.name.c_str());
                            };
                        break;
                    case 1: {
                                mvwprintw(win, 12, 0, "pos_x: %d", mdl.position.x);
                            };
                        break;
                    case 2: {
                                mvwprintw(win, 13, 0, "pos_y: %d", mdl.position.y);
                            };
                        break;
                };
                attroff(A_REVERSE);
            };
   /// ///
            int ch = getch();

            switch (ch) {
                case KEY_UP:
                    if (selected_param > 0) --selected_param;
                    break;
                case KEY_DOWN:
                    if (selected_param < 2) ++selected_param;
                    break;
                case 10: // Enter
                    editing_param = true;
                    break;
                case 'q':
                    endwin();
                    write_level_data(filename, level);
                    exit(0);
                    break;
                case 27: // ESC
                    editing_done = true;
                    break;
            };
        } else {
            clear();
            display_header(win, filename, level);
            list_chunks(win, chunks, selected_chunk);
            mvwprintw(win, 10, 0, "editing model %d - press `ESC` to return to main menu.", selected_model);
            switch (selected_param) {
                case 0: {
                            mvwprintw(win, 11, 0, "name: ");
                            std::string tx;
                            getinput("%s", &tx);
                            mdl.name = tx;
                        };
                    break;
                case 1: {
                            mvwprintw(win, 12, 0, "pos_x: ");
                            getinput("%d", &mdl.position.x);
                        };
                    break;
                case 2: {
                            mvwprintw(win, 13, 0, "pos_y: ");
                            getinput("%d", &mdl.position.y);
                        };
                    break;
            };
            editing_param = false;
        };
        clear();
        level.chunks.at(selected_chunk).models.at(selected_model) = mdl;
    };

    chunks.at(selected_chunk).models.at(selected_model) = mdl;
};

// Main function to navigate through chunks
void navigate_and_edit(WINDOW* win, Gmeng::Level& level) {
    int selected_chunk = 0;
    int selected_model = 0;

    bool editing_model = false;

    while (true) {
        //clear();

        if (editing_model) { edit_model(win, level.chunks, selected_chunk, selected_model); editing_model = false; clear(); continue; };


        display_header(win, filename, level);
        list_chunks(win, level.chunks, selected_chunk);
        show_chunk_details(win, level.chunks[selected_chunk], selected_chunk, selected_model);


        // Handle user input
        int ch = getch();
        switch (ch) {
            case KEY_LEFT:
                if (selected_chunk > 0) --selected_chunk;
                selected_model = 0;
                break;
            case KEY_RIGHT:
                if (selected_chunk < level.chunks.size() - 1) ++selected_chunk;
                selected_model = 0;
                break;
            case KEY_UP:
                if (selected_model > 0) --selected_model;
                break;
            case KEY_DOWN:
                if (selected_model < level.chunks.at(selected_chunk).models.size() - 1) ++selected_model;
                break;
            case 10: // enter
                editing_model = true;
                break;
            case 'q': // Quit the utility
                if (editing_model) break;
                endwin();
                return;
                break;
        };
        refresh();
    }
}

class level_file_editor_command_t : public Gmeng_Commandline::Subcommand {
  public:
    level_file_editor_command_t(string name, string desc): Subcommand(name, desc) {};

    void run(vector<std::string> args) {
        if (args.size() < 1) {
            LOG("~r~ERROR!~n~ please provide a filename.\n\t~g~Usage:~n~ " + Gmeng::global.executable + " ~b~glvl~n~ <filename>");
            return;
        }

        filename = args[0];

        read_level_data(filename, level);

        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);

        int height, width;
        getmaxyx(stdscr, height, width);
        WINDOW* main_win = stdscr;
        display_header(main_win, filename, level);
        navigate_and_edit(main_win, level);

        endwin(); // Clean up ncurses
        write_level_data(filename, level);
        return;
    };
};

static level_file_editor_command_t lfec("glvl", "allows you to inspect a binary GLVL file");

static Gmeng_Commandline::InterfaceRegistrar register_lfec(
    std::make_unique<level_file_editor_command_t>( lfec )
);
