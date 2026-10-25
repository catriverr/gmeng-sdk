#pragma once
#include <cerrno>
#include <cmath>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <dlfcn.h>
#include <execinfo.h>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/signal.h>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include <noble/lib/scripts/arch.cc> // NOBLE: No Bad Language Esoterics (https://github.com/catriverr/noble)
                                     // used for .dylib prebuilt C++ script handling.
#include "../gmeng.h"
#include "../utils/gridmap.cc"
#include "imgui.h"

#ifdef GMENG_SDL
#include "SDL_keycode.h"
#endif

#ifdef _WIN32
#include <windows.h> // at some point it will be important
#include <conio.h>  // For _getch() on Windows
#else
#include <termios.h>
#include <unistd.h>
#endif

/// Kitty Terminal Graphics Protocol Utility
#include "../utils/graphics.h"
/// Kitty Terminal Keyboard Protocol Utility
#include "../utils/keyboard.h"


#if GMENG_SDL_INIT == true
    // ignore coc-nvim errors, this compiles
    #include "../utils/window.cpp"
#endif

#include "../types/camera.h"



void setRawMode(bool enable) {
#ifndef _WIN32
    static struct termios oldt, newt;
    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);  // Get current terminal settings
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);  // Disable canonical mode and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);  // Apply new settings immediately
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  // Restore old settings
    }
#endif
}



/// text input in the console with output feedback
std::string lineinput(bool secret = false) {
    std::string input;

    if (secret) {
        #ifdef _WIN32
        // windows support even though
        // we don't even slightly have
        // compatibility with it
        char ch;
        while (true) {
            ch = _getch();  // Get character without echoing on Windows
            if (ch == '\r') {  // Enter key on Windows
                std::cout << std::endl;
break;
            } else if (ch == '\b') {  // Backspace
                if (!input.empty()) {
                    input.pop_back();
                    std::cout << "\b \b";  // Remove the last asterisk
                }
            } else {
                input.push_back(ch);
                std::cout << '*' << std::flush;  // Display asterisk instead of character
            }
        }
        #else
        setRawMode(true);  // Enable raw mode on Unix-like systems
        char ch;
        while (true) {
            ch = getchar();
            if (ch == '\n') {  // Enter key
                std::cout << std::endl;
                break;
            } else if (ch == 127 || ch == '\b') {  // Backspace (127 or '\b')
                if (!input.empty()) {
                    input.pop_back();
                    std::cout << "\b \b" << std::flush;  // Remove the last asterisk
                }
            } else {
                input.push_back(ch);
                std::cout << '*' << std::flush;  // Display asterisk immediately
            }
        }
        setRawMode(false);  // Restore original terminal settings
        #endif
    } else {
        std::getline(std::cin, input);
    }

    return input;
};

// generates a trajectory between two coordinates (x1,y1), (x2, y2)
// using the most efficient path towards point 2
//
// does NOT factor in collision. The trace WILL contain unit coordinates
// that are not collidable if they are part of the most efficient trajectory.
std::vector<Objects::coord> g_trace_trajectory(int x1, int y1, int x2, int y2) {
    __functree_call__(g_trace_trajectory);

    std::vector<Objects::coord> coordinates;

    int dx = abs(x2 - x1); int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1; int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (x1 != x2 || y1 != y2) {
        Objects::coord point;
        point.x = x1; point.y = y1;
        coordinates.push_back(point);
        int err2 = 2 * err;
        if (err2 > -dy) { err -= dy; x1 += sx; }
        if (err2 < dx) { err += dx; y1 += sy; }
    };

    // Include the final point (x2, y2)
    Objects::coord point;
    point.x = x2; point.y = y2;
    coordinates.push_back(point);

    return coordinates;
};

#include "../types/eventloop.h"

namespace Gmeng {

    /// v10.1.0 - Unit object that has a position.
    struct Positioned_Unit {
        Unit unit;
        Objects::coord position;
    };
    /// v10.1.0
    struct Overlay {
        std::vector<Positioned_Unit> units;
    };


    inline color32_t apply_lighting(const LightingState& state) {
        auto clamp = [](int v) { return std::clamp(v, 0, 255); };
        DEBUGGER gm_log("random_ass_color("$( uint32_from_color32(state.surface_color) )"): "$(state.r_factor)", "$(state.g_factor)","$(state.b_factor)" modf: "$(state.modulation_factor)".");
        return {
            static_cast<uint8_t>(clamp(static_cast<int>(state.surface_color.r * state.r_factor * state.modulation_factor))),
            static_cast<uint8_t>(clamp(static_cast<int>(state.surface_color.g * state.g_factor * state.modulation_factor))),
            static_cast<uint8_t>(clamp(static_cast<int>(state.surface_color.b * state.b_factor * state.modulation_factor)))
        };
    };


	template<std::size_t _w, std::size_t _h>
	inline Camera<_w, _h> UseRenderer(Gmeng::G_Renderer<_w, _h> __r) {
        __functree_call__(Gmeng::UseRenderer);
		Gmeng::Camera<_w, _h> wrldmp;
		wrldmp.w = __r.width; wrldmp.h = __r.height;
		wrldmp.constructor(__r.display.unitmap);
		for (int i = 0; i < __r.height; i++) {
			for (int j = 0; j < __r.width; j++) {
				int current_unit_id = (i * __r.width) + j;
				Gmeng::Unit current_unit = __r.display.unitmap[current_unit_id];
                std::string final = wrldmp.draw_unit(current_unit);
				wrldmp.raw_unit_map[current_unit_id] = final;
			};
		};
		return wrldmp;
	};
};

#include "../utils/network.cpp"

/// since we only support 1 server per game instance,
/// we set it as a namespace to the Gmeng namespace.
namespace Gmeng::RemoteServer {
    static bool state = false;
    static gmserver_t server(7385);
    /// password
    static std::string aplpass = v_str(g_mkid());
    static std::thread* thread = nullptr;
};

/// creates the RemoteServer thread
static void _gremote_server_apl(bool state, std::string aplpass) {
    __functree_call__(_gremote_server_apl);
    Gmeng::RemoteServer::state = state;
    Gmeng::RemoteServer::aplpass = aplpass;

    auto thread_t = Gmeng::create_thread([&]() {
        Gmeng::RemoteServer::server.run();
        Gmeng::RemoteServer::server.create_path(path_type_t::POST, "/stop", [&](request req, response res) {
            bool body_matches = req.body == Gmeng::RemoteServer::aplpass;
            if (body_matches) Gmeng::RemoteServer::server.stop(), exit(0); /// exit application
            else res.status_code = 401, res.body = "e_unauthorized";
        });
    });

    Gmeng::RemoteServer::thread = &thread_t;
};



/// GMENG EVENTLOOP IMPLEMENTATION

#include "renderer.cpp"

#include <unistd.h>
#ifndef _WIN32
#include <termios.h>
#endif

#ifndef _WIN32
#include "../utils/terminal.h"
#endif


// Game log
static std::vector<std::string>  PROP_LOGSTREAM = {
    "Gmeng: Terminal Game Engine - \033[34mhttps://gmeng.org/git",
};

static std::vector<std::string>* GAME_LOGSTREAM = &PROP_LOGSTREAM;
static std::stringstream GAME_LOGSTREAM_STR;

#include "../types/event.h"

