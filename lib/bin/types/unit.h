#pragma once

#include <string>
#include <vector>

namespace Gmeng {
    // unicode characters for the 'unit' pixel
    // terminal-only - not used with sdl2 or ncurses
    // for ncurses, see wc_unit
    const char c_unit[4] = "\u2588";
    /// Ceiling outer_unit. Half a character in height.
    /// In monospaced fonts, this is a perfect 1x1 square.
	const char c_outer_unit[4] = "\u2584";
    /// Floor outer unit. Half a character in height.
    /// In monospaced fonts, this is a perfect 1x1 square.
	const char c_outer_unit_floor[4] = "\u2580";

    /// Unit : Gmeng Implementation for screen piece.
    /// Works as a pixel, a fragment of a texture, model,
    /// entity, or display map.
	struct Unit {
      public:
            /// 24-bit representation of RGB derived
            /// from `gmeng::color32_t`. values 0-7 are
            /// reserved for Gmeng's default palette.
			uint32_t color = 1;
            /// whether the Unit can be collided with.
            /// If set to true, collision is disabled
            /// and entities can collide with this specific
            /// unit within its larger container (Models, Entities.)
            bool collidable = true;
            /// @deprecated gmeng 1.1 - whether the unit is part of an entity.
            /// kept for backwards compatibility but is unused since gmeng 4.0.
            bool is_entity = false;
            /// whether the Unit is transparent. If set
            /// to true, the unit is treated as nonexistent,
            /// and will not have collision enabled even if
            /// its `collidable` property is set to true.
            bool transparent = false;
            /// whether the unit is special. Special units
            /// will break `cubic_render` functionality and
            /// display a full-height character in the size
            /// of a 2x1 rectangle, but allow for displaying
            /// custom items within 2 units. For esoteric
            /// textures and models, this can come in handy.
            bool special = false;
            /// color of the special unit. If `special` is
            /// enabled, `special_clr` will set its foreground
            /// color (24-bit RGB derived from `gmeng::color32_t`).
            int special_clr = 0;
            /// character string of the special unit. If
            /// `special` is set, this item will be displayed
            /// in a 2x1 rectangular full-size character occupying
            /// the next Y coordinate's same X position.
            char special_c_unit = ' ';

            ~Unit() = default;

            bool operator==(const Unit&) const = default;
	};

};
