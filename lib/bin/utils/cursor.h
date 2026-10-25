#pragma once

#include <iostream>
#include <string_view>
#include <chrono>
#include <thread>

enum class cursor_type {
    DEFAULT,        // DEFAULT
    POINTER,        // CLICKING HAND
    CROSSHAIR,      // PRECISION CROSS
    TEXT,           // TEXT I-BEAM
    WAIT,           // LOADING / SPINNING WHEEL
    HELP,           // ARROW WITH QUESTION MARK
    NOTALLOWED,     // CIRCLE WITH A SLASH
    ERESIZE,        // HORIZONTAL RESIZE ARROW
    NERESIZE,
    NWRESIZE,
    NRESIZE,
    SERESIZE,
    SWRESIZE,
    SRESIZE,
    WRESIZE,
    EWRESIZE,       // HORIZONTAL RESIZE ARROWS
    NSRESIZE,       // VERTICAL RESIZE ARROWS
    NESWRESIZE,
    NWSERESIZE,
    ZOOM_IN,
    ZOOM_OUT,
    ALIAS,
    COPY,
    NODROP,
    GRAB,
    GRABBING,
    CELL,           // PRECISION CELL
    VERTICAL_TEXT,  // VERTICAL TEXT
    MOVE,           // MOVE VECTOR

};

/// Gmeng : Helper function to map cursor type enum values to escape code names
constexpr std::string_view gmeng_get_term_cursor_name(const cursor_type& shape) {
    switch (shape) {
        case cursor_type::DEFAULT:          return "default";
        case cursor_type::POINTER:          return "pointer";
        case cursor_type::CROSSHAIR:        return "crosshair";
        case cursor_type::TEXT:             return "text";
        case cursor_type::WAIT:             return "wait";
        case cursor_type::HELP:             return "help";
        case cursor_type::NOTALLOWED:       return "not-allowed";
        case cursor_type::ERESIZE:          return "e-resize";
        case cursor_type::NERESIZE:         return "ne-resize";
        case cursor_type::NWRESIZE:         return "nw-resize";
        case cursor_type::NRESIZE:          return "n-resize";
        case cursor_type::SERESIZE:         return "se-resize";
        case cursor_type::SWRESIZE:         return "sw-resize";
        case cursor_type::SRESIZE:          return "s-resize";
        case cursor_type::WRESIZE:          return "w-resize";
        case cursor_type::EWRESIZE:         return "ew-resize";
        case cursor_type::NSRESIZE:         return "ns-resize";
        case cursor_type::NWSERESIZE:       return "nwse-resize";
        case cursor_type::NESWRESIZE:       return "nesw-resize";
        case cursor_type::ALIAS:            return "alias";
        case cursor_type::COPY:             return "copy";
        case cursor_type::NODROP:           return "no-drop";
        case cursor_type::GRAB:             return "grab";
        case cursor_type::GRABBING:         return "grabbing";
        case cursor_type::CELL:             return "cell";
        case cursor_type::VERTICAL_TEXT:    return "vertical-text";
        case cursor_type::ZOOM_IN:          return "zoom-in";
        case cursor_type::ZOOM_OUT:         return "zoom-out";
        case cursor_type::MOVE:             return "move";
    };
    return "default";
}

/// Gmeng : Function to set the mouse cursor icon using the enum
inline constexpr void gmeng_set_cursor(const cursor_type& shape) {
    std::cout << "\033]22;" << gmeng_get_term_cursor_name(shape) << "\007" << std::flush;
};

namespace Gmeng {
    static constexpr void set_pointer_icon(const cursor_type& shape) {
        gmeng_set_cursor(shape);
    };
    static constexpr void reset_pointer_icon() {
        gmeng_set_cursor( cursor_type::DEFAULT );
    };
};