namespace Gmeng {
    /// Default Event Hooks & handlers of Gmeng.
    /// Individual hooks defined in this dictionary
    /// can be disabled by external event hooks via
    /// (EventInfo*) info->prevent_default(true).
    std::unordered_map<Event, vector<EventHook>> DEFAULT_EVENT_HOOKS = {
        { FIXED_UPDATE,
          {
            EventHook {
                .events  = { FIXED_UPDATE },
                .handler = [](Level* level, EventInfo* info) {
                    for ( auto& entity : level->entities )
                        if ( entity != nullptr && entity->active ) {
#ifndef GMENG_NO_SOL
                            for ( auto& routine : entity->routines ) {
                                routine.progress += 1.0f / level->display.fixed_update_fps;
                                if ( routine.progress >= routine.period ) {
                                    routine.progress -= routine.period;

                                    if (!routine.init) {
                                        routine.init = true;
                                        Scripts::lua[routine.entry_point] = sol::lua_nil;
                                        Scripts::lua.script( routine.lua_source );
                                    } else if (routine.executable) Scripts::lua[routine.entry_point]( entity );
                                };
                            };
#endif
                            entity->_animate( NULL, level->display.fixed_update_fps );
                        }
                }
            }
          }
        },
    };
};
/// Event Hook System
/// alternatives
#define MOUSE_REST_4_CHECKER(x) x == 4 ? Gmeng::MOUSE_CLICK_LEFT_START : (\
                                       x == 6 ? Gmeng::MOUSE_CLICK_RIGHT_START : (\
                                           x == 39 ? Gmeng::MOUSE_MOVE : Gmeng::UNKNOWN ))
#define MOUSE_REST_3_CHECKER(x) x == 71 ? Gmeng::MOUSE_SCROLL_DOWN : (\
                                        x == 70 ? Gmeng::MOUSE_SCROLL_UP : MOUSE_REST_4_CHECKER(x) )
#define MOUSE_REST_2_CHECKER(x) x == 0 ? Gmeng::MOUSE_CLICK_LEFT_START : (\
                                        x == 1 ? Gmeng::MOUSE_CLICK_MIDDLE_START : (\
                                                x == 2 ? Gmeng::MOUSE_CLICK_RIGHT_START : (\
                                                        x == 3 ? Gmeng::MOUSE_CLICK_END_ANY : (\
                                                                x == 35 ? Gmeng::MOUSE_MOVE : MOUSE_REST_3_CHECKER(x)) )))
#define MOUSE_REST_1_CHECKER(x) x == 65 ? Gmeng::MOUSE_SCROLL_DOWN : MOUSE_REST_2_CHECKER(x)
/// Returns the Mouse Event from a sscanf(  ) for 1006-rawmode instanced mouse input.
/// scroll up, scroll down, mouse move, left click, right click, etc.
#define SELECT_MOUSE_EVENT(x) x == 64 ? Gmeng::MOUSE_SCROLL_UP : ( x == 32 ? Gmeng::MOUSE_MOVE : MOUSE_REST_1_CHECKER(x))
                                                                // TODO> x == 32 is actually mouse left click DRAG.
                                                                //       we can add that event in the future.

static void mclickevent_set_click_type(Gmeng::Event& event, char eventType) {
    if (eventType == 'm') {
        switch(event) {
            case Gmeng::MOUSE_CLICK_LEFT_START:
                event = Gmeng::MOUSE_CLICK_LEFT_END;
                break;
            case Gmeng::MOUSE_CLICK_RIGHT_START:
                event = Gmeng::MOUSE_CLICK_RIGHT_END;
                break;
            case Gmeng::MOUSE_CLICK_MIDDLE_START:
                event = Gmeng::MOUSE_CLICK_RIGHT_END;
                break;
            default:
                break;
        };
    };
};

#define DECIDE_IF_MOUSEUP_OR_MOUSEDOWN(event, eventType) mclickevent_set_click_type(event, eventType)

/// (Gmeng) returns the last n lines of a vector of strings
std::deque<std::string> get_last_n_lines(std::vector<std::string>& ss, int n) {
    std::deque<std::string> lines;

    int i = 0;
    while (i < ss.size()) {
        lines.push_back(ss.at(i));
        if (lines.size() > n) {
            lines.pop_front();
        };
        i++;
    };

    while (lines.size() < n) lines.push_back("");

    return lines;
};


#ifndef GMENG_COMPILING_SCRIPT
/// (Gmeng) emit a log string to the engine's developer console
/// unless debug mode, developer mode or dont_hold_back is enabled,
/// this will not write to the gmeng.log logfile.
///
/// This implementation is for internal logging (files included by
/// the source code of the main program during compile time).
#define GAME_LOG(str)                                              \
    do {                                                           \
        auto splitEntries = g_splitStr(str, "\n");                 \
        for (const auto& entry : splitEntries) {                   \
            GAME_LOGSTREAM->push_back(entry);                      \
            GAME_LOGSTREAM_STR << entry << '\n';                   \
        }                                                          \
    } while (0)

#else
/// (Gmeng) emit a log string to the engine's developer console
/// unless debug mode, developer mode or dont_hold_back is enabled,
/// this will not write to the gmeng.log logfile.
///
/// This implementation is for externally attached programs (files
/// not included during compile time of the source program) such as
/// NOBLE scripts and plugins, liblmf attachments and dylibs.
#define GAME_LOG(str)                                                                                                                   \
    do {                                                                                                                                \
        auto splitEntries = g_splitStr(str, "\n");                                                                                      \
        for (const auto& entry : splitEntries) {                                                                                        \
            ev->logstream->push_back("[" + get_filename(std::string(__FILE__)) + ":" + std::to_string(__LINE__) + "] " + entry);        \
            *ev->logstream_str << "[" + get_filename(std::string(__FILE__)) + ":" + std::to_string(__LINE__) + "] " << entry << '\n';    \
        }                                                                                                                               \
    } while (0)

#endif

#ifndef GMENG_NO_SOL
#ifndef GMENG_COMPILING_SCRIPT
/// Defines the GAME_LOG macro binding to lua-script.
static int __gmeng_define_lua_gmeng_log_method = ([]() -> int {
    Gmeng::Scripts::lua.set_function("GAME_LOG", [&](const std::string& message) {
        GAME_LOG( message );
    });
    return 0;

})();
#endif
#endif

std::deque<std::string> gmeng_log_get_last_lines(int n = 5) {
    return get_last_n_lines(*GAME_LOGSTREAM, n);
};

static std::string dev_console_input = "";
static bool already_writing = false;
static Gmeng::EventInfo* dev_next = nullptr;
static bool dev_console_open = false;
static bool dev_console_first_open = true;

static bool crash_protector = false;
static int GMENG_DEV_CONSOLE_WIDTH = 80;
static int GMENG_DEV_CONSOLE_HEIGHT = 30;



/// variables in-game usable in the developer console/
/// these variables are only integers (currently)
///
/// variables can point to other variables
/// using the '*' token at the start of the variable name
/// and the index of the variable it is pointing to/
///
/// example:
///
/// { "errorlevel", 0 },
/// { "hello_world", 0 },
/// { "*hw", 1 }, // but the value of 'hw' would point to 'hello_world' as hello world is variables[1].
/// hence the hw's value of 1.
static vector< std::tuple<string, int> > variables = {
    { "errorlevel", 0 },
    { "*?", 0 }, // ptr to variables[0] (errorlevel)
    { "*!", 0 }, // ptr to variables[0] (errorlevel)
};


#ifndef GMENG_COMPILING_SCRIPT
// EXTERNS FOR NOBLE SCRIPTS
// all NOBLE scripts must use these methods for their functionality.
// scripts can not be executed from different entrypoint functions,
// but can have multiple of the gmeng-provided entries for different
// types of execution such as a command based initialization entry
// and a periodic control entry.


