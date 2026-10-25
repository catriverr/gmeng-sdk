#pragma once
/// Gmeng EventLoop
#include "event.h"
#include "../utils/global.h"
#include "../utils/log.h"
#include "../utils/functree.h"
#include "../utils/terminal.h"

#include "modifier.h"

#include <functional>
#include <unordered_map>
#include <vector>

#include "../utils/texture_editor.h"

namespace Gmeng {
    class Level;

    using std::vector, std::string;


    /// EventHook handler function type
    using handler_function_type = std::function<void(Gmeng::Level*, EventInfo*)>;
    /// EventHook handler type for event_loop
    struct EventHook {
      public:
        int id = g_mkid(); vector<Event> events;
        handler_function_type handler;
        bool locked;

    };

    /// Gmeng EventLoop Object.
    /// EventLoop allows for a continuous running with very in-depth
    /// default behaviour as well as error handling, so the engine can
    /// do the heavy-lifting of rendering, running, optimizing and handling
    /// a game instance in Gmeng.
    ///
    /// Only one EventLoop object can be ran per gmeng instance,
    /// so no two games can be run at the same time with the same executable.
    typedef struct EventLoop {
      private:
          inline void init_default_event_hooks();
      public:
        /// ID of the GameLoop. Isn't useful within a single program but
        /// it can be used to communicate the instance on multiplayer
        /// instances to check if the required event loop is set-up on
        /// both clients, as a simple validation check in a server.
        int id;
        /// Level of the EventLoop. `do_event_loop` manages this level
        /// instance and emits events based on it.
        Gmeng::Level* level;
        /// Log stream of the level. also accessable via `GAME_LOGSTREAM`
        /// but this is neccessary for NOBLE scripts & plugins because they
        /// can't access the memory of the program they are hooked into,
        /// and since GAME_LOGSTREAM lives in the stack of the program they
        /// need a reference to it, via the `EventLoop::logstream`.
        std::vector<std::string>* logstream;
        /// log string of the level. also accessable via `GAME_LOGSTREAM_STR`
        /// but this is neccessary for NOBLE scripts & plugins because they
        /// can't access the memory of the program they are hooked into,
        /// and since GAME_LOGSTREAM_STR lives in the stack of the program they
        /// need a reference to it, via the `EventLoop::logstream_str`.
        std::stringstream* logstream_str; 
        /// EventLoop Modifiers.
        ModifierList modifiers = { {
            modifier { "allow_console", 1 },
            modifier { "server_passkey", 738867 } /// Should be changed for better password-protection in custom servers.
        } };

        /// Processes, used for registering event calls for the
        /// next tick of the event loop. Called with `UPDATE` event
        vector<EventHook> processes;

        /// Event Hooks of the Loop.
        std::unordered_map<Event, std::vector<EventHook>> hooks;
        /// Default Event Hooks of the Loop. Can be cancelled by
        /// external hooks to prevent default behaviour.
        std::unordered_map<Event, vector<EventHook>> defaults;

        /// Modifier of whether the EventLoop is cancelled or not.
        /// If cancelled is `true`, the game exits.
        bool cancelled = false;

        /// For External SDL Screen utility. Default to false.
        /// Changing this with code will not modify the SDL
        /// Window state, it is used for calcuations only.
        ///
        /// Setting this to `false` while an SDL window is open
        /// will freeze it, and you will have to force exit
        /// the window.
        bool uses_sdl = false;

        /// Total count of remote connections.
        /// This is used by default-provided script:
        /// 'server.dylib'.
        int RCON_REMOTE_COUNT = 0;

        /// type declaration for utility states
        struct UtilStateHolder {
            /// gmeng-provided developer console state
            bool* dev_console_open;
            /// gmeng-provided vgm editor state
            bool* vgm_open;
            /// gmeng-provided level inspector state
            bool* level_inspector_open;
        };

        /// holds states for the developer console,
        /// vgm editor and the level inspector.
        UtilStateHolder states;

        /// Frame times stored as an array of floats
        std::array<float, 10> frame_time_graph;

        GMENG_INIT_TYPE( id, level, logstream, logstream_str,
                         modifiers, processes, hooks, defaults,
                         cancelled, uses_sdl, RCON_REMOTE_COUNT,
                         states, frame_time_graph );

        /// EventLoop Constructor, Parses external EventHooks if any is provided.
        EventLoop( std::unordered_map<Event, std::vector<EventHook>> _hooks = {} ) : hooks( _hooks ), uses_sdl(false) {
            this->init_default_event_hooks();

            this->id = g_mkid();
            gm_log("" $(id) ": created main game eventloop with id " $(this->id) ".");

#ifndef _WIN32
#endif
        };

        ~EventLoop() {
#ifndef _WIN32
#ifndef GMENG_SDL
            gm_log("" $(id) ": destroyed eventloop " $(this->id) ".");
            gm_log("" $(id) ": disabled mouse tracking");
            TerminalUtil::set_non_blocking(false);
            gm_log(""$(id)": disabled non blocking input mode");
            TerminalUtil::disable_mouse_tracking();
#endif
#endif
        };

        void next_tick(handler_function_type handler) {
            int id = g_mkid();
            this->processes.push_back({ id, { UPDATE }, handler });
        };

        void progress_tick() {
            if (this->tick_handler) return;
            this->tick_handler = true;
            int i = 0;

            for (auto proc : this->processes) {
                /// delete the process
                this->processes.erase(this->processes.begin()+i);
                proc.handler(this->level, &Gmeng::NO_EVENT_INFO);
                i++;
            };

            this->tick_handler = false;
            /// clear next tick processes.
        };

        /// Registers an Event Hook to the EventLoop.
        /// Event Hooks can have multiple events to listen for.
        /// This is useful in cases where the interaction can be
        /// handled by the same event listener, like mouse interactions
        /// ({ MOUSE_CLICK_LEFT, MOUSE_CLICK_RIGHT }).
        void add_hook(vector<Event> events, handler_function_type handler);

        /// Emits an Event of type 'ev' with the specified EventInfo
        void call_event( const Event& ev, EventInfo& info ) {
            /// set the event loop pointer
            info.event_loop = this;
            /// Search for event handlers, call each hook that uses it
            for (auto& hook : this->hooks[ev]) {
                /// check if the event handler is busy
                if (hook.locked) continue;
                    /// lock the handler, prevent being recalled
                    /// while the handler is busy.
                    hook.locked = true;
                    /// Run the handler for the hook.
                    hook.handler( this->level, &info );
                    /// unlock the handler
                    hook.locked = false;

                    if ( info.prevent_further ) return;
            };

            /// If the event has prevent_default enabled via &EventInfo.prevent_default = true,
            /// do not run the default hook for it (if one exists)
            if (info.prevent_default || info.prevent_further) return;

            /// Search for default event handlers
            if ( !this->defaults.empty() )
                for (auto& hook : this->defaults[ ev ]) {
                    /// check if the event handler is busy
                    if (hook.locked) continue;
                        /// lock the handler, prevent being recalled
                        /// while the handler is busy.
                        hook.locked = true;
                        /// Runs the handler for the hook.
                        hook.handler( this->level, &info );
                        /// unlock the handler
                        hook.locked = false;
                };
        };


        /// ImGui Texture editor map.
        std::map<std::string, std::unique_ptr<GmengImGuiTextureEditor>> editors;

        /// Ends the EventLoop, setting this->cancelled to true
        void cancel() { this->cancelled = true; };

        private:
          bool tick_handler = false;
    } EventLoop;

