#pragma once
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../gmeng.h"
#include "../strops.h"

#define vector std::vector
#define size_t std::size_t

namespace Gmeng_Commandline {
  using string = std::string;

    static int argc = 0;
    static char** argv;
    static vector<string> arguments;
    static bool init_arguments;

    static void patch_argv(int _argc, char** _argv) {
        if (init_arguments) arguments.clear();
        argc = _argc; argv = _argv;
        for (int in = 0; in < _argc; in++)
          arguments.push_back(_argv[in]);
        init_arguments = true;
    };

    static bool check_scaremongerers() {
        bool found;
        for (const auto& val : arguments)
          if (val == "--i-am-a-scaremongerer") { found = true; break; };
        return found;
    };

    class Subcommand {
      protected:
          struct subcmd_info_t {
            std::string name; std::string description;
            size_t arg_count; size_t min_arg_count;
            bool requires_arguments = false;
          };
      public:
        subcmd_info_t info;

        virtual ~Subcommand() = default;

        virtual bool check_arguments(vector<string> arguments) final {
            if ((this->info.requires_arguments
            &&   this->info.arg_count > 0) && (
            this->info.min_arg_count > arguments.size()
            )) {
                for (const auto& arg : arguments)
                  if (arg == "--?") {
                    if (!this->info.description.empty()) SAY("~h~" + Gmeng::global.executable + " ~y~" + this->info.name + "~n~: ~_~" + this->info.description);
                    else SAY("~h~\x0F~r~cannot determine the functionality of this subcommand");
                    return false;
                  };
            };
            return true;
        };


        /// Base logger for the current subcommand
        /// not internal
        virtual void LOG(std::string data) final {
            std::string text = data;
            replace_all(text, "\n", "\n" + repeatString(" ", 3 + (int)this->info.name.length()  ));
            SAY("[" + this->info.name + "] " + text + "\n");
        };

        virtual void run(vector<string> arguments) = 0;
        virtual void help(vector<string> arguments = {}) { /* no_base_func */ };
    };

    static std::map<string, std::unique_ptr<Subcommand>> subcommands;

    template<typename T>
    static void register_subcommand(std::unique_ptr<T> cmd) {
        Gmeng_Commandline::subcommands.insert_or_assign(
            cmd->info.name, std::move( cmd )
        );
    };

    static std::shared_ptr<Subcommand> get_subcommand(string name) {
        if (!Gmeng_Commandline::subcommands.contains(name)) return nullptr;
        auto returned_ob = Gmeng_Commandline::subcommands.find(name)->second.get();
        return std::shared_ptr<Subcommand> ( returned_ob, [](Subcommand*) {} );
    };

    struct execution_scope {
        bool elevated = false;
        int id; string name;
        execution_scope(bool _elevated, int _id, string _name) : elevated(_elevated), id(_id), name(_name) {};
    };

    static const execution_scope gmng_internal = execution_scope(false, 0, "gmeng:internal");

    class InterfaceRegistrar {
      public:
          InterfaceRegistrar(std::unique_ptr<Subcommand> cmd) {
              Gmeng_Commandline::register_subcommand( std::move( cmd ) );
          };
    };
};


#ifndef __GMENG_COMMANDLINE_IMPORTS__

#include "./test.cc"

#define __GMENG_COMMANDLINE_IMPORTS__ true

#endif