// NOBLE SCRIPT HELPER FOR GMENG
//
// Command-based, executed once
extern "C" int gmeng_script_command( Gmeng::EventLoop* );
// NOBLE SCRIPT HELPER FOR GMENG
//
// Periodic, executed with event UPDATE
extern "C" int gmeng_script_periodic( Gmeng::EventLoop* );

#endif


static bool gmeng_console_state_change_modifier = false;
static bool state_changed = false;

#include "../utils/console.h"

#include "refresh.cc"

/// Runs the developer console for Gmeng.
/// This utility is for Terminal-mode only.
/// It should not be used with Gmeng SDL.
///
/// Kitty Terminal Graphics Protocol will be used if available.
void gmeng_dev_console(Gmeng::EventLoop* ev, Gmeng::EventInfo* info) {
    if (!dev_console_open) return;

    /// for handling the menu
    /// since the screen can change in the time the console is open, we call the method
    /// afterwards to draw the console with all of its previous calls with dev_next.

    if (already_writing) {
        dev_next = info; return;
    };

    if (dev_next != nullptr) {
        auto next = *dev_next;
        dev_next = nullptr;

        gmeng_dev_console(ev, &next);
    };

    dev_console_open = true;
    already_writing = true;

    /// todo: make this non-blocking so imgui doesnt crash for some whorey reason
    if ( ev->level->display.camera.has_modifier("imgui") ) {
        gmeng_kgp_console(ev, info);
        return;
    };

    using namespace Gmeng;
    using namespace Gmeng::Renderer;
    using std::string, std::vector, std::deque, std::cout;

    EventInfo dd_info = *info;
    bool run = false;
    std::string cmd = "";


    if (info->EVENT == KEYPRESS || info->EVENT == KEYREPEAT) {
        switch (info->KEYPRESS_CODE) {
            case 27: /// esc or shift+tab
                break;
            case gk_ENTER: /// enter
                run = true;
                cmd = dev_console_input;
                dev_console_input = "";
                break;
            case 127: /// backspace
                if (dev_console_input.length() > 0) dev_console_input.pop_back();
                break;
            case '~':
                gmeng_console_state_change_modifier = true;
                break;
            default:
                if (dev_console_input.length() >= GMENG_DEV_CONSOLE_WIDTH-7) return;
                dev_console_input += info->KEYPRESS_CODE;
                break;
        };
    };


    int last_result = -1;

    if (run == true) run = false, last_result = gmeng_run_dev_command(ev, cmd);
    if (ev->cancelled) return;

    int CUR_COLOR = last_result == -1 ? color_t::YELLOW : ( last_result == 0 ? GREEN : RED );

    Display* display = &ev->level->display;
    drawpoint delta_xy = { 0, 0 };
    Camera<0, 0>* camera = &ev->level->display.camera;

    color_t CONSOLE_OUTLINE_COLOR = color_t::GREEN;


    #define OUTLINE "┌" + repeatString("─",   GMENG_DEV_CONSOLE_WIDTH-2) + "┐"
    #define OUTLINE_2 "├" + repeatString("─", GMENG_DEV_CONSOLE_WIDTH-2) + "┤"
    #define OUTLINE_3 "└" + repeatString("─", GMENG_DEV_CONSOLE_WIDTH-2) + "┘"

    deque<string> log_last = gmeng_log_get_last_lines(GMENG_DEV_CONSOLE_HEIGHT);


    log_last.push_front(OUTLINE_2);
    log_last.push_front("│ gmeng " + version + "-" + GMENG_BUILD_NO + " debug " + "console" );
    while(log_last.at(0).length() < GMENG_DEV_CONSOLE_WIDTH-1) log_last.at(0) += " ";
    log_last.at(0) += "  │";

    camera->set_curXY( -1, delta_xy.x+2 );
    cout << colors[CONSOLE_OUTLINE_COLOR] << OUTLINE << resetcolor;
    int i = 0;

    for (int j = 0; i < log_last.size(); i++) {
        string data = log_last.at(i);

        camera->set_curXY(i, delta_xy.x+2);

        if (i != 0 && i != 1 &&
            data.length() > GMENG_DEV_CONSOLE_WIDTH-4) data = data.substr(0, GMENG_DEV_CONSOLE_WIDTH-7) + "...";

        if (i == 0 || i == 1) { cout << colors[CONSOLE_OUTLINE_COLOR] << data << resetcolor; continue; };

        while (data.length() < GMENG_DEV_CONSOLE_WIDTH-4) data += " ";

        cout << colors[CONSOLE_OUTLINE_COLOR] << "│ " << resetcolor << data << colors[CONSOLE_OUTLINE_COLOR] << " │" << resetcolor;
    };

    /// unused
    int d = 0;
    while (i < GMENG_DEV_CONSOLE_HEIGHT+2) {
        camera->set_curXY(i, delta_xy.x+2);
        cout << colors[CONSOLE_OUTLINE_COLOR] << "│" << resetcolor << repeatString(" ", GMENG_DEV_CONSOLE_WIDTH-2) << colors[CONSOLE_OUTLINE_COLOR] << "│" << resetcolor;
        i++;
    };

    camera->set_curXY(i+d, delta_xy.x+2);
    cout << colors[CONSOLE_OUTLINE_COLOR] << OUTLINE_2 << resetcolor;
    camera->set_curXY(1+i+d, delta_xy.x+2);

    std::string commandline = dev_console_input + Gmeng::c_unit;
    while(commandline.length() < GMENG_DEV_CONSOLE_WIDTH-4) commandline += " ";

    cout << colors[CONSOLE_OUTLINE_COLOR] << "│ " << boldcolor << colors[CUR_COLOR] << "➜ " << resetcolor << commandline << colors[CONSOLE_OUTLINE_COLOR] << " │" << resetcolor;

    camera->set_curXY(2+i+d, delta_xy.x+2);
    cout << colors[CONSOLE_OUTLINE_COLOR] << OUTLINE_3 << resetcolor;

    already_writing = false;
};

/// Checks and displays warnings for various terminal and shell programs.
/// This function is called by `do_event_loop` in the beginning of a game
/// event loop. It is not required to implicitly call it.
void check_terminal_and_shell( Gmeng::EventLoop* ev ) {
    char* term_prog = getenv("TERM_PROGRAM");
    char* term = getenv("TERM");

    if ( term != nullptr && ( std::string(term) != "xterm-kitty" ) ) {
        gmeng_show_warning("bad_terminal");
    };

    if ( ev->level->display.camera.modifiers.get_value("lighting") == 1 &&
         term_prog != nullptr && ( std::string(term_prog) == "tmux" ))
            gmeng_show_warning("lighting_with_tmux");
};

#ifndef _WIN32
#if GMENG_SDL != true

static Objects::coord font_size = { 0, 0 };

void gmeng_handle_sigint(int) {
    gm_log("SIGINT received, exiting...");
    Gmeng::main_event_loop->cancelled = true;
};

void gmeng_handle_sigwinch(int) {
    gm_log("SIGWINCH received, window resized, reapplying...");
    font_size = get_font_size_values();
};

void gmeng_write_stack_trace() {
    void *array[64];
    int size;

    size = backtrace(array, 64);

    for ( int i = 0; i < size; ++i ) {
        Dl_info info;

        if ( dladdr(array[i], &info) ) {
            const char* obj_name = info.dli_fname ? info.dli_fname : "obj_unknwn";

            const char* last_slash = std::strrchr(obj_name, '/');
            if ( last_slash )
                obj_name = last_slash + 1;

            const char* final_func_name = "[hidden symbol]";
            char* demangled = nullptr;

            if ( info.dli_sname ) {
                int status = 0;
                demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);

                if ( status == 0 && demangled ) {
                    final_func_name = demangled;
                } else final_func_name = info.dli_sname;
            };
            const char* std__1 = "std::__1";
            const char* gmeng = "Gmeng";
            if ( !contains(obj_name, "libsystem_") && !contains(final_func_name, "std::__1") )
                std::cerr << "gmeng:" << i << " " << obj_name << "\t\t " << array[i] << "\t" << final_func_name << "\n";
            if ( demangled ) free(demangled);
        } else std::cerr << "gmeng:" << i << "\t[hidden symbol]\t" << array[i] << "\t[hidden symbol]\n";
    };
};


