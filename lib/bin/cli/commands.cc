#pragma once
#include <stdexcept>
#include <string>

#include "index.h"

using string = std::string;

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
    };
};

static editor_command_t editor_command("/editor", "editors for Textures, GLVL and GM map files");

static Gmeng_Commandline::InterfaceRegistrar
register_editor_command(
    std::make_unique<editor_command_t> ( ( editor_command ) )
);
