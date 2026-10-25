#pragma once
#include "../src/objects.cpp"
#include <string>
#include <vector>

namespace Gmeng {
    class EventLoop;
};

namespace Gmeng {
    using std::vector, std::string;
    // applicable events for the game loop
    //
    // note that these events aren't what happens in a game itself, rather its interactions with the engine.
    enum Event {
        /// Initialization event. Called once when `do_event_loop` is ready.
        INIT,
        /// RAW update, ran every frame. NOT time-delta based.
        /// Used for processing rendering & real-time changes.
        ///
        /// Since this event is not fixed on a timeline, using
        /// it for input processing for actions like movement
        /// via `getkey()` will yield unstable results.
        ///
        /// Event handlers for physics and other time-based
        /// processes should use the FIXED_UPDATE event instead.
        UPDATE,
        /// FIXED update, time-delta based. This event is ran
        /// every `1000 / (level::display::fixed_update_fps)` milliseconds.
        ///
        /// The default timetable is 60FPS, so by default this
        /// event will be fired every ~16.6667ms. However, before
        /// calling`do_event_loop()`, users can set the
        /// `level.display.fixed_update_fps` value to their desired
        /// FPS for FIXED_UPDATE and then run the event loop to
        /// change how fast it fires.
        FIXED_UPDATE,
        /// RESET event. This happens when a core process of Gmeng
        /// is manipulated to hard fail, causing a RESET to be performed.
        RESET,
        /// RELOAD event, unused internally but scripts & the program
        /// can call this event to simulate a RELOAD event within games.
        RELOAD,
        /// EXIT event. Called when `ev->cancel()` is executed,
        /// `ev->cancelled` is equal to true, the `exit` command is called,
        /// the program receives the `SIGINT` signal from the terminal or
        /// if the application is running in SDL-Mode, when the window closes.
        EXIT,

        /// Mouse click start events
        MOUSE_CLICK_LEFT_START,
        MOUSE_CLICK_RIGHT_START,
        MOUSE_CLICK_MIDDLE_START,

        /// Mouse click end events
        MOUSE_CLICK_LEFT_END,
        MOUSE_CLICK_RIGHT_END,
        MOUSE_CLICK_MIDDLE_END,
        MOUSE_CLICK_END_ANY,

        /// Mouse scroll events
        MOUSE_SCROLL_UP,
        MOUSE_SCROLL_DOWN,
        MOUSE_MOVE, /// with 1006 raw input mode, we can capture mouse move events.

        /// Key events
        KEYPRESS,
        KEYREPEAT,
        KEYRELEASE,

        /// Unknown event
        UNKNOWN,
        /// helper for the event name receiver, unused
        EVENT_COUNT
    };

    static const std::string event_names[EVENT_COUNT] = {
        "INIT",
        "UPDATE",
        "FIXED_UPDATE",
        "RESET",
        "RELOAD",
        "EXIT",
        "MOUSE_CLICK_LEFT_START",
        "MOUSE_CLICK_RIGHT_START",
        "MOUSE_CLICK_MIDDLE_START",
        "MOUSE_CLICK_LEFT_END",
        "MOUSE_CLICK_RIGHT_END",
        "MOUSE_CLICK_MIDDLE_END",
        "MOUSE_CLICK_END_ANY",
        "MOUSE_SCROLL_UP",
        "MOUSE_SCROLL_DOWN",
        "MOUSE_MOVE",
        "KEYPRESS",
        "KEYREPEAT",
        "KEYRELEASE",
        "UNKNOWN"
    };

    /// returns the name of a Gmeng::Event as a string
    static const std::string& get_event_name(const Event& event) {

        if (event < 0 || event >= EVENT_COUNT) {
            static const std::string unknown = "UNKNOWN";
            return unknown;
        };

        return event_names[event];
    };

    /// returns a list of every event in a vector as a string
    static string list_events(vector<Event>& events) {
        string final;
        for (const auto ev : events) {
            final += get_event_name(ev) + ", ";
        };
        final.pop_back(); /// remove final ' '
        return final;
    };

