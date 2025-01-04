/* modules */
#define __GMENG_ALLOW_LOG__ true

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#undef FDEV_SETUP_STREAM
#define FDEV_SETUP_STREAM(p, g, f) \
    {                              \
        .buf = NULL,               \
        .unget = 0,                \
        .flags = f,                \
        .size = 0,                 \
        .len = 0,                  \
        .put = p,                  \
        .get = g,                  \
        .udata = 0                 \
    }

/* files */
#include "../gmeng.h"
#include "../cli/index.h"


int main(int argc, char** argv) {

    patch_argv_global(argc, argv);
    Gmeng_Commandline::patch_argv(argc, argv);
    init_logc(1000);

    if (!Gmeng::global.shush) {
      SAY("~h~\x0F~y~WARN! ~_~This CLI is a work-in-progress beta.\n");
      SAY("~h~contribute to ~b~https://github.com/catriverr/gmeng-sdk~n~\n");
      SAY("~_~\x0F~i~please report bugs to ~b~https://gmeng.org/report~n~\n\n"); };

    if (Gmeng::global.dont_hold_back) SAY("\narguments: " + Gmeng::global.executable + " " + g_joinStr(Gmeng_Commandline::arguments, " ") + "\n");

    /// "executable_name" "subcommand" "...arguments"
    /// [0]               [1]          [2..n]
    vector<std::string> redone_args;

    for (int i = 0; i < argc; i++)
      if (!std::string(argv[i]).starts_with("-"))
        redone_args.push_back(argv[i]);

    redone_args.erase(redone_args.begin());

    if (Gmeng::global.dev_mode) {
         SAY("total subcommands: " + v_str(Gmeng_Commandline::subcommands.size()) + "\n");
         SAY("executable: " + Gmeng::global.executable + "\n");
         SAY("user: " + Gmeng::global.user + "\n");
         SAY("working directory: " + Gmeng::global.pwd + "\n");
         for (const auto& subcmd_ : Gmeng_Commandline::subcommands) {
             SAY("SUBCMD_NAME: " + subcmd_.first + "\n");
         };
    };

    if (redone_args.size() < 1) {
        INF("no subcommands provided so you get the help menu instead\n");
        char* args[] = { argv[0], "-help"};
        patch_argv_global(2, args);
    };

    string subcmd = redone_args.size() >= 1 ? redone_args[0] : "";
    if ( subcmd.empty() ) {
        SAY("~r~cheater! provide a subcommand name~n~\n");
        /// we should not reach this point
    } else if (!Gmeng_Commandline::subcommands.contains(subcmd)) {
        SAY("~r~unknown command: ~n~" + subcmd + "\n");
    } else {
        vector<string> params = redone_args;
        params.erase(params.begin());
        Gmeng_Commandline::get_subcommand(subcmd)->run(params);
    };
    return 0;
};