void gmeng_handle_sigsegv(int signal) {
    /// we can't use std::cout or gm_log because we don't know what caused the segmentation fault,
    /// we have to assume that almost all of the preexisting memory objects are corrupted.
    const char msg[] = "\n[gmeng:signal_handler] caught SIGSEGV: (segmentation fault), quitting\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);

    /// Deinitialize the keyboard so if the program uses the kitty keyboard protocol
    /// they can actually type on their terminal properly.
    gmeng_keyboard_deinit();
    /// DEinitialize the mouse so mouse events don't write to the terminal causing
    /// it to write garbage to the screen.
    Gmeng::TerminalUtil::disable_mouse_tracking();

    /// writes the stack trace to the screen.
    gmeng_write_stack_trace();

    /// Unregister signal handler and re-raise to produce core dump / exit properly
    std::signal(signal, SIG_DFL);
    std::raise(signal);
};

void gmeng_handle_buserror(int signal) {
    const char msg[] = "\n[gmeng:signal_handler] caught SIGBUS: (bus errror), quitting\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);

    /// Deinitialize the keyboard so if the program uses the kitty keyboard protocol
    /// they can actually type on their terminal properly.
    gmeng_keyboard_deinit();
    /// DEinitialize the mouse so mouse events don't write to the terminal causing
    /// it to write garbage to the screen.
    Gmeng::TerminalUtil::disable_mouse_tracking();

    gmeng_write_stack_trace();
    /// Unregister signal handler and re-raise to produce core dump / exit properly
    std::signal(signal, SIG_DFL);
    std::raise(signal);
};

#include "../utils/splashscreen.cc"

#include "../utils/gm_imgui.h"