    /// Gmeng Event Info, contains
    /// variables regarding an event_loop event.
    typedef struct EventInfo {
        /// The name of the Event
        Event EVENT;
        /// If it is a KEYPRESS event, this determines the keycode.
        /// TERMINAL-ONLY: Will not work with SDL.
        char KEYPRESS_CODE;
        /// For SDL-based events, so the symbol can be used instead of the charcode
        int KEYPRESS_SYM = -1;
        /// If it is a mouse event, the X position of the mouse.
        int MOUSE_X_POS = -1;
        /// If it is a mouse event, the Y position of the mouse.
        int MOUSE_Y_POS = -1;
        /// SDL-Only, mouse drag relative position X
        int MOUSE_REL_X_POS = -1;
        /// SDL-Only, mouse drag relative position Y
        int MOUSE_REL_Y_POS = -1;

        /// Raw Mouse Position, the value can be pixels if SGR-Pixel
        /// is supported by the terminal emulator the app is being ran on.
        Objects::coord RAW_MOUSE_POS = { 0, 0 };

        /// Prevents any other event hook from executing.
        bool prevent_further = false;
        /// Prevents default behaviour
        /// from executing
        bool prevent_default = false;
        /// Alternative usage.
        /// usually triggered by holding shift.
        /// one example is using shift + scroll for
        /// left-right movement instead of usual down-up movement.
        bool alternative = false;
        /// Keypress WITH ctrl keydown
        /// SDL-Only
        bool ctrl = false;
        /// Keypress WITH alt keydown
        /// SDL-Only
        bool alt = false;
        /// optional EventLoop object reference
        Gmeng::EventLoop* event_loop = nullptr;

    } EventInfo;

    static EventInfo NO_EVENT_INFO = EventInfo { Gmeng::UNKNOWN, 0, -1, -1, false };
    static EventInfo UPDATE_INFO   = EventInfo { Gmeng::UPDATE,  0, -1, -1, false };
    static EventInfo INIT_INFO     = EventInfo { Gmeng::INIT,    0, -1, -1, false };

};

#ifndef GMENG_NO_SOL

#include "../utils/scripts.h"
static int __gmeng_define_lua_event_method = ([]() -> int {
    using namespace Gmeng;
    using namespace Scripts;

    gmeng_lua_namespace.new_enum("Event",
        "INIT", Event::INIT,
        "UDATE", Event::UPDATE,
        "FIXED_UPDATE", Event::FIXED_UPDATE,
        "RESET", Event::RESET,
        "RELOAD", Event::RELOAD,
        "EXIT", Event::EXIT,
        
        "MOUSE_CLICK_LEFT_START", Event::MOUSE_CLICK_LEFT_START,
        "MOUSE_CLICK_RIGHT_START", Event::MOUSE_CLICK_RIGHT_START,
        "MOUSE_CLICK_MIDDLE_START", Event::MOUSE_CLICK_MIDDLE_START,

        "MOUSE_CLICK_LEFT_END", Event::MOUSE_CLICK_LEFT_END,
        "MOUSE_CLICK_RIGHT_END", Event::MOUSE_CLICK_RIGHT_END,
        "MOUSE_CLICK_MIDDLE_END", Event::MOUSE_CLICK_MIDDLE_END,
        "MOUSE_CLICK_END_ANY", Event::MOUSE_CLICK_END_ANY,

        "MOUSE_SCROLL_UP", Event::MOUSE_SCROLL_UP,
        "MOUSE_SCROLL_DOWN", Event::MOUSE_SCROLL_DOWN,
        "MOUSE_MOVE", Event::MOUSE_MOVE,

        "KEYPRESS", Event::KEYPRESS,
        "KEYREPEAT", Event::KEYREPEAT,
        "KEYRELEASE", Event::KEYRELEASE,

        "UNKNOWN", Event::UNKNOWN,
        "EVENT_COUNT", Event::EVENT_COUNT
    );

    gmeng_lua_namespace.new_usertype<EventInfo>(
        "EventInfo",

        "EVENT", &EventInfo::EVENT,
        "KEYPRESS_CODE", &EventInfo::KEYPRESS_CODE,
        "KEYPRESS_SYM", &EventInfo::KEYPRESS_SYM,
        "MOUSE_X_POS", &EventInfo::MOUSE_X_POS,
        "MOUSE_Y_POS", &EventInfo::MOUSE_Y_POS,
        "MOUSE_REL_X_POS", &EventInfo::MOUSE_REL_X_POS,
        "MOUSE_REL_Y_POS", &EventInfo::MOUSE_REL_Y_POS,

        "RAW_MOUSE_POS", &EventInfo::RAW_MOUSE_POS,

        "prevent_further", &EventInfo::prevent_further,
        "prevent_default", &EventInfo::prevent_default
    );

    gmeng_lua_namespace["NO_EVENT_INFO"] = NO_EVENT_INFO;
    gmeng_lua_namespace["UPDATE_INFO"]   =   UPDATE_INFO;
    gmeng_lua_namespace["INIT_INFO"]     =     INIT_INFO;

    return 0;
})();
#endif


#include "eventloop.h"
