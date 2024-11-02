#pragma once
#include <stdexcept>
#include <string>

#include "../gmeng.h"

#include "index.h"

using string = std::string;


class inspect_texture_command_t : public Gmeng_Commandline::Subcommand {
  public:
    inline void run(vector<string> args) override {
        if (args.size() < 1) {
            LOG("~r~ERROR!~n~ please provide a texture file name.");
            LOG("~g~Usage:~n~ ~y~" + Gmeng::global.executable + "~n~ preview ~b~file.tx~n~");
            return;
        };
        LOG("Checking for texture atlas in ~g~"+args.at(0)+"~n~...");
        if (!filesystem::exists(args.at(0))) {
            LOG("~r~ERROR!~n~ file ~b~" + args.at(0) + "~n~ does not exist.");
            return;
        }
        Gmeng::texture txt = Gmeng::LoadTexture(args.at(0));
        std::string render = Gmeng::Util::draw_texture_string(txt);
        MSG("~r~Gmeng~n~ TEXTURE PREVIEW:\n");
        MSG("~_~file: `" + args.at(0) + "`~n~\n");
        std::cout << render << std::endl;
    };

    inspect_texture_command_t(string _name, string _description) : Subcommand(_name, _description) {
        this->info = { _name, _description };
    };
};

static inspect_texture_command_t inspect_command("preview", "Displays a preview of a texture file");

static Gmeng_Commandline::InterfaceRegistrar
register_inspect_command(
    std::make_unique<inspect_texture_command_t>( ( inspect_command ) )
);

class test_command_t : public Gmeng_Commandline::Subcommand {
  public:
    inline void run(vector<string>) override {
        LOG("~g~Hello, World!~n~");
        LOG("Current version: " + Gmeng::version);
        LOG("Current scope: " + this->scope.name + " (" + v_str(this->scope.id) + ")");
    };

    test_command_t(string _name, string _description) : Subcommand(_name, _description) {
        this->info = {
            .name = _name,
            .description = _description
        };
    };
};

static test_command_t test_command( "test", "Test Subcommand / Checks command-line system functionality" );

static Gmeng_Commandline::InterfaceRegistrar
register_test_command(
std::make_unique<test_command_t> ( ( test_command ) )
);

#include "../src/editors.cpp"

class editor_command_t : public Gmeng_Commandline::Subcommand {
  public:
    editor_command_t(string _name, string _desc) : Subcommand(_name, _desc) {};
    inline void run(vector<string> args) override {
        MSG("~h~GMENG~n~ editors\n");
        string editor_name;
        try {
            editor_name = args.at(0);
        } catch (std::out_of_range& e) {
            MSG("please specify an editor: ~_~" + Gmeng::global.executable + " /editor <editor_name>~n~\n");
        };
        MSG(">> ~_~searching internal editors for '" + editor_name + "'...~n~\n");
        bool stat = Gmeng::Editors::map.contains(editor_name);
        if (!stat) {
            MSG(">> ~_~\x0F~r~could not find an editor for: " + editor_name + "~n~\n");
            return;
        };
        bool scope_check = Gmeng::Editors::map.find(editor_name)->second->info.scope.id == Gmeng_Commandline::current_scope.id
                           || Gmeng_Commandline::current_scope.id == 0;
        if (!scope_check) {
            MSG(">> ~_~\x0F~r~the execution scope of this editor is different than the user's scope~n~\n");
            return;
        };

        auto editor = Gmeng::Editors::map.find(editor_name)->second.get();
        editor->menu();
    };
};

static editor_command_t editor_command("/editor", "editors for Textures, GLVL and GM map files");

static Gmeng_Commandline::InterfaceRegistrar
register_editor_command(
    std::make_unique<editor_command_t> ( ( editor_command ) )
);


#include <ncurses.h>
#include <fstream>
#include <unordered_map>
#include <string>
#include <iostream>
#include <cstdlib>

void modify_properties(gmeng_properties_t& properties);

using namespace Gmeng;

class gamestate_editor_t : public Gmeng_Commandline::Subcommand {
  public:
    inline void run(vector<string> args) override {
        if (args.size() < 1) {
            LOG("~r~ERROR!~n~ provide a file name.");
            MSG("~g~Usage:~n~ ~y~"+Gmeng::global.executable+" ~b~gamestate~n~ <filename>\n");
            return;
        };

        std::string filename = args.at(0);

        if (!filesystem::exists(filename)) {
            LOG("~r~ERROR!~n~ the file `"+filename+"` does not exist.");
            MSG("~g~Usage:~n~ ~y~"+Gmeng::global.executable+" ~b~gamestate~n~ <filename>\n");
            return;
        };

        gmeng_properties_t properties = read_properties(filename);

        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);

        modify_properties(properties);
        writeout_properties(filename, properties);

