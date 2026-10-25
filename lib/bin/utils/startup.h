#pragma once

#include <string>
#include <unistd.h>

#include "./ioctl.h"

#include "../types/color32.h"
#include "../src/version.h"

#include "colorformat.h"
#include "warning.h"
#include "repeat.h"
#include "kitty.h"


static std::string boolstr(bool x) {
    return x ? "true" : "false";
};

static void init_logc(int ms = 250) {
    __functree_call__(init_logc);
    #if __GMENG_DISABLE_LOG__ == true
        return;
    #endif
    if (!Gmeng::global.restarted_instance) {
        /// initialize the log
        __gmeng_write_log__("gmeng.log", "-- cleared previous log --\n", false);
        gm_log("Gmeng "+Gmeng::version+" (build " + GMENG_BUILD_NO + ").\n\nGmeng is built and maintained by catriverr.\n\nDocumentation available in https://gmeng.org.\nGmeng is an open source project. https://gmeng.org/git.\nPlease report bugs or unexpected behaviour at https://gmeng.org/report.\n\nGmeng: Go-To Console Game Engine.\n\nSPAWN(1) = v_success / at " + get_curtime() + "/" + get_curdate() + "\ncontroller_t of termui/_udisplay_of(GMENG, window) handed over to: controller_t(gmeng::threads::get(0))\n");
        gm_log("startup information:\n\n----------------------------------\nExecutable Name: " + Gmeng::global.executable + "\nCurrent Working Directory: " + Gmeng::global.pwd + "\nCurrent User: " + Gmeng::global.user + "\n----------------------------------\n" +
                ("Global Variables\n\t- restarted_instance: " + std::string(Gmeng::global.restarted_instance ? "true" : "false" ) + "\n\t- devmode: " + boolstr(Gmeng::global.dev_mode) + "\n\t- debugger: " + boolstr(Gmeng::global.debugger) + "\n\t- silenced: " + boolstr(Gmeng::global.shush) + "\n\t- dont_hold_back: " + boolstr(Gmeng::global.dont_hold_back) + "\n----------------------------------\n"));
    } else {
        /// print restart message
        gm_log("-- RESTARTED --\n");
    };

    if (!Gmeng::global.shush) /*Gmeng::create_thread([&]() {
            __functree_call__(_glog_thread_create);
            for ( ;; ) {
            if (!Gmeng::global.dev_console) continue;
            if (Gmeng::logstream.str().length() > Gmeng::logc.v_drawpoints.size()) {
               Gmeng::completelog << Gmeng::logstream.str();
               Gmeng::logstream.str(""); /// flush sstream
               _uflush_display(Gmeng::logc, 5);
               gm_log("t_display *job_flush -> flushed display at gm:thread" + _uget_thread() + " (detached from gm:thread0 / generated from gm:thread0) ; display memory address: " + _uconv_1ihx(_uget_addr(&Gmeng::logc)));
            };
            _udraw_display(Gmeng::logc);
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }
        /// close log file
    })*/;
};

static void set_terminal_title(const std::string& title) {
    // \033]0; starts the title sequence
    // \007 (BEL character) ends the sequence
    std::cout << "\033]0;" << title << "\007" << std::flush;
};

/// Gets the current working directory
static std::string get_cwd() {
    char buffer[PATH_MAX];

#ifdef _WIN32
    if (GetCurrentDirectoryA(PATH_MAX, buffer)) {
        return std::string(buffer);
    }
#else
    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        return std::string(buffer);
    }
#endif
    return std::string();
}


/// Patches gmeng's 'global' variables required by the engine,
/// used for stuff like parsing command-line arguments.
/// Generally, this method should be run for almost
/// every program depending on gmeng.
///
/// While edge cases are existent, if you do not extensively know
/// what this method does, it is advised to run it in your
/// `main()` function.
#include "functree.h"
#include "tracefunc.h"


/// returns the username of the current user
static std::string get_username() {
    const char* username = nullptr;
#ifdef _WIN32
    username = getenv("USERNAME");
#else
    username = getenv("USER");
#endif
    if (username) return std::string(username);
    else return std::string();
};

