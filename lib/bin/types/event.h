#pragma once
#include <string>
#include <vector>

namespace Gmeng {
    using std::vector, std::string;
    // applicable events for the game loop
    //
    // note that these events aren't what happens in a game itself, rather its interactions with the engine.
    enum Event {
        INIT,
        UPDATE,
        FIXED_UPDATE,
        RESET,
        RELOAD,
        EXIT,

        MOUSE_CLICK_LEFT_START,
        MOUSE_CLICK_RIGHT_START,
        MOUSE_CLICK_MIDDLE_START,

        MOUSE_CLICK_LEFT_END,
        MOUSE_CLICK_RIGHT_END,
        MOUSE_CLICK_MIDDLE_END,
        MOUSE_CLICK_END_ANY,

        MOUSE_SCROLL_UP,
        MOUSE_SCROLL_DOWN,
        MOUSE_MOVE, /// with 1006 raw input mode, we can capture mouse move events.

        KEYPRESS,
        KEYREPEAT,
        KEYRELEASE,

        UNKNOWN,
        EVENT_COUNT // helper for the event name receiver, do not remove
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