    /// Main Gmeng EventLoop, for accessibility from everywhere.
    inline EventLoop* main_event_loop = nullptr;


    typedef struct EventLoop_Controller_State {
        Event last_event = INIT;
        bool console_open = false;
    } EventLoop_Controller_State;
};

#define GMENG_TEXTURE_EDITOR_IMPLEMENTATION
#include "../utils/texture_editor.h"
#undef GMENG_TEXTURE_EDITOR_IMPLEMENTATION

#include "../utils/gm_imgui.h"
#include "../utils/hexcolors.cc"

inline void Gmeng::EventLoop::init_default_event_hooks() {
        __functree_call__(Gmeng::EventLoop::init_default_event_hooks);
#ifndef GMENG_SDL
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); 
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplGmeng_Init(this);
        ImGui_ImplKitty_Init();

        imgui_style_global();
#endif
};

inline void Gmeng::EventLoop::add_hook(std::vector<Gmeng::Event> events, handler_function_type handler) {
    __functree_call__(Gmeng::EventLoop::add_hook);
    /// Unique identifier for the EventHook
    int id = g_mkid();
    /// Register the event hook to the EventLoop
    for ( auto& event : events ) {
       this->hooks[event].push_back({ id, events, handler });
    };
    /// log the new event hook
    gm_log("registered an EventHook("$(id)") for the following events:");
    gm_log(list_events(events) + ".");
};