inline void apply_argument(const std::string& argument, bool& IS_USING_GMENG_APPLETERMINAL_THEMED) {
        /// For the game to launch using the correct apple terminal profile.
        if (argument == "--profile" ) { IS_USING_GMENG_APPLETERMINAL_THEMED = true; };
        if (argument == "--no-window-control") { Gmeng::global.window_control = false; };
        if ( argument == "--no-advanced-keyboard" ) { Gmeng::global.no_advanced_keyboard = true; };
        if (argument == "--debug-args") {
            IS_USING_GMENG_APPLETERMINAL_THEMED = true;
            Gmeng::global.window_control = false;
            Gmeng::global.no_advanced_keyboard = true;
            Gmeng::global.bypass_splashscreen = true;
        };
        if (argument == "--no-splashscreen") {
            Gmeng::global.bypass_splashscreen = true;
        };

        if ( argument == "-help" || argument == "/help" || argument == "--help" || argument == "/?" || argument == "-?" ) {
            __functree_call__(__gmeng__help__menu__);
            struct winsize size;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
            int times = size.ws_col-11;

            gm_log("command-line argument requested help menu");
            SAY("~Br~\x0F~h~\x0F~y~GMENG " + (Gmeng::version) + "~n~ | " + Gmeng::colors[6] + "Terminal-Based 2D Game Engine~n~ | Help Menu\n");
            SAY("~_~~st~" + repeatString("-", times+11) + "~n~\n");
            SAY("~h~~r~Gmeng~n~ is a standalone terminal-based game engine, utilizing ~y~pthread~n~ and the ~b~C++ Standard library~n~.\n");
            SAY("~_~Currently, Gmeng is only available to MacOS and Linux users while on its active development phase.\n");
            SAY("~_~You may contribute to porting ~r~Gmeng~_~ to windows: ~b~https://github.com/catriverr/gmeng-sdk\n");
            SAY("~b~~i~PARAMETERS ~st~" + repeatString("-", times) + "~n~\n");
            SAY("\t    ~y~gmeng ~p~-devc \t\t~_~enables developer console\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-no-devc\t\t~_~disables developer console\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-devmode\t\t~_~enables developer diagnostics\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~--profile\t\t~_~asumes terminal is correct\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-log-to-cout\t\t~_~streams logs to stout\t\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-no-functree\t\t~_~disables the gmeng functree\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-debugger -debug\t~_~enables extensive debug logs\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-shut-the-fuck-up\t~_~silences all logging\t\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-functree-extensive\t~_~makes functree logs extensive\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-tell-me-everything\t~_~enables all logging methods\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("~b~" + repeatString(" ", 11) + "~st~" + repeatString("-", times) + "~n~\n");
            exit(0);
        };
        if ( argument == "-devc" ) Gmeng::global.dev_console = true;
        if ( argument == "-no-devc" || argument == "-shut-the-fuck-up" ) Gmeng::global.dev_console = false;
        if ( argument == "-shut-the-fuck-up" ) Gmeng::global.shush = true;
        if ( argument == "-debugger" || argument == "-debug" || argument == "--debugger" ) Gmeng::global.debugger = true;
        if ( argument == "-log-to-cout" || argument == "-lc" ) Gmeng::global.log_stout = true;
        if ( argument == "-devmode" ) Gmeng::global.dev_mode = true;
        if ( argument == "-tell-me-everything" ) Gmeng::global.shush = false, Gmeng::global.dev_mode = true, Gmeng::global.dev_console = true, Gmeng::global.dont_hold_back = true, Gmeng::global.debugger = true;
        if ( argument == "-no-functree" ) { Gmeng::functree_enabled = false; SAY("~b~\x0F~y~WARN! ~_~it is not recommended to disable the Gmeng Functree.\n"); };
        if ( argument == "-functree-extensive" ) { Gmeng::functree_extensive = true; };
        if ( argument == "-weird" ) Gmeng::global.weird_ass = true;
        if ( argument == "-ignore-assert" ) Gmeng::global.ignore_assert = true;
};

/// returns true if the argument is usable, false if it
/// is an alias and has been consumed instead.
inline bool gmeng_check_argument(std::string& argument, bool& profile_c) {
    static std::map<char, std::string> cargument_list = {
        { 'p', "--profile" },
        { 's', "--no-splashscreen" },
        { 'k', "--no-advanced-keyboard" },
        { 'w', "--no-window-control" },
    };

    if (!argument.starts_with('-') || argument.length() < 2) return true;

    /// starts with '--', can't be an alias.
    if (argument.starts_with("--")) return true;

    bool is_usable = true;

    for ( auto& c : argument.substr(1) ) {
        if (cargument_list.contains(c)) {
            is_usable = false;

            apply_argument(cargument_list[c], profile_c);
        };
    };

    return is_usable;
};


static void patch_argv_global(int argc, char* argv[], TRACEFUNC) {
    __annot__(patch_argv_global, "patches the Gmeng::global variable with the command-line arguments.");
    __functree_call__(patch_argv_global);

    #if _WIN32
        print_windows_error_message();
        return;
    #endif

        if ( std::string( func_caller ) != "main" ) {
            gmeng_show_warning( "init_from_main" );
        };

        if ( argc == 0 ) {
            gmeng_show_warning("no_arguments");
        };

        auto* colorterm = std::getenv("COLORTERM") ? std::getenv("COLORTERM") : "";

        if ( std::string(colorterm) != "truecolor" ) {
            gmeng_show_warning("no_truecolor");
        };

    Gmeng::global.pwd = get_cwd();
    Gmeng::global.user = get_username();

#if _WIN32 == false
    Gmeng::global.prog_argc = argc;
    Gmeng::global.prog_argv = argv;

    Gmeng::global.executable = (std::string(argv[0]).substr(std::string(argv[0]).rfind("/") + 1)).c_str();
    Gmeng::global.raw_executable_name = argv[0];

    char *restart_flag = "--GMENG_INTERNAL_RESTARTED_INSTANCE";

    kitty_check_focus(true);

    std::vector<char *> v_argv;
    for (int i = 0; i < argc; i++) {
        v_argv.push_back(argv[i]);
        Gmeng::global.raw_arguments += argv[i] + std::string(" ");
    };

    int has_restart_flag = std::count(v_argv.begin(), v_argv.end(), restart_flag);
    bool restarted_instance = false;
    if (has_restart_flag > 0) restarted_instance = true;

    Gmeng::global.restarted_instance = restarted_instance;

    if (!restarted_instance) Gmeng::outfile.open("gmeng.log");
    else Gmeng::outfile.open("gmeng.log", std::ios::app);

    bool IS_USING_GMENG_APPLETERMINAL_THEMED = false;

    for (int i = 0; i < argc; i++) {
        char *v_arg = argv[i];
        std::string argument (v_arg);

        if (gmeng_check_argument( argument, IS_USING_GMENG_APPLETERMINAL_THEMED ))
            apply_argument(argument, IS_USING_GMENG_APPLETERMINAL_THEMED);
    };

#ifdef __APPLE__
    /*if ( std::string( getenv("TERM") ) != "xterm-kitty" ) {
        /// Launch Gmeng MacOS setup (setup will exit immediately if the game directory was already set-up)
        bool setup = gmeng_macos_terminal_setup(argc, argv);
        if (!setup) {
            INF("[apple_setup] error (0) nontrue return value from gmeng_macos_terminal_setup. cannot proceed.\n");
            exit(1);
        }

        /// Force MacOS to use Apple Terminal

        std::string exc_path = get_executable_directory();
        int force_profile = enforce_macos_terminal_profile(exc_path + "/assets/gmeng.terminal", IS_USING_GMENG_APPLETERMINAL_THEMED);
        /// return value (3) means the game has launched with the required gmeng profile.
        if (force_profile == 3) exit(0);
        /// return value (0) means the game is already using the required gmeng profile.
        /// otherwise, error.
        if (force_profile != 0) {
            INF("[apple_setup] error (" + v_str(force_profile) +") nonzero return value from enforce_macos_terminal_profile. cannot proceed.\n");
            exit(1);
        };
    }*/
#if GMENG_SDL != true
    /// Since 13.4.0: force kitty terminal.
    if (!IS_USING_GMENG_APPLETERMINAL_THEMED) {
        std::string exc_path = get_executable_directory();
        std::system(("kitty --start-as=fullscreen --config \"" + exc_path + "/assets/gmeng_kitty.conf\" " + Gmeng::global.raw_arguments + " --profile").c_str());
        exit(0);
    } else kitty_set_font_size(7);
#endif

#endif
    set_terminal_title("GMENG " + Gmeng::version + " | build " + GMENG_BUILD_NO);
    init_logc();
    gm_log("hello");
#endif
};




/// Restarts the current executable/program.
/// Requires `patch_argv_global()` to have been called.
/// This method runs `execvp` from the `unistd.h` library
/// to replace the current program image in the memory
/// with a new instance of the current program, in `argv[0]`.
static int restart_program() {
    __annot__(restart_program, "Restarts the current executable.");
    __functree_call__(restart_program);

    std::string arg0 = Gmeng::global.raw_executable_name;

    if (arg0.empty()) {
        gm_log("cannot restart program: `Gmeng::global.raw_executable_name` is empty. `patch_argv_global()` must be called with the correct arguments before this function can be used");
        return 1;
    };

    // add `restarted-instance` flag to argv if it does not exist
    std::string restart_flag = "--GMENG_INTERNAL_RESTARTED_INSTANCE";
    bool flag_exists = false;

    for (int i = 0; i < Gmeng::global.prog_argc; ++i) {
        if (std::strcmp(Gmeng::global.prog_argv[i], restart_flag.c_str()) == 0) {
            flag_exists = true;
            break;
        }
    };


    std::vector<std::string> new_argv;

    if (!flag_exists) new_argv.push_back(restart_flag);

    for (int i = 0; i < Gmeng::global.prog_argc; ++i) {
        new_argv.push_back(Gmeng::global.prog_argv[i]);
    };

    std::vector<char*> prog_argv;
    for ( auto v : new_argv )
        prog_argv.push_back( const_cast<char*>( v.c_str() ) );


    gm_log("restarting program...");

    if (execvp(arg0.c_str(), prog_argv.data()) == -1) {
        gm_log("cannot restart program: `execvp()` returned non-zero value (-1): " + std::to_string(errno));
        return 2;
    };

    return 0;
};