/// Terminal-Based Event Loop handler. Default-provided by Gmeng.
/// Since 8.0.0, this is the correct way for a Gmeng game to be ran.
/// `do_event_loop` will automatically check the environment in which
/// Gmeng is being run.
///
/// See `gmeng.org/do_event_loop` for more information.
int do_event_loop(Gmeng::EventLoop* ev) {

    ev->defaults = Gmeng::DEFAULT_EVENT_HOOKS;
    ev->logstream = GAME_LOGSTREAM;
    ev->logstream_str = &GAME_LOGSTREAM_STR;

    /// Call to the functree
    __functree_call__(do_event_loop);
    using namespace Gmeng;
    /// Create an event loop controller state. Used
    /// for multiple booleans/settings within the event loop.
    EventLoop_Controller_State state;
    /// Check the terminal and shell. As of version 13.0.0, Gmeng recommends
    /// `kitty` terminal as it is the best at displaying text most efficiently.
    /// However, it will run on almost any terminal. So, if a user wants to
    /// run Gmeng in a different terminal/shell, we only display a simple warning.
    /// Some basic non-breaking requirements are truecolors and window resizing.
    check_terminal_and_shell( ev );

    /// Initialize Gmeng's default terminal font. The font is in `assets/term-font`
    /// and every character is stored as a Gmeng::texture object in the `.gt` format.
    /// By default, most characters are 4x5 here. It also does not support any non-english
    /// character or punctuation except the ones found on the ABC keyboard in macOS Settings.
    /// However, users can use this function to load a valid font into the map and it will
    /// be fully functional with that font. Font sizes are currently set by default but an
    /// implementation of a function like Gmeng::Blob::stretch() may change this.
    gmeng_terminal_font_init();

    /// Initialize Gmeng's implementation for Kitty Terminal Graphics Protocol. This function
    /// will check if the terminal running the application is Kitty, and will use the protocol
    /// if so. The protocol allows for displaying raw framebuffers to the terminal's screen as
    /// RGB pixels, and it is much faster than writing ASCII color code escape characters.
    gmeng_terminal_graphics_init();

    /// Initialize Gmeng's implementation for Kitty Terminal Keyboard Protocol. This function
    /// will check if the terminal running the application is Kitty, and will use the protocol
    /// if so. The protocol allows for receiving keyUp and keyDown events, enabling Gmeng to
    /// track individual key states. This enables having multiple keys being pressed at once
    /// to be parsed indivudally. So, if the user presses W+A at once, the program executes
    /// the functionality for both W and A. This differs from the default terminal stdin
    /// read calls, which is just a raw, dumb input buffer. It is like playing a game
    /// on a text editor, waiting for key repeats and single-key text input.
    bool kb = gmeng_keyboard_init();

    if ( !kb ) GAME_LOG("keyboard support is not enabled");

    /// Only one instance of an EventLoop can be valid. A game cannot run two different
    /// EventLoops, and neither can a single program. If a single program needs to run
    /// two different EventLoops at once, they can do so by implementing the other loop
    /// in a NOBLE script or launch a subprocess/child containing another EventLoop handler.
    if (Gmeng::main_event_loop != nullptr) return 1;
    Gmeng::main_event_loop = ev;

    /// Keyboard and Mouse input read buffer. used with std::sscanf() to read their contents.
    /// The buffer is at max something like \033[100;200;300m so sizeof(char)*64 is actually
    /// a bit overkill.
    char buf[64];

    /// If the EventLoop is cancelled before `do_event_loop` even has the chance to start it,
    /// it must have either been called with a loop that was already cancelled, or, more likely,
    /// a loose thread within the user's program is cancelling it prematurely. Since this is a
    /// potential memory leak and is begging to create undefined behaviour, we cancel immediately.
    if (ev->cancelled) return 1;

    ev->level->display.camera.clear_screen();

    /// Show the Gmeng splashscreen to the display if the Kitty Terminal Graphics Protocol is
    /// supported by the terminal. This can be bypassed via Gmeng::global.bypass_splashscreen.
    if ( global.terminal_has_kgp_pixel_support && !global.bypass_splashscreen )
        show_splash_screen( ev->level->display.camera.renderer );

    ev->level->display.camera.clear_screen();

    /// The event loop is in order, call the INIT hook. This is required because variables within
    /// Gmeng that are set by `do_event_loop` up until this point contain some that are user-configured.
    /// For example, `gmeng_terminal_font_init` will initialize the default font, but the user
    /// may have a different font they would like to use with the game instead. Since gmeng HAS to
    /// initialize stuff like its default font (because, even if the user defines a custom font,
    /// it may not contain specific characters, or may inherit them from the default font, the
    /// engine needs to initialize its default font at all times, and the same goes for other
    /// variables' behaviour), the options are that Gmeng keeps checking its variables for
    /// correct initialization every time it uses them (which is both inefficient and bad design)
    /// or initializes itself completely and lets the user tweak its settings after the fact,
    /// which is what's done with the Gmeng::INIT event cast.
    ev->call_event(Gmeng::INIT, Gmeng::INIT_INFO);


    /// SIGINT handler, in almost all cases this is `CTRL+C`.
    /// Gmeng will still force an EXIT event when this signal is received, but we do
    /// internal cleanup here (most importantly disabling mouse reporting) so the
    /// terminal does not output dirty characters to the terminal.
    signal(SIGINT, gmeng_handle_sigint);
    /// SIGSEGV is segmentation fault (engine or the user fucked up big time),
    /// we handle it by running gmeng_keyboard_deinit() so if the program uses
    /// the kitty keyboard protocol they can actually type on their terminal.
    signal(SIGSEGV, gmeng_handle_sigsegv);
    signal(SIGTRAP, gmeng_handle_sigsegv);
    /// SIGBUS is bus error, probably a memory size issue. we handle it and
    /// re-cast the signal so the application exits according to a bus error
    /// as it would have by its default behaviour.
    signal(SIGBUS, gmeng_handle_buserror);
    signal(SIGABRT, gmeng_handle_sigsegv);
    /// SIGWINCH so if the window size changes we can auto adjust the font size
    /// values to match the pixel coordinates of the screen. Only useful on
    /// Kitty but without using the Kitty Graphics Protocol so edge-case.
    signal(SIGWINCH, gmeng_handle_sigwinch);

    int cl = (ev->level->display.camera.has_modifier("kitty_graphics")) ? 75 : std::max(ev->level->display.viewpoint.end.x - ev->level->display.viewpoint.start.x, 10)
                      + (ev->level->display.camera.has_modifier("draw_outline") ? 2 : 0);
    int rw = (ev->level->display.camera.has_modifier("kitty_graphics")) ? 30 : std::max(ev->level->display.viewpoint.end.y - ev->level->display.viewpoint.start.y, 15)/2
                      + (ev->level->display.camera.has_modifier("draw_outline") ? 2 : 0);

    GMENG_DEV_CONSOLE_WIDTH = cl-10;
    GMENG_DEV_CONSOLE_HEIGHT = rw-10;

    ev->level->display.max_width = ev->level->base.width;
    ev->level->display.max_height = ev->level->base.height;

    int vpw =  vp_width( ev->level->display.viewpoint );
    int vph = vp_height( ev->level->display.viewpoint );

    bool do_outline = ev->level->display.camera.has_modifier("draw_outline");


    gm_log("attempting to resize terminal to (these + do_outline) cl="$(vpw)",rw="$(vph)"...");
    if (!ev->level->display.camera.has_modifier("kitty_graphics"))
        resize_terminal(vpw + ( do_outline ? 2 : 0 ), vph/2 + ( do_outline ? 2 : 0 ), true);
    else resize_terminal( 75, 30, true );

    /// Y-coord increment by 1 pixel so we show odd viewpoint heights too.
    if ( (vph/2) % 2 != 0)
        kitty_increment_window_size( 0, 1 );

    bool sgr_support_checker = false;

    /// Check for SGR-Pixel mouse position
    /// reporting support from the terminal
    while (!sgr_support_checker)
        attempt_set_terminal_sgr_support( sgr_support_checker );

    if ( !Gmeng::global.terminal_has_sgr_pixel_support ) {
        /// SGR-Pixel provides accurate mouse reporting.
        /// If the terminal doesn't have it, show a warning
        /// via adding an element to the UI manager (linger 5s)
        std::string w_title = "WARNING";
        std::string w_text1 = "Your terminal does not support SGR-Pixel.";
        std::string w_text2 = "click/move positions will be inaccurate.";

        Objects::coord title_pos = { (int)(vpw - w_title.length())/2, (int)vph/2 };
        Objects::coord text1_pos = { (int)(vpw - w_text1.length())/2, (int)vph/2+2 };
        Objects::coord text2_pos = { (int)(vpw - w_text2.length())/2, (int)vph/2+4 };


        ui_element title_elem(5000, "~y~" + w_title + "~n~", title_pos);
        ui_element text1_elem(5000, w_text1, text1_pos);
        ui_element text2_elem(5000, w_text2, text2_pos);

        ev->level->display.camera.ui_manager.add_element( title_elem );
        ev->level->display.camera.ui_manager.add_element( text1_elem );
        ev->level->display.camera.ui_manager.add_element( text2_elem );
    };

    /// Enable mouse tracking. TerminalUtil will check
    /// and enable SGR-Pixel or SGR-Base mode by default.
    TerminalUtil::enable_mouse_tracking();
    /// Set raw mode to true. This allows read() to capture
    /// input events without the need for ENTER to be pressed.
    TerminalUtil::set_raw_mode(true);

    state.console_open = false;

    /// This is a bad implementation, so, it's actually not used right now.
    std::unordered_map<std::shared_ptr<Gmeng::EntityBase>,
                       std::vector<std::shared_ptr<Gmeng::EntityBase>>> interaction_map;


    /// handler thread for UPDATE and FIXED_UPDATE
    create_thread([&] {
        const float fixed_time_step = 1.0f / ev->level->display.fixed_update_fps;
        float fixed_update_accumulator = 0.0f;

        /// Start a single profile once so the value is not unitialized
        ev->level->display.profiler.start_profile();

        while( !ev->cancelled ) {
            /// set the raw delta time of the frame to the camera's frame_time value
            float& raw_delta_time = ev->level->display.camera.frame_time;
            /// set the frame the of the camera (this converts float to uint32_t)
            /// we still have the float value via profiler->duration.count() though.
            raw_delta_time = ev->level->display.profiler.end_profile();
            /// Start a frame time count with high_resolution_clock for
            /// the next frame. Used to send a correct FIXED_UPDATE event.
            ev->level->display.profiler.start_profile();

            bool imgui_enabled = ev->level->display.camera.modifiers.get_value("imgui") == 1;

            if ( imgui_enabled ) {
                    ImGui_ImplGmeng_NewFrame( ev->level->display.camera.renderer.width,
                                              ev->level->display.camera.renderer.height,
                                              ev->level->display.profiler.duration.count());
                    ImGui::NewFrame();
            };

            fixed_update_accumulator += (raw_delta_time > 0.25f) ? 0.25f : raw_delta_time;

            while ( fixed_update_accumulator >= fixed_time_step ) {
                ev->call_event( FIXED_UPDATE, Gmeng::UPDATE_INFO );
                fixed_update_accumulator -= fixed_time_step;
            };

            ev->call_event( UPDATE, Gmeng::NO_EVENT_INFO );
        };
    });

    /// handler! thread for and developer console.
    /// this also handles the EventLoop::next_tick processes.
    /// this also handles dev console processes.
    create_thread([&]() {
        while (!ev->cancelled) {

            ///unsigned int model_count = 0;
            ///for ( auto chunk : ev->level->chunks )
            ///    model_count += chunk.models.size();
            ///ev->level->display.camera.model_count = model_count;

            if (state.console_open && !ev->level->display.camera.has_modifier("kitty_graphics"))
                gmeng_dev_console(ev, &Gmeng::NO_EVENT_INFO); // developer console is on, no raw update.
            else if (gmeng_console_state_change_modifier) {
                gmeng_console_state_change_modifier = false;

                gmeng_run_dev_command(ev, "refresh", true); // refresh the screen
            };


            interaction_map = ev->level->entity_grid.build_interaction_map();
        };

    });
///#ifndef GMENG_COMPILING_SCRIPT
///    /// handler thread for all server utils.
///    /// not exactly for multiplayer, but for a server instance
///    /// that allows for external connection to the server.
///    /* disabled 12.0.0
///    create_thread([&]() -> void {
///        auto server_handle = noble_file_open("scripts/server.dylib");
///
///        int port = noble_function(server_handle, gmeng_script_command, ev);
///        GAME_LOG("server at port "$(port)" was closed");
///    });*/
///#endif

    bool resize_was_done = false;
    bool cubic_render = ev->level->display.camera.has_modifier("cubic_render");
    font_size = get_font_size_values();

    std::string term_prog = getenv("TERM_PROGRAM") != nullptr ? getenv("TERM_PROGRAM") : "";

    /// main handler thread, for the game events
    while (!ev->cancelled) {
        ssize_t n = read(STDIN_FILENO, buf, sizeof(buf)); /// total bytes read
        Gmeng::Event t_event = Gmeng::UNKNOWN; // current event
        Gmeng::EventInfo info; // filled later, event info

        bool imgui_enabled = ev->level->display.camera.has_modifier("imgui");

        if (n > 0) { /// if the event is not null
            buf[n] = '\0'; /// string terminate
            if (strstr(buf, "\033[<") != nullptr) {
                /// MOUSE EVENTS (scroll, click, release)
                /// BUTTON IDS:
                ///  - scroll up: 64,
                ///  - scroll down: 65
                ///  - left click: 0
                ///  - middle click: 1
                ///  - right click: 2
                ///  - all buttons release: 3 (track with state.last_event to determine)
                ///  - mouse move: 35 ( 1006 raw input mode required )
                int button, x, y;
                int raw_x = 0, raw_y = 0;
                char eventType;

                auto scan_out = sscanf(buf, "\033[<%d;%d;%d%c", &button, &x, &y, &eventType);
                if (scan_out != 4) {
                    gm_log("while parsing STDIN raw input, a determined mouse event did not match the parsed sscanf output.");
                    gm_log("ignoring this event.");
                    continue;
                };

                /// TODO: !!!!!!!!!!!!!!!!!!!!!!!!!!!
                /// don't fire the mouse event if the
                /// X/Y coordinates of the event are
                /// larger than the display viewpoint.

                Gmeng::Event event_tp = SELECT_MOUSE_EVENT(button);
                DECIDE_IF_MOUSEUP_OR_MOUSEDOWN(event_tp, eventType);

                /// if SGR-Pixel is enabled, mouse reporting will be
                /// done via pixel coordinates. We divide the pixels
                /// by the font's width and height to get the correct
                /// column and row of the mouse click.
                ///
                /// This is useful because without pixels, we can't know
                /// the Y coordinate of the mouse position correctly.
                /// cubic_render binds Y and Y+1 coordinates together,
                /// so we use this to divide the font's height by 2 if
                /// cubic_render is enabled. Allows us to get two Y coordinates
                /// per a single character displayed on the terminal.
                if ( Gmeng::global.terminal_has_sgr_pixel_support ) {
                    /// Save the raw values to raw_x and raw_y
                    raw_x = x, raw_y = y;
                    /// Check if user is running iTerm. iTerm has SGR-Pixel
                    /// support but reports every second pixel instead, so
                    /// we have to multiply by 2 if the user is running iTerm.
                    int size_factor = (term_prog == "iTerm.app" ? 2 : 1);
                    x = x  / (font_size.x / ( size_factor ) );
                    y = (y / ((font_size.y / ( size_factor )) / ( cubic_render ? 2 : 1 )));
                } else {
                    /// 1006-mode SGR column, row reporting reports
                    /// 1-based values, we need them to convert to 0.
                    x--; y--;
                };

                if (event_tp == Gmeng::UNKNOWN) {
                    gm_log("received an unknown supposed mouse event: SELECT_MOUSE_EVENT(x): { UNKNOWN, .button="$(button)", .posX="$(x)", .posY="$(y)" }");
                    gm_log("cancelling this event call.               ^ ~~~~~~~~~~~~~~~~~~");
                    continue;
                };

                if (event_tp == MOUSE_CLICK_LEFT_END || event_tp == MOUSE_CLICK_RIGHT_END || event_tp == MOUSE_CLICK_MIDDLE_END) {
                    Gmeng::EventInfo d = {
                        .EVENT = Gmeng::MOUSE_CLICK_END_ANY,
                        .KEYPRESS_CODE = 0,
                        .MOUSE_X_POS = 0,
                        .MOUSE_Y_POS = 0,
                        .prevent_default = false
                    };

                    ev->call_event(Gmeng::MOUSE_CLICK_END_ANY, d);
                };

                bool is_alternative = (
                    button == 39 || button == 4 || button == 6 || button == 71 || button == 70
                //  shift+move      shift+LMB      shift+RMB      shift+SCROLLUP  shift+SCROLLDOWN
                );

                info = {
                    .EVENT = event_tp,
                    .KEYPRESS_CODE = (char)button,
                    .MOUSE_X_POS = x,
                    .MOUSE_Y_POS = y,
                    .RAW_MOUSE_POS = { raw_x, raw_y },
                    .prevent_default = false,
                    .alternative = is_alternative
                };

                t_event = event_tp;
            } else {
                /// Application Focus Events
                /// \033[O -> Focus lost
                /// \033[I -> Focus gained
                if (std::string(buf) == "\033[O" ||
                    std::string(buf) == "\033[I") {
                    Gmeng::global.app_focused = std::string(buf) == "\033[I";
                    continue;
                }
                /// KEYBOARD EVENTS (keypress)
                if ( Gmeng::global.terminal_has_keyboard_support ) {
                    /// The terminal supports the Kitty Terminal Keyboard Protocol.
                    /// We can track individual key states, so let the keyboard.h
                    /// utility handle the parsing for the keys and update the
                    /// `Gmeng::key_states` map which contains key pressed states.
                    int code;
                    KEYEVENT event;
                    /// Parse the key sequence and update the key states
                    gmeng_update_keys(buf, code, event);

                    /// keycode is bigger than largest Extended ASCII character keycode
                    /// i'm not actually sure if this is fine to do actually, but i don't care
                    /// haha, US keyboard defaultism from a non-USian...
                    if ( code > 255 ) continue;

                    /// Still cast the KEYPRESS / KEYRELEASE events to the event loop.
                    info = {
                        .EVENT = gmeng_keyevent( event ),
                        .KEYPRESS_CODE = gmeng_toinput(code),
                        .MOUSE_X_POS = -1,
                        .MOUSE_Y_POS = -1,
                        .prevent_default = false
                    };

                    if ( ev->modifiers.get_value("allow_console") == 1 && event != gkEv_RELEASE ) {
                        if ( ( getkey( gk_LSHIFT ) || getkey( gk_RSHIFT ) ) &&
                               getkey( gk_BACKQUOTE )) {

                            if ( !GMENG_CONSOLE_FOCUSED ) {

                                if (state.console_open || !Gmeng::global.terminal_has_kgp_pixel_support) {
                                    ev->level->display.camera.clear_screen();
                                }

                                state.console_open = !state.console_open;
                                dev_console_open = state.console_open;

                                if ( !imgui_enabled ) gmeng_dev_console(ev, &info);
                                gmeng_run_dev_command(ev, "refresh", true);
                            };

                        };

                        Gmeng::EventInfo d = {
                            .EVENT = Gmeng::MOUSE_CLICK_END_ANY,
                            .KEYPRESS_CODE = 0,
                            .MOUSE_X_POS = 0,
                            .MOUSE_Y_POS = 0,
                            .prevent_default = false
                        };

                        ev->call_event(Gmeng::MOUSE_CLICK_END_ANY, d);
                    };
                    t_event = info.EVENT;
                } else {
                    /// No kitty keyboard protocol support, KEYCODE is buf[0]; read from
                    /// the default tty stdin file, does not support individual key tracking.
                    info = {
                        .EVENT = Gmeng::KEYPRESS,
                        .KEYPRESS_CODE = buf[0],
                        .MOUSE_X_POS = -1,
                        .MOUSE_Y_POS = -1,
                        .prevent_default = false,
                    };

                    if (ev->modifiers.get_value("allow_console") == 1) {
                        if (info.KEYPRESS_CODE == '~') { /// either ESC or shift+tab
                            if (state.console_open) ev->level->display.camera.clear_screen();
                            state.console_open = !state.console_open;
                            dev_console_open = state.console_open;


                            gmeng_dev_console(ev, &info);
                            gmeng_run_dev_command(ev, "refresh", true);
                        };

                        Gmeng::EventInfo d = {
                            .EVENT = Gmeng::MOUSE_CLICK_END_ANY,
                            .KEYPRESS_CODE = 0,
                            .MOUSE_X_POS = 0,
                            .MOUSE_Y_POS = 0,
                            .prevent_default = false
                        };

                        ev->call_event(Gmeng::MOUSE_CLICK_END_ANY, d);
                    };

                    t_event = Gmeng::KEYPRESS;
                };
            };
        };

        Gmeng::EventInfo scope = {
            .EVENT = t_event
        };

        /*for ( auto interaction : interaction_map ) {
            for ( auto interactor : interaction.second )
                interaction.first->_interact( {
                    Gmeng::Entity_Interaction::PROXIMITY,
                    interactor
                }, ev->level );
        };*/

        /// Since 13.5.1: FIXED_UPDATE now handled by UPDATE thread
        if (imgui_enabled || !state.console_open) ev->call_event(info.EVENT, info);
        else if ( !Gmeng::global.terminal_has_kgp_pixel_support ) gmeng_dev_console(ev, &info);
    };
    /// call EXIT
    ev->call_event( Gmeng::EXIT, Gmeng::NO_EVENT_INFO );
    // exit scene, since the while loop broke, ev->cancelled is true.
    gm_log("main game event loop (with id "$(ev->id)") closed");
    Gmeng::main_event_loop = nullptr;

    /// disable all terminal modes
    Gmeng::TerminalUtil::set_non_blocking(false);
    Gmeng::TerminalUtil::set_raw_mode(false);
    Gmeng::TerminalUtil::disable_mouse_tracking();
    /// disable kitty's keyboard protocol
    gmeng_keyboard_deinit();

    // clear threads
    Gmeng::clear_threads();
    return 0;
};

