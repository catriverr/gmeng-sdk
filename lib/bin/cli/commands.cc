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

using namespace Gmeng;




class netstream_util_t : public Gmeng_Commandline::Subcommand {
  public:
    netstream_util_t(string _name, string _description) : Subcommand(_name, _description) {
        this->info = {
            .name = _name,
            .description = _description
        };
    };

    inline void run( vector<string> args ) override {
        if (args.size() < 2) {
            LOG("provide a server path");
            LOG("usage: gmeng stream GET localhost:7388/&streamgame");
            return;
        };
        std::string _method = args.at(0);
        std::string path = args.at(1);
        std::cout << _method << ' ' << path << '\n';
        path_type_t method = _method == "GET" ? path_type_t::GET : path_type_t::POST;
        std::cout << "sending request..\n";
        send_stream_request(method, path,
        [&](std::string& command, long long latency, stream_util& util) {
            LOG (command);
            std::cout << ">> ";
            std::string ginput = lineinput(false);
            util.send_data( streamstr(ginput) );
        });
        return;
    };
};

static netstream_util_t netcmd("stream", "Connects to a Gmeng::Networking style network stream.");

static Gmeng_Commandline::InterfaceRegistrar register_netstream_command(
    std::make_unique<netstream_util_t>( netcmd )
);
