#pragma once
#include <string>
#include <vector>

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
        /// calling `do_event_loop()`, users can set the
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
    static const std::string& get_event_name(Event event) {

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
};