#endif // NO WIN32
#endif // GMENG_SDL

#ifdef GMENG_SDL

/// ignore coc-vim errors, this code compiles
#include "../types/IconsFontAwesome7.h"
    #include "../utils/window.cpp"

#include "../utils/hexcolors.cc"

namespace Gmeng {
    GmengImGuiVGM* imgui_vgm_handler;
    GmengImGuiConsole* imgui_console_handler;
    GmengImGuiInfoWindow* imgui_info_handler;
    GmengImGuiLevel* level_inspector_handler;
    ImFont font;
};


/// SDL-Based Gmeng Event Loop.
int do_event_loop(Gmeng::GameWindow* win, Gmeng::EventLoop* ev, TRACEFUNC) {
    __functree_call__(do_event_loop::external_window_mode : SDL);
    gm_log("[GMENG::SDL_BINDING] External Window do_event_loop, requested from " + TRACEFUNC_STR);

    if (Gmeng::main_event_loop != nullptr) {
        gm_log("Call to create an eventloop handler from " + TRACEFUNC_STR + " REJECTED: Gmeng::main_event_loop is not nullptr, cannot initialize multiple event loops in once instance.");
        return -1;
    };

    Gmeng::main_event_loop = ev;

    if (win == nullptr || ev == nullptr) return -2;
    if (ev->cancelled) return 1;

    /// ImGui Context Creation
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontDefault();

    ImFontConfig f_cfg;
    f_cfg.MergeMode = true;
    f_cfg.OversampleH = 4;
    f_cfg.GlyphMinAdvanceX = 24.0f;
    static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    io.Fonts->AddFontFromFileTTF("assets/fa7.ttf", 24.0f, &f_cfg, icon_ranges);

    ImGui_ImplSDL2_InitForSDLRenderer(win->window, win->renderer);
    ImGui_ImplSDLRenderer2_Init(win->renderer);

    imgui_style_global();

    gm_log("ImGui init complete");

    ev->call_event(Gmeng::Event::INIT, Gmeng::INIT_INFO);

    /// Developer console
    GmengImGuiConsole console(GAME_LOGSTREAM_STR);
    Gmeng::imgui_console_handler = &console;

    GmengImGuiInfoWindow info_window(ev);
    Gmeng::imgui_info_handler = &info_window;

    GmengImGuiScripts script_manager;
    GmengImGuiVGM vgm_manager( ev, win->renderer );
    Gmeng::imgui_vgm_handler = &vgm_manager;
    bool vgm_open = true;

    GmengImGuiLevel level_inspector( win->renderer, ev );
    Gmeng::level_inspector_handler = &level_inspector;
    bool level_inspector_open = false;

    ev->states = Gmeng::EventLoop::UtilStateHolder {
        &dev_console_open,
        &vgm_open,
        &level_inspector_open
    };

    GAME_LOG("gmeng " + Gmeng::version + " build " + std::string(GMENG_BUILD_NO) + " developer console");
    GAME_LOG("run 'help' for help.");
    GAME_LOG("contribute: https://gmeng.org/git");
    GAME_LOG("gmeng is built & maintained by catriverr");

    SDL_Event e;
    /// Main Event Loop
    while (!ev->cancelled) {
        /// SDL Event Processing
        while (SDL_PollEvent(&e) != 0) {
            /// For ImGui
            ImGui_ImplSDL2_ProcessEvent(&e);
            /// For Gmeng User (optional)
            if (win->external_sdl_event_handler != NULL)
                win->external_sdl_event_handler(&e);

            /// For Gmeng Engine (internal)
            if (e.type == SDL_QUIT) {
                gm_log("SDL_Quit called for EventLoop with id " + v_str(ev->id) + ", EventLoop origin is from " + TRACEFUNC_STR + ".");
                ev->call_event(Gmeng::Event::EXIT, Gmeng::NO_EVENT_INFO);
                ev->cancel();
            } else if (e.type == SDL_WINDOWEVENT) {
                /// If/When the window is resized, update the window size properties
                /// within the window object as well.
                if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                    /// Getting the window size is a reference passed function in SDL,
                    /// just passing it directly to the window's width & height variables
                    /// will result in them being updated to the correct value.
                    SDL_GetWindowSize(win->window, &win->width, &win->height);
                    gm_log("Window resized to "$(win->width)"x"$(win->height)".");
                    win->frame_width = win->width; win->frame_height = win->height;
                };
            } else if (e.type == SDL_KEYDOWN) {
                /// A key on the keyboard has been pressed,
                /// Since SDL uses key repeat (and we want it)
                /// we will not ignore repeat events.
                auto key_sym = e.key.keysym.sym;

                bool shift = (e.key.keysym.mod & KMOD_SHIFT);
                bool ctrl = (e.key.keysym.mod & KMOD_CTRL);
                bool alt = (e.key.keysym.mod & KMOD_ALT);

                /// Default keystroke for Gmeng Dev Console Open
                if ( key_sym == SDLK_TAB && shift ) {
                    if ( Gmeng::global.dev_console ) dev_console_open = !dev_console_open;
                    if (dev_console_open) gm_log("developer console opened (instance " + _uconv_1ihx(GET_TIME()) + ")");
                    level_inspector_open = dev_console_open;

                    if (dev_console_open) {
                        win->frame_height = win->frame_height/2;
                        win->frame_width = win->frame_width/2;
                    } else {
                        win->frame_height = win->frame_height*2;
                        win->frame_width = win->frame_width*2;
                    };
                };

                /// Event Information
                Gmeng::EventInfo info = {
                    Gmeng::KEYPRESS, 0, key_sym,
                    -1, -1, 0, 0,
                    false, shift, ctrl, alt
                };
                /// Call the event if none of the editors/controllers are open
                if (!dev_console_open)
                    ev->call_event( Gmeng::Event::KEYPRESS, info );
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                /// Mouse clicks, we want all 3 mouse buttons
                auto button = e.button.button;
                Gmeng::Event mb_event = Gmeng::UNKNOWN;
                switch (button) {
                    case SDL_BUTTON_LEFT:  {
                                             mb_event = Gmeng::Event::MOUSE_CLICK_LEFT_START;
                                           }; break;
                    case SDL_BUTTON_RIGHT: {
                                             mb_event = Gmeng::Event::MOUSE_CLICK_RIGHT_START;
                                           }; break;
                    case SDL_BUTTON_MIDDLE:
                                           {
                                             mb_event = Gmeng::Event::MOUSE_CLICK_MIDDLE_START;
                                           };
                    default:
                        gm_log("unknown mouse button click, ignoring");
                        break;
                };
                /// Check the mouse event
                if (mb_event != Gmeng::Event::UNKNOWN) {
                    /// The Mouse button event is valid, continue and initialize the event

                    /// CTRL+Click
                    bool ctrl  = (SDL_GetModState() & KMOD_CTRL );
                    /// Shift+Click
                    bool shift = (SDL_GetModState() & KMOD_SHIFT);
                    /// Event Information
                    Gmeng::EventInfo info = {
                        mb_event, 0, 0,
                        e.button.x, e.button.y, 0, 0,
                        false, shift, ctrl, false
                    };
                    /// Call the event
                    ev->call_event( mb_event, info );
                };
            } else if (e.type == SDL_MOUSEBUTTONUP) {
                /// Mouse click end, we want all 3 mouse buttons
                auto button = e.button.button;
                Gmeng::Event mb_event = Gmeng::UNKNOWN;
                switch (button) {
                    case SDL_BUTTON_LEFT:  {
                                             mb_event = Gmeng::Event::MOUSE_CLICK_LEFT_END;
                                           }; break;
                    case SDL_BUTTON_RIGHT: {
                                             mb_event = Gmeng::Event::MOUSE_CLICK_RIGHT_END;
                                           }; break;
                    case SDL_BUTTON_MIDDLE:
                                           {
                                             mb_event = Gmeng::Event::MOUSE_CLICK_MIDDLE_END;
                                           };
                    default:
                        gm_log("unknown mouse button up, ignoring");
                        break;
                };
                /// Check the mouse event
                if (mb_event != Gmeng::Event::UNKNOWN) {
                    /// The Mouse button event is valid, continue and initialize the event

                    /// Event Information
                    /// shift/ctrl does not matter here (hopefully)
                    Gmeng::EventInfo info = {
                        mb_event, 0, 0,
                        e.button.x, e.button.y, 0, 0,
                        false, false, false, false
                    };
                    /// Call the event
                    ev->call_event( mb_event, info );
                    /// Also call mouse_event_end_any, compatibility with terminal mode
                    ev->call_event( Gmeng::Event::MOUSE_CLICK_END_ANY, info);
                };
            } else if (e.type == SDL_MOUSEMOTION) {
                /// Mouse Move Events
                int x = e.motion.x, y = e.motion.y;
                int rx = e.motion.xrel, ry = e.motion.yrel;
                /// Event Information
                Gmeng::EventInfo info = {
                    Gmeng::Event::MOUSE_MOVE, 0, 0,
                    x, y, rx, ry,
                    false, false, false, false
                };
                /// Call the event
                ev->call_event( Gmeng::Event::MOUSE_MOVE, info );
            } else if (e.type == SDL_MOUSEWHEEL) {
                /// Mouse Scroll Events
                bool shiftHeld = (SDL_GetModState() & KMOD_SHIFT) != 0;

                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                Gmeng::Event mw_event = e.wheel.y > 0 ? Gmeng::Event::MOUSE_SCROLL_UP : Gmeng::MOUSE_SCROLL_DOWN;
                bool alternative = shiftHeld || e.wheel.x != 0;
                /// Event Information
                Gmeng::EventInfo info = {
                    mw_event, 0, 0, mouseX, mouseY, mouseX, mouseY,
                    false, alternative, false, false
                };
                /// Call the event
                ev->call_event( mw_event, info );
            };
        };

        //ev->level->display.camera.entity_count = ev->level->entities.size();

        auto m_time_begin = GET_TIME();
        /// Outragiously long and idiotic Frame Begin Definition for ImGui
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        /// All external ImGui utils will be called from Update.
        /// They don't have to be defined here, but a simple Draw() function
        /// to an extendable class for ImGui children is well.
        ev->call_event(Gmeng::Event::UPDATE, Gmeng::NO_EVENT_INFO);

        /// After external hooks have been called, we can draw the default
        /// Gmeng ImGui definitions, however they might not be enabled,
        /// or an external hook might have disabled it. So we check.
        int i = 0;
        vector<std::string> names = { "gmeng log", "gmeng functree", "gmeng_unknown1", "gmeng_unknown2" };
        if (dev_console_open) {
            for ( GmengImGuiLog& Hook : win->default_windows ) { // 320, 375
                Hook.Draw(names.at(i).c_str(), {(float)480+(i*480), 540}, { (float)480-(i*220), 375 });
                i++;
            };

            for ( auto& t_e : ev->editors ) {
                if (!t_e.second->loaded) t_e.second->Load( t_e.second->current_filename );
                if (t_e.second->is_open) t_e.second->Draw();
            };
        };

        /// Info Menu draw, handled by GmengImGuiInfoWindow
        if (ev->level->display.camera.has_modifier("draw_info") && dev_console_open)
           info_window.Draw( win->renderer );

        /// Developer Console draw, handled by GmengImGuiConsole
        if (dev_console_open) console.Draw(
                std::string("gmeng " + Gmeng::version + " build " + GMENG_BUILD_NO + " developer console").c_str(),
                ev);

        /// VGM VisualCache Graphics Manager draw, handled by GmengImGuiVGM
        if (vgm_open && dev_console_open) vgm_manager.Draw();

        /// Gmeng Level Inspector draw, handled by GmengImGuiLevel
        if (level_inspector_open) level_inspector.Draw(), script_manager.Draw();

        /// End ImGui Frame and Render it
        ImGui::Render();
        /// Access ImGui's Renderscale for SDL, don't use SDL's Renderscale.
        SDL_RenderSetScale(win->renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        /// Clear the screen
        win->clear();
        auto m_time_frame_begin = GET_TIME();
        /// Actual Game Event Update, can draw the source image now.
        ev->call_event(Gmeng::Event::UPDATE, Gmeng::NO_EVENT_INFO);
        /// Update frame time
        ev->level->display.camera.frame_time = GET_TIME() - m_time_frame_begin;
        /// End Render Phase, draw the screen
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), win->renderer);
        /// Update draw time
        ev->level->display.camera.draw_time = GET_TIME() - m_time_begin;
        /// Refresh the screen & Present the renderer
        win->refresh();
    };
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    return 0;
};
#endif

#if _WIN32

/// WINDOWS IMPLEMENTATIONS
/// NOT COMING ANYTIME SOON.

int do_event_loop(Gmeng::EventLoop* ev) {
    /// edgy easter egg
    /// we're pretty safe since the code compiles with preprocessor
    /// target to _WIN32 so nobody running windows can magically
    /// swap to the UNIX do_event_loop functions
    std::cout << "how did you bypass the not-supported OS warning\n";
    std::cout << "stop trying to make this engine work in windows\n";
    return -2; /// -2 for windows detected
};

#endif


