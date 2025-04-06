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

    struct execution_scope {
        bool elevated = false;
        int id; string name;
        execution_scope(bool _elevated, int _id, string _name) : elevated(_elevated), id(_id), name(_name) {};
    };

    static const execution_scope gmng_user = execution_scope(false, g_mkid(), get_username() + ":tty");
    static const execution_scope gmng_internal = execution_scope(true, 0, "gmeng:internal");

    static int argc = 0;
    static char** argv;
    static vector<string> arguments;
    static bool init_arguments;
    static execution_scope current_scope = gmng_user;

    static void patch_argv(int _argc, char** _argv) {
        __functree_call__(Gmeng_Commandline::patch_argv);
        if (init_arguments) arguments.clear();
        argc = _argc; argv = _argv;
        for (int in = 0; in < _argc; in++)
          arguments.push_back(_argv[in]);
        init_arguments = true;
        if (Gmeng::global.dev_mode) current_scope = gmng_internal;
    };

    static bool check_scaremongerers() {
        __functree_call__(Gmeng_Commandline::check_scaremongerers);
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
        execution_scope scope = gmng_user;
        Subcommand(string _name, string _description) {
            this->info = {
                .name = _name,
                .description = _description
            };
        };

        virtual ~Subcommand() = default;

        virtual bool check_arguments(vector<string> arguments) final {
            __functree_call__(Gmeng_Commandline::Subcommand::__instance_any__::check_arguments);
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
        /// not internal / for subsystem logging
        /// for text-based messages to the console,
        /// use MSG(x)
        virtual void LOG(std::string data) final {
            std::string text = data;
            replace_all(text, "\n", "\n" + repeatString(" ", 14 + (int)this->info.name.length()  ));
            std::string msg = "(" + this->scope.name + ")[" + this->info.name + "] " + text;
            gm_log(msg);
            if (!check_scaremongerers()) SAY(msg + "\n");
        };

        virtual void MSG(std::string data) final {
            gm_log("[" + this->info.name + "] " + data + "\n");
            if (!check_scaremongerers()) SAY(data);
        };

        virtual void run(vector<string> arguments) = 0;
        virtual void help(vector<string> arguments = {}) { /* no_base_func */ };
    };

    static std::map<string, std::unique_ptr<Subcommand>> subcommands;

    template<typename T>
    static void register_subcommand(std::unique_ptr<T> cmd) {
        __functree_call__(Gmeng_Commandline::register_subcommand);
        Gmeng_Commandline::subcommands.insert_or_assign(
            cmd->info.name, std::move( cmd )
        );
    };

    static std::shared_ptr<Subcommand> get_subcommand(string name) {
        __functree_call__(Gmeng_Commandline::get_subcommand);
        if (!Gmeng_Commandline::subcommands.contains(name)) return nullptr;
        auto returned_ob = Gmeng_Commandline::subcommands.find(name)->second.get();
        if (Gmeng::global.dev_mode) returned_ob->scope = gmng_internal;
        return std::shared_ptr<Subcommand> ( returned_ob, [](Subcommand*) {} );
    };

    class InterfaceRegistrar {
      public:
          InterfaceRegistrar(std::unique_ptr<Subcommand> cmd) {
              __functree_call__(Gmeng_Commandline::InterfaceRegistrar::registry_component_update);
              Gmeng_Commandline::register_subcommand( std::move( cmd ) );
          };
    };
};


#ifndef __GMENG_COMMANDLINE_IMPORTS__

#include "./commands.cc"
#include "./level_editor.cc"
#include "./basic_songmaker.cc"

#define __GMENG_COMMANDLINE_IMPORTS__ true

#endif