        endwin();
    };

    gamestate_editor_t(string _name, string _description) : Subcommand(_name, _description) {
        this->info = {
            .name = _name,
            .description = _description
        };
    };
};

static gamestate_editor_t gamestate_editor_command("gamestate", "Modifies or views a gamestate binary file");

static Gmeng_Commandline::InterfaceRegistrar
register_gamestate_command(
    std::make_unique<gamestate_editor_t>( gamestate_editor_command )
);

void modify_properties(gmeng_properties_t& properties) {
    int highlight = 0;
    int mode = 0;  // 0 = Main properties, 1 = Model positions
    const int num_main_fields = 6;
    bool exit_program = false;

    while (!exit_program) {
        clear();

        // Display section based on mode
        if (mode == 0) {
            mvprintw(0, 0, "Modify Main Properties (Press ENTER to edit, UP/DOWN to navigate, 'm' for model positions, 'q' to quit)");
            mvprintw(2, 0, "1. DEF_DELTAX: %d", properties.DEF_DELTAX);
            mvprintw(3, 0, "2. DEF_DELTAY: %d", properties.DEF_DELTAY);
            mvprintw(4, 0, "3. SKY_WIDTH: %d", properties.SKY_WIDTH);
            mvprintw(5, 0, "4. SKY_HEIGHT: %d", properties.SKY_HEIGHT);
            mvprintw(6, 0, "5. SKY_COLOR: %d", properties.SKY_COLOR);
            mvprintw(7, 0, "6. A00_CAKE_INTERACT_LOOPC: %d", properties.A00_CAKE_INTERACT_LOOPC);
            mvchgat(2 + highlight, 0, -1, A_REVERSE, 0, NULL);
        } else {
            int pos_index = 0;
            mvprintw(0, 0, "Modify Model Positions (Press ENTER to edit, 'a' to add, 'd' to delete, 'm' to go back, 'q' to quit)");
            for (const auto& [key, point] : properties.model_positions) {
                mvprintw(2 + pos_index, 0, "%d. %s: (%d, %d)", pos_index + 1, key.c_str(), point.x, point.y);
                if (pos_index == highlight) {
                    mvchgat(2 + pos_index, 0, -1, A_REVERSE, 0, NULL);
                }
                pos_index++;
            }
        }

        int ch = getch();
        switch (ch) {
            case KEY_UP:
                highlight = (highlight - 1 + (mode == 0 ? num_main_fields : properties.model_positions.size())) %
                            (mode == 0 ? num_main_fields : properties.model_positions.size());
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % (mode == 0 ? num_main_fields : properties.model_positions.size());
                break;
            case '\n': {
                if (mode == 0) {
                    int new_value;
                    echo();
                    mvprintw(10, 0, "Enter new value: ");
                    scanw("%d", &new_value);
                    noecho();

                    switch (highlight) {
                        case 0: properties.DEF_DELTAX = new_value; break;
                        case 1: properties.DEF_DELTAY = new_value; break;
                        case 2: properties.SKY_WIDTH = new_value; break;
                        case 3: properties.SKY_HEIGHT = new_value; break;
                        case 4: properties.SKY_COLOR = (color_t)new_value; break;
                        case 5: properties.A00_CAKE_INTERACT_LOOPC = new_value; break;
                    }
                } else if (!properties.model_positions.empty()) {
                    auto it = properties.model_positions.begin();
                    std::advance(it, highlight);
                    echo();
                    int new_x, new_y;
                    mvprintw(10, 0, "Enter new x value: ");
                    scanw("%d", &new_x);
                    mvprintw(11, 0, "Enter new y value: ");
                    scanw("%d", &new_y);
                    noecho();

                    it->second.x = new_x;
                    it->second.y = new_y;
                }
                break;
            }
            case 'm':
                mode = 1 - mode;  // Toggle between main properties and model positions
                highlight = 0;
                break;
            case 'a':
                if (mode == 1) {
                    char new_key[64];
                    int new_x, new_y;
                    echo();
                    mvprintw(10, 0, "Enter new key: ");
                    getstr(new_key);
                    mvprintw(11, 0, "Enter x value: ");
                    scanw("%d", &new_x);
                    mvprintw(12, 0, "Enter y value: ");
                    scanw("%d", &new_y);
                    noecho();

                    properties.model_positions[std::string(new_key)] = {new_x, new_y};
                }
                break;
            case 'd':
                if (mode == 1 && !properties.model_positions.empty()) {
                    auto it = properties.model_positions.begin();
                    std::advance(it, highlight);
                    properties.model_positions.erase(it);
                    highlight = std::min(highlight, static_cast<int>(properties.model_positions.size()) - 1);
                }
                break;
            case 'q':
                exit_program = true;
                break;
        }
    }
};
