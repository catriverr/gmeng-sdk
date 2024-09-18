#pragma once
#include <string>

#include "index.h"

using string = std::string;

class test_command_t : public Gmeng_Commandline::Subcommand {
  public:
    inline void run(vector<string>) override {
        LOG("~g~Hello, World!~n~\nexciting times...");
        LOG("Current version: " + Gmeng::version);
    };

    test_command_t(string _name, string _description) {
        this->info = {
            .name = _name,
            .description = _description
        };
    };
};

static test_command_t test_command( "test", "Test Subcommand / Checks command-line system functionality" );

static Gmeng_Commandline::InterfaceRegistrar
register_test_cc_command(
            std::make_unique<test_command_t> ( ( test_command ) )
        );
