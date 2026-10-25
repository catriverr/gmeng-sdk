#pragma once

#include "color32.h"
#include "entity.h"
#include "modifier.h"

#include "../utils/log.h"
#include "../utils/global.h"
#include "../utils/warning.h"
#include "../utils/colorformat.h"

#include "../utils/graphics.h"
#include "../utils/terminal.h"
#include "../utils/repeat.h"
#include "../utils/text.h"

#include "../src/objects.cpp"
#include "../strings/trim.cc"

#include "texture.h"
#include "unit.h"

#include <cstddef>
#include <cstdint>


/// MAX MAP SIZE
/// can be set in the makefile
/// version 10.1.1 will add this option to `make configure`
#ifndef GMENG_MAX_MAP_SIZE
    #define GMENG_MAX_MAP_SIZE 65434
#endif

namespace Gmeng {

    /// DisplayMap, container for the unitmap within the
    /// camera. Units are not raw, so they can be accessed
    /// by external functions to be modified. Such changes
    /// will be reflected after `get_lvl_view` and before
    /// `emplace_lvl_camera` is called. Modifying them
    /// directly will only reflect for a single frame.
	template<std::size_t d_width = 0, std::size_t d_height = 0>
	class DisplayMap {
		public:
		/// Height of the DisplayMap.
        int __h = d_width;
        /// Width of the DisplayMap.
        int __w = d_height;
        /// Contains the undrawn unitmap within the camera.
        /// Units are not raw, so they can be accessed by
        /// external functions to be modified. Such changes
        /// will be reflected after `get_lvl_view` and before
        /// `emplace_lvl_camera` is called. Modifying them
        /// directly will only reflect for a single frame.
        std::vector<Gmeng::Unit> unitmap = std::vector<Gmeng::Unit>(65536);
        /// Pool size of the unitmap. Differs from `CONSTANTS::UNITMAP_SIZE`.
        /// This property calculates (at compile-time) the byte size of the
        /// unitmap in accordance to the byte size of one `Gmeng::Unit` instance.
        int pool_size = unitmap.size();
        /// Data of raw framebuffer. Used & updated if the Kitty Terminal
        /// Graphics Protocol is supported. This is used to display raw
        /// framebuffers to the terminal program's screen, which is much
        /// faster and more efficient than drawing ASCII colorcodes.
        uint8_t* framebuffer = NULL;
	};

    /// UI element blob that can be clicked/interacted.
    ///
    /// has callbacks for hover, left click and right click.
    ///
    /// Using the ui_clickable type is not done via creating
    /// instances and applying them. This is automatically formatted from
    /// a text string.
    ///
    /// Example usage:
    ///
    /// ui_element { .text="click #clickable{ <tag>, <color32_t bg>, <color32_t fg>, label of clickable } to quit." }
    /// <type> is BLOB, BUTTON or UNKNOWN. Set via 0,1,2 integers.
    /// <color32_t> is integers in color32 formatting. default colors are available 0-7.
    /// <label> is the title of the clickable entity that will show up in drawing.
    struct ui_clickable {
      public:
        enum clickable_type { BLOB, BUTTON, UNKNOWN };
        /// tag for clickable. the ID is used to differentiate clickable objects.
        unsigned int tag = g_mkid();
        /// type of clickable.
        clickable_type type = BUTTON;
        /// background color of the clickable.
        color32_t color_bg = color32_t((uint32_t)BLACK);
        /// foreground color of the clickable.
        color32_t color_fg = color32_t((uint32_t)WHITE);
        /// label of the clickable.
        std::string label = "[clickable." + v_str(this->tag) + "]";

        ui_clickable() = default;
        ui_clickable( unsigned int tag, unsigned int type, color32_t bg, color32_t fg, std::string label ) :
            tag( tag ), type( (clickable_type)type ), color_bg( bg ), color_fg( fg ), label ( label ) {
                gm_log("ui_clickable entity created with tag " + v_str(tag) + ".");
            };
    };

    /// UI element blob that can be emplaced to a Camera instance
    /// to be drawn. Can hold ui_clickable items such as buttons
    struct ui_element {
      public:
        /// position of the element. default is 1,1 which is the top left corner with
        /// a padding of 1 unit from the X and Y coordinates.
        Objects::coord position;
        /// active status. When set to false the Camera instance this element
        /// is emplaced unto will remove it from the std::vector of active elements.
        bool active = true;
        /// id for the element.
        int id = g_mkid();
        /// background color for the element
        /// default is black.
        color32_t color_bg = (uint32_t)BLACK;
        /// foreground color for the element
        /// default is white.
        color32_t color_fg = (uint32_t)WHITE;
        /// formatted text for the element. This supports ui_clickables.
        /// see doc:ui_clickable on how to incorporate ui_clickables into your
        /// ui elements.
        std::string text;


        /// expiration time for the element (in milliseconds)
        /// default (0) means no expiration until its 'active' parameter is set to false.
        unsigned int expire_after = 0;
        /// creation time_ms state for the element.
        unsigned long long created_at = 0;
        ui_element( unsigned int expire_after, std::string text, Objects::coord pos = { 0, 0 }, color32_t col_bg = (uint32_t)BLACK, color32_t col_fg = (uint32_t)WHITE ) :
        active( true ), position(pos), expire_after( expire_after ), text( text ), color_bg( col_bg ), color_fg( col_fg ) {
            DEBUGGER gm_log("UI element created with ID " + v_str(id) + ". Expiration is t+"$(expire_after)"ms.");
            this->created_at = GET_TIME();
        };


        ui_element() = default;
    };

    /// Manages UI elements for a Camera instance.
    class Camera_UI_Manager {
      public:
        struct formatted_text_obj {
            enum what_is_it { TEXTBLOB, CLICKABLE };
            what_is_it type;
            std::string content_or_tag;
        };


        struct formatted_element {
            ui_element base;
            std::unordered_map<unsigned int, ui_clickable> clickables;
            std::vector<formatted_text_obj> text_formatted;
            Objects::coord position;
        };

        std::unordered_map<int, ui_element> elements;
        std::unordered_map<int, formatted_element> cache;


        ui_clickable parse_clickable( std::string& _text ) {
            gm_log("[" +_text + "]");
            auto text = trim( _text );
            ui_clickable final;

            if ( !text.starts_with("#clickable{") || !text.ends_with("}") ) {
                gm_err(1, "invalid format of ui_clickable as text for parse_clickable. PREFIXSUFFIXINVALID");
            };

            /// starting reading position because 11 equals the length of '#clickable{'
            /// which we will skip because there's nothing there to be parsed as value.
            int readpos = 11;
            while ( text.at(readpos) == ' ' ) readpos++;
            std::string sub = text.substr( readpos, text.find_last_of("}")-readpos );
            auto splitted = g_splitStr( sub, "," );
            int i = 0;
            for ( auto split : splitted ) {
                auto s_text = trim(split);
                if ( i == 0 ) // tag
                    final.tag = std::stoi( s_text );
                if ( i == 1 ) // bg color
                    final.color_bg = color32_t((uint32_t)std::stoi( s_text ));
                if ( i == 2 ) // fg color
                    final.color_fg = color32_t((uint32_t)std::stoi( s_text ));
                i++;
                if (i > 2) break;
            };
            // the previous values have been used and unneeded, delete them so they are not added to the label.
            splitted.erase( splitted.begin(), splitted.begin()+3 );
            std::string text_fin = trim(g_joinStr(splitted, ",")); // join the remaining text and trim
            text_fin = trim(text_fin.substr(0, text_fin.length()-1)); // remove trailing }
            final.label = text_fin;
            gm_log( text_fin );
            return final;
        };

        struct clickablestate {
            std::vector<size_t> positions;
            ui_clickable clickable;
        };

        formatted_element format_element(ui_element& element_full) {
            std::vector< clickablestate > clickable_positions;
            formatted_element final;
            std::string element = element_full.text;

            size_t pos = element.find("#clickable{");
            size_t endpos = 0;

            while (pos != std::string::npos) {
                endpos = element.find("}",endpos+1);
                if ( endpos == std::string::npos ) { pos = element.find("#clickable{", endpos+1); continue; };
                /// add the clickable entity here.
                /// substr takes a length parameter instead of a start pos and end pos parameter weirdly.
                /// pos-endpos+1 equals the length of the text we want.
                std::string clickable_text = element.substr(pos, endpos-pos+1);
                ui_clickable clickable_rn = parse_clickable( clickable_text );
                clickable_positions.push_back( { { pos, endpos }, clickable_rn });
                final.clickables[ clickable_rn.tag ] = clickable_rn;

                /// setting the pos to find after endpos means we skip nested clickables, which are not supported because
                /// they are not useful anyways and may rise complications in position setting.
                pos = element.find("#clickable{", endpos+1);
            };


            int last_endpos = 0;
            for ( auto pos_s : clickable_positions ) {
                std::string current_blob = element.substr(last_endpos+( last_endpos == 0 ? 0 : 1 ), pos_s.positions[0]-last_endpos-( last_endpos == 0 ? 0 : 1 ) );
                /// if a minor formatting problem occurs, it will occur here.
                last_endpos = pos_s.positions[1];

                final.text_formatted.push_back( { formatted_text_obj::TEXTBLOB,  current_blob } );
                final.text_formatted.push_back( { formatted_text_obj::CLICKABLE, v_str(pos_s.clickable.tag) } );
            };
            auto final_text = element.substr(last_endpos+(last_endpos == 0 ? 0 : 1));
            final.text_formatted.push_back({ formatted_text_obj::TEXTBLOB, final_text });

            final.base = element_full;
            final.position = final.base.position;
            return final;
        };

        /// Get the final text content of the UI element as input.
        /// Requires a formatted element. Use format_element( ui_element& )
        /// to convert a raw element to a formatted element.
        std::string get_final(formatted_element& elem) {
            /// Final text. Apply the background + foreground color setting of the base element.
            std::string base_color = BCOLOR32_TO_ASCII( elem.base.color_bg ) + FCOLOR32_TO_ASCII( elem.base.color_fg );
            std::string final = base_color;
            /// loop through all text objects.
            for ( auto e : elem.text_formatted ) {
                if (e.type == formatted_text_obj::TEXTBLOB ) final += colorformat(e.content_or_tag);
                else {
                    auto v = elem.clickables.at( std::stoi(e.content_or_tag) );
                    final += BCOLOR32_TO_ASCII( v.color_bg ) + FCOLOR32_TO_ASCII( v.color_fg ) + v.label + base_color;
                };
            };
            return final + Gmeng::resetcolor;
        };

        /// Adds a UI element to the Camera.
        inline void add_element( ui_element& elem ) {
            DEBUGGER gm_log("registering new UI element with id "$(elem.id)" to the UI element manager.");
            this->elements[ elem.id ] = elem;
        };

        /// updates a UI element by rewriting the cache at the specified ID.
        inline void update_element( ui_element& elem ) {
            if ( !this->cache.contains( elem.id ) )
                gm_err(0, "UPDATE_UI_ELEMENT for "$(elem.id)" not possible: the element does not exist in the cache.");

            this->cache.at( elem.id ) = format_element( elem );
        };
    };


    /// Camera Instance of any viewport in the game engine.
    /// All displays are rendered through this class.
	template<std::size_t _w = 1, std::size_t _h = 1>
	class Camera { /// v8.2.1: Camera, v4.1.0: CameraView, v1.0.0: WorldMap
      public:
        /// width of the Camera.
        /// can be set by template parameters with Camera<5, 5>.
        /// or can be set with Camera<0, 0>.SetResolution(5, 5);
        std::size_t w = _w;
        /// height of the Camera.
        /// can be set by template parameters with Camera<5, 5>.
        /// or can be set with Camera<0, 0>.SetResolution(5, 5);
        std::size_t h = _h;
        // frame time, calculated internally
        // time in milliseconds for frame generation
        float frame_time = 0;
        // draw time, currently not internally calculated
        // time in milliseconds for frame drawing (to the output)
        float draw_time = 0;
        // entity count, set by external Level controller class.
        // will be 0 if used without external controller classes.
        uint32_t entity_count;
        // model count, set by external Level controller class.
        // will be 0 if used without external controller classes.
        uint32_t model_count;

        // all modifiers for the camera
        ModifierList modifiers = {
			{
                /// noclip modifier, disables all collision checking
                /// currently unused.
				modifier { .name="noclip",                  .value=0 },
                /// force_update disallows draw() from just writing the
                /// output in raw_unit_map, forcing a call to update() first.
				modifier { .name="force_update",            .value=0 },
                /// should be moved to the Game Event Loop.
                /// controls plugin usage.
				modifier { .name="allow_plugins",           .value=1 },
                /// deprecated, casts events through std::cerr
                /// used by old typescript eventloop.
				modifier { .name="cast_events",             .value=1 },
                /// for deprecated logging methods
                /// unused
				modifier { .name="allow_writing_plog",      .value=1 },
                /// cubic render, squishes units into squares [|] instead of
                /// writing them as 1x2 full unit characters [I].
                modifier { .name="cubic_render",            .value=1 }, // enabled since v8.2.2-d
			    /// debug render mode, enables many different rendering
                /// options, most notably model border rendering.
                modifier { .name="debug_render",            .value=1 }, // since v10.4.0-d
                /// wireframe render mode, draws wireframes on units with collision
                /// info. X for no collision, x for collision.
                modifier { .name="wireframe_render",        .value=0 },
                /// enable/disable lighting and illumination
                /// used by renderscale functions
                modifier { .name="lighting",                .value=0 },
                /// minimum light intensity for the camera, default 5
                /// increasing this brightens the entire camera view.
                modifier { .name="lighting_floor",          .value=5 },
                /// UI element controller. enables/disables HUD and other
                /// UI elements from being drawn.
                modifier { .name="allow_ui",               .value=1 },
                /// Whether FIXED_UPDATE event calls follow UPDATE event calls.
                /// FIXED_UPDATE will be called every frame when enabled.
                /// This can be disabled if in-game events only happen upon
                /// input from the player. Usually only in singleplayer games.
                modifier { .name="update_all_frames",      .value=1 },
                /// Whether the orange outline of the game view should
                /// be drawn. Applies to consoleview only.
                modifier { .name="draw_outline",           .value=1 },
                /// efficient_render mode. Various optimizations that make rendering
                /// chunks faster, but can sometimes cause issues with external game code.
                /// However, since these are almost completely nonexistent, we can get away
                /// with enabling efficient_render by default. For any edge-case that requires
                /// it to be disabled, this option is dynamic and can be disabled.
                modifier { .name="efficient_render",       .value=1 },
                /// kitty_graphics mode. This will enable using the Kitty Terminal Graphics
                /// Protocol to draw raw framebuffers to the terminal screen instantly.
                /// This is (1) faster, (2) better looking, and (3) really cool.
                /// Enabling this will not immediately cause Gmeng to use it, the value of
                /// `Gmeng::global::terminal_has_kgp_pixel_support` must be `true`.
                modifier { .name="kitty_graphics",         .value=1 },
                /// Enables the ImGui_ImplGmeng wrapper for ImGui which supports the
                /// Kitty Graphics Protocol to add ImGui to the term_graphics_renderer.
                modifier { .name="imgui", .value=1 },
            }
		};

        /// UI manager of the camera. Handles all UI elements
        /// including clickables and textblobs.
        Camera_UI_Manager ui_manager;
        /// Display map, contains the current
        /// screen data in units
		DisplayMap<_w, _h> display_map;
        /// Terminal Graphics Manager. Handles the Kitty Terminal
        /// Graphics Protocol and raw framebuffer present() methods.
        term_graphics_renderer renderer;
        // raw unit map, contains the rendered units
        // to be written to the console output
        std::vector<std::string> raw_unit_map = vector<std::string>( GMENG_MAX_MAP_SIZE );
        /// override map, for overriding specific units
        /// used by UI functions.
        std::map<int, std::string> overrides;
        /// extra drawlist, for adding render blobs to the
        /// camera with single-frame instances.
        std::vector< std::tuple< Objects::coord, Gmeng::Blob > > frame_draw_list;
        /// after-render drawlist, for adding UI elements
        std::vector< std::tuple< Objects::coord, Gmeng::Blob > > after_draw_list;

        GMENG_INIT_TYPE(w, h, frame_time, draw_time,
                              entity_count, model_count,
                              modifiers, ui_manager,
                              display_map, raw_unit_map,
                              overrides, frame_draw_list );

        /// calculates light illumination for a given RGB color
        /// and returns the modified color to be used instead
        LightingState calculate_illumination(
            const color32_t& color,
            const color32_t& light_color,
            int light_intensity_max,
            int light_distance,
            int max_brightness,
            int min_intensity
        ) {
            LightingState result;
            result.surface_color = color;
            result.light_color = light_color;

            // Calculate current light intensity (1-10 scale)
            int intensity = std::min(max_brightness, light_intensity_max - light_distance);

            if (intensity < min_intensity) {
                intensity = min_intensity;

                result.brightness = intensity;

                // Don't tint, just use neutral white light
                result.r_factor = 1.0f;
                result.g_factor = 1.0f;
                result.b_factor = 1.0f;
            } else {
                // Tint the color by the light source color
                result.r_factor = light_color.r / 255.0f;
                result.g_factor = light_color.g / 255.0f;
                result.b_factor = light_color.b / 255.0f;

                result.brightness = intensity;
            }

            // Normalize intensity to 0.0–1.0 scale based on max of 10
            result.modulation_factor = static_cast<float>(intensity) / 10.0f;

            return result;
        }




        /// Sets the resolution of the camera.
		inline void SetResolution(std::size_t w, std::size_t h) {
            __functree_call__(Gmeng::Camera::SetResolution);
			display_map.__h = h; display_map.__w = w;
			this->w = w; this->h = h;

            this->renderer.width = w;
            this->renderer.height = h;
		};
        /// Updates the raw_unit_map to match the display_map's
        /// units. This means rendering the camera's viewport fully.
		[[deprecated("update() is deprecated")]] inline void update() {
            __functree_call__(Gmeng::Camera::update);
            auto time = GET_TIME();
			for (int i = 0; i < this->h; i++) {
				for (int j = 0; j < this->w; j++) {
					if (i*j == this->h*this->w) break;
					int current_unit_id = (i * this->w) + j;
					Gmeng::Unit current_unit = this->display_map.unitmap[current_unit_id];
					std::string final = this->draw_unit(current_unit);
					this->raw_unit_map[current_unit_id] = final;
				};
			};
            auto time_fin = GET_TIME() - time;
            this->frame_time = time_fin;
		};

        /// Temporarily sets the Unit in a position only on the raw_unit_map.
        /// This means that in the next rendered frame including this drawpoint
        /// the unit will revert to its original state.
        inline void temp_displacement(int __pX, int __pY, Gmeng::Unit u) {
            __functree_call__(Gmeng::Camera::temp_displacement);
            this->set_curXY(__pX, __pY);
            int pos_in_map = (__pY * this->w) + __pX;
            this->raw_unit_map[pos_in_map] = this->draw_unit(u);
            this->rewrite_mapping({ pos_in_map });
        };

        /// Draws the raw_unit_map as a string (to be written to a terminal output stream)
        /// This method also appends an outline to the camera.
        /// Cubic rendering is accounted for automatically with this method.
        ///
        /// TODO: 13.0.0: make this function draw the screen in real time instead of
        /// appending to a string and drawing the final output. Will hopefully increase
        /// frame-rate in the console.
		inline std::string draw() {
            auto time = GET_TIME();

            bool draw_outline = this->has_modifier("draw_outline");
            ///this->clear_screen();
            ///   disabled since v6.0.0: since draw() does not output anything,
            ///   it should not interfere with the screen.
            std::string final = "";

            /// Reserve an estimated size of the string, the value
            /// is probably much higher than this, and calculating
            /// the value more precisely is also more costly, so
            /// a middle ground is needed. Reserving space is more
            /// efficient than appending new data constantly.
            final.reserve( this->w * this->h * 8 * sizeof(char) );

            /// when cubic render is on, in case the height is not even,
            /// extend the height by 1 and fill with void.
            int cubic_height = (this->h % 2 == 0) ? (this->h/2) : (this->h/2)+1;
            /// actual character size of the output frame.
            /// This will be equal to width*height if cubic_render is not enabled,
            /// and width * (height / 2) if cubic_render is enabled ( since 8.2.1, it is. )
            int cc = ( this->has_modifier("cubic_render") ) ? ( this->w*(cubic_height) ) : ( this->w*this->h );
            /// handle overrides
            /// loop through the raw unit map and draw the final image
			for (int i = 0; i < (cc); i++) {
				if (i % this->w == 0) {
                    /// appends the frame's left and right outline to the output.
					if (i > 1) final += (draw_outline ? "\x1B[38;2;246;128;25m" : ""),  final += (draw_outline ? Gmeng::c_unit : "\n");
					final += (draw_outline ? "\n\x1B[38;2;246;128;25m" : ""); final += (draw_outline ? Gmeng::c_unit : "");
				};
                /// if the unit is empty, make it a void pixel. should not happen though.
                final +=
                  this->raw_unit_map[i].empty() ?
                      colors[BLACK] + Gmeng::c_unit + colors[WHITE] :
                      this->raw_unit_map[i];
			};
            /// Top of the outline's frame. ANSI escape code for orange.
			std::string __cu = "\x1B[38;2;246;128;25m";
            /// Bottom of the outline's frame. ANSI escape code for orange.
			std::string __cf = "\x1B[38;2;246;128;25m";
            /// match the size of the camera
			if (draw_outline) for (int i = 0; i < this->w+2; i++) { __cu += Gmeng::c_outer_unit; __cf += Gmeng::c_outer_unit_floor; };
			/// append the outline
            if (draw_outline) {
                final += ("\x1B[38;2;246;128;25m"); final += (Gmeng::c_unit);
			    final = __cu + "" + final + "\n" + __cf;
            };
            auto time_fin = GET_TIME() - time;
            this->draw_time = time_fin;
			return final;
		};

        /// sets the cursor's position (accounted for the frame)
		inline void set_curXY(int x, int y) {
            //__functree_call__(Gmeng::Camera::set_curXY);
   			// this functree call caused so much logging that VIM would crash the OS when opening it.
            // so it is disabled now.

            std::cout << "\033[" << x+2 << ";" << y+2 << "H"; return; // extra numbers account for the border around the map.
		};

        /// Writes all UI elements to the camera view.
        /// Use only after a frame has been generated.
        ///
        /// Gmeng::Renderer::Display::refresh() automatically
        /// handles all required draws by default. This method
        /// should only be called by the user if you are using
        /// a custom screen refresh handler.
        inline void apply_ui() {
            /// if allow_ui isn't enabled ( allow_ui != 1 ),
            /// don't apply any UI elements to the screen.
            if ( !this->has_modifier("allow_ui") ) return;
            std::vector<int> to_erase;
            /// loop through all UI elements getting their id and element object
            for ( auto [ e_id, e ] : this->ui_manager.elements ) {
                /// skip inactive elements and erase them from
                /// cache and element maps. Elements can't be
                /// reactivated or modified so this is fine.
                if ( GET_TIME() - e.created_at > e.expire_after ) {
                    /// reserve for deletion after loop
                    to_erase.push_back( e_id );
                    /// continue drawing other items.
                    continue;
                };

                /// final formatted element
                Camera_UI_Manager::formatted_element elem = this->ui_manager.format_element( e );


                /// true position of the element, accords to cubic_render if it is enabled
                Objects::coord pos = { elem.position.x, (elem.position.y / ( this->has_modifier("cubic_render") ? 2 : 1) ) };

                /// get the final text-based content of the element
                auto text = ui_manager.get_final( elem );

                /// Check the string length and split it if it's longer than the width of the frame.
                auto Stext = strip_ansi( text );
                unsigned int len_div = Stext.length() / (unsigned int)this->w;
                if ( len_div > 0 ) {
                    std::vector<int> spots;
                    int diff = text.length() / len_div;
                    for (int i = 1; i <= len_div; i++) {
                        spots.push_back( diff*i-1 );
                    };
                    for ( auto sp : spots ) {
                        std::size_t logical_index = logical_to_physical_index(text, sp);
                        if ( logical_index != std::string::npos )
                            text.insert( text.begin()+logical_index, '\n' );
                    };
                };


                /// newlines will be split, so this takes that into account.
                /// counts the amount of lines in the final text content
                int line = 0;
                /// split the string into individual lines
                auto spl = g_splitStr(text, "\n");
                /// loop through the lines
                for ( auto _l : spl ) {
                    auto l = BCOLOR32_TO_ASCII( elem.base.color_bg ) + FCOLOR32_TO_ASCII( elem.base.color_fg ) + _l + Gmeng::resetcolor;
                    /// line position
                    int pos_x = pos.x; int pos_y = (elem.position.y / ( this->modifiers.get_value("cubic_render") ==1 ? 2 : 1 ))+line;

                    auto split_l = split_with_ansi( l );
                    int ab = 0;
                    for ( auto s_ch : split_l ) {
                        this->raw_unit_map[ pos_y*this->w + elem.position.x + ab ] = s_ch;
                        ab++;
                    };
                    line++;
                };
            };
            for ( auto elem : to_erase ) {
                this->ui_manager.elements.erase( elem );
            };
        };

        /// Adds a Blob render obect to the draw list. This method can
        /// be used to create basic shapes such as rectangles and circles
        /// that should not be treated as models or entities. Particularly
        /// useful for items like waypoints, objective markers, selection
        /// highlights and other items that are temporary.
        ///
        /// Gmeng's Easy Header ( `lib/bin/easy.h` ) provides basic
        /// geometric shape constructors for rectangles and circles.
        inline void add_draw( Objects::coord position, const Gmeng::Blob& blob ) {
            this->frame_draw_list.push_back( { position, blob } );
        };

        /// Adds a Blob render obect to the after-render draw list.
        /// This method can be used to draw custom UI elements.
        ///
        /// Gmeng's Easy Header ( `lib/bin/easy.h` ) provides basic
        /// geometric shape constructors for rectangles and circles.
        inline void add_draw_after( const Objects::coord& position, const Gmeng::Blob& blob ) {
            this->after_draw_list.push_back( { position, blob } );
        };

        /// Adds a string to the drawlist. The text is split into each
        /// character and converted into a `Gmeng::Blob` via the font
        /// loaded with `gmeng_terminal_font_init( font_file )`.
        ///
        /// Gmeng's Text Header ( `utils/text.h` ) provides utilities
        /// for terminal-based fonts and texts. See `CHARACTER_FONT_TEXTURES`.
        /// You can add or set your custom font if you prefer. However, they
        /// are in a custom format. The default-provided font is at `assets/term-font`.
        /// See the directory's contents to understand how it is stored.
        /// Each character is a different gmeng texture object (Blob).
        inline void add_text( Objects::coord position, const std::string& text, uint32_t color = WHITE ) {
            /// offset from the original position. Every character writes to this.
            int x_offset_by = 0,
                y_offset_by = 0;
            /// the Y offset of the current line. Equal to the height of the current
            /// line. The current line is whatever the line is after the last \n.
            int current_line_y_offset = 0;
            for ( const char& chr : text ) {
                /// Newline, reset the X offset while incrementing the Y offset.
                if ( chr == '\n' ) {
                    y_offset_by += current_line_y_offset + 1;
                    x_offset_by  = current_line_y_offset = 0;

                    continue;
                };
                /// Get the font blob of the character to add to the draw list.
                /// This will throw a std::out_of_range exception if it encounters
                /// a character that isn't in the English latin script or punctuation.
                Gmeng::Blob& char_blob = gmeng_terminal_font_get_char( chr );
                char_blob.recolor( color );
                /// Add the character blob to the drawlist.
                this->add_draw({
                    position.x + x_offset_by,
                    position.y + y_offset_by
                }, char_blob);
                /// increase the X offset by the width of the character's blob
                /// width and add 1 unit of space between the characters.
                x_offset_by += char_blob.width + 1;

                if ( current_line_y_offset < char_blob.height )
                     current_line_y_offset = char_blob.height;
            };
        };

        /// checks for internal camera modifiers
		inline constexpr bool has_modifier(std::string_view name) {
            //__functree_call__(Gmeng::Camera::update_modifier);
            return modifiers.get_value(name) == 1;
        };

        /// updates internal camera modifiers
		[[deprecated]] inline void update_modifier(Gmeng::modifier& modifier, int value) {
            modifier.value = value;
        };

        /// sets internal camera modifiers
		[[deprecated]] inline void set_modifier(std::string_view name, int value) {
            this->modifiers.set_value( name, value );
		};


        /// resets the cursor to the 0,0 position of the terminal
		inline constexpr void reset_cur() {
            //__functree_call__(Gmeng::Camera::reset_cur);
			this->set_curXY(-2, -2);
		};

        /// returns the 1D vector index of a unit
        /// as a 2D coordinate.
        ///
        /// eg.
        /// ```
        /// { 0, 1, 2, 3, 4,
        ///   5, 6, 7, 8, 9 }
        /// camera.width = 5
        /// get_xy(7) = { .y=1, .x=3 };
        /// ```
		inline Objects::coord get_xy(int __p1) {
            __functree_call__(Gmeng::Camera::get_xy);
			int __p1_x = __p1 / this->w;
			int __p1_y = __p1 % this->w;
			return { .x=__p1_x,.y=__p1_y };
		};

        /// draws some infographics to the screen.
        /// like the F3 menu in the minecraft engine.
        ///
        /// TERMINAL-MODE only.
        inline void draw_info(int x = 0, int y = 0) {
            std::string vb_text = "~r~gmeng ~y~" + Gmeng::version + "~n~ - build id: ~b~" + GMENG_BUILD_NO + "~n~";
            std::string vb_text_formatted = colorformat(vb_text);
            int length = strip_ansi( vb_text_formatted ).length();

            ui_element version_build_text(1, vb_text, { (int)this->w - length, 0 });
            this->ui_manager.add_element( version_build_text );


            int fps = 1000 / draw_time;
            if (fps == 0 || draw_time == 0) fps = 1000;
            std::string frames_text = "frame: ~g~" + v_str(frame_time) + "ms~n~ | draw: ~y~" + v_str(draw_time) + "ms~n~ | fps: ~b~" + v_str(fps);
            int frames_length = strip_ansi( colorformat(frames_text) ).length();
            if (frames_length < length) frames_text += repeatString(" ", length - frames_length);
            ui_element frames(1, frames_text, { (int)this->w - length, 2 });
            this->ui_manager.add_element( frames );

            std::string vps = "viewport_size: ~p~"$(this->w)"x"$(this->h)"~n~ | map: ~y~"$(this->display_map.__w * this->display_map.__h)"";
            if (strip_ansi( colorformat(vps) ).length() < length) vps += repeatString(" ", length - strip_ansi( colorformat(vps) ).length());


            std::string emtotal = "entities: ~b~"$(this->entity_count)"~n~ | models: ~c~"$(this->model_count)"~n~ | all: ~g~" $(this->entity_count + this->model_count) "~n~";
            if (strip_ansi( colorformat(emtotal) ).length() < length) emtotal += repeatString(" ", length - strip_ansi( colorformat(emtotal) ).length());


            ui_element ems(1, emtotal, { (int)this->w - length, 4 });
            this->ui_manager.add_element( ems );


            ui_element vpsize(1, vps, { (int)this->w - length, 6 });
            this->ui_manager.add_element( vpsize );



/**
            /// __functree_call__(Gmeng::Camera::draw_info);
            this->set_curXY(y,x);
            std::cout << Gmeng::resetcolor;
			WRITE_PARSED("[ ~r~gmeng ~y~$!__VERSION~n~ - build ~b~$!__BUILD~n~ ]   ");

            this->set_curXY(y+1,x);            WRITE_PARSED("[ frame_time: ~g~"$(this->frame_time)"ms~n~, draw_time: ~y~"$(this->draw_time)"ms~n~ ]   ");
			this->set_curXY(y+2,x);            WRITE_PARSED("[ viewport_size: ~p~"$(this->w)"x"$(this->h)"~n~ ]   ");
            this->set_curXY(y+3, x);           WRITE_PARSED("[ entities: ~b~"$(this->entity_count)"~n~, models: ~c~"$(this->model_count)"~n~ ]   ");
            this->set_curXY( y+4, x );         WRITE_PARSED("~_~" + repeatString("-", 40) + "~n~");
            this->set_curXY( y+5, x );         WRITE_PARSED("modifiers:");
            this->set_curXY( y+6, x );

#define nmb2(x) (x == 1 ? "~g~" + v_str(x) + "~n~" : "~y~" + v_str(x) + "~n~")
#define nmb_(x) (x < 0 ? "~r~" + v_str(x)+"~n~" : nmb2(x))
#define nmb(x) (x == 0 ? "~p~" + v_str(x) + "~n~" : nmb_(x))

            WRITE_PARSED("cubic_render: " + nmb(
                this->modifiers.get_value("cubic_render")
            ) + " | debug_render: " + nmb(
                this->modifiers.get_value("debug_render")
            ) + "   ");
            this->set_curXY( y+7, x );
            WRITE_PARSED("noclip: " + nmb(
                this->modifiers.get_value("noclip")
            ) + " | wireframe_render: " + nmb(
                this->modifiers.get_value("wireframe_render")
            ) + "   ");

            this->set_curXY( y+8, x );         WRITE_PARSED("~_~" + repeatString("-", 40) + "~n~");**/
        };

        /// draws a 'Unit' object and returns it as a printable string
		inline std::string draw_unit(Gmeng::Unit __u, Gmeng::Unit __nu = Unit { .is_entity=1 }, bool prefer_second = false) {
			Gmeng::Unit current_unit = __u;
            // check if cubic render is preferred, and a next unit is provided
            bool nu = this->has_modifier("cubic_render") && !__nu.is_entity;
            bool use_resetcolor = this->modifiers.get_value("use_resetcolor") != 0;
            Gmeng::Unit next_unit = __nu;
            // by default, colors are transparent (void/black)
            /// 12.0.0: add RGB support
            color32_t u1_color_rgb = color32_t( current_unit.color );
            color32_t u2_color_rgb = color32_t( nu ? next_unit.color : BLACK );

            char u1_colorcode[32], u2_colorcode[32];

            TERM_RGB_FG( u1_colorcode, u1_color_rgb.r, u1_color_rgb.g, u1_color_rgb.b ),
            TERM_RGB_BG( u2_colorcode, u2_color_rgb.r, u2_color_rgb.g, u2_color_rgb.b );

            /*if (current_unit.color == next_unit.color) {
                if (current_unit.color == WHITE) return colors[WHITE] + Gmeng::c_unit + Gmeng::resetcolor;
                return bgcolors_bright[current_unit.color] + " " + Gmeng::resetcolor;
            };*/ // v8.2.2-d: this expects the units to be in Y-index ordered
            if (current_unit.color == next_unit.color) {
                if (this->has_modifier("wireframe_render")) {
                    if (prefer_second) return u2_colorcode + colors[BLACK] +
                        ((current_unit.collidable || this->has_modifier("noclip") ? "x" : "X")) + ( use_resetcolor ? Gmeng::resetcolor : "" );
                    else return u1_colorcode + colors[BLACK] +
                        ((next_unit.collidable || this->has_modifier("noclip") ? "x" : "X")) + ( use_resetcolor ? Gmeng::resetcolor : "" );
                };
                return u1_colorcode + std::string( c_unit );
            };

            /// Draws the 'special' Text-type Units,
            /// unless the second unit (__nu) is preferred as the
            /// main entity on the function call.
            if (current_unit.special && !prefer_second) {
                return u1_colorcode + colors[current_unit.special_clr] + current_unit.special_c_unit +  (use_resetcolor ? Gmeng::resetcolor : "");
            } else if (nu && next_unit.special && ( !current_unit.special || prefer_second )) {
                return u2_colorcode + colors[next_unit.special_clr] + next_unit.special_c_unit +  (use_resetcolor ? Gmeng::resetcolor : "");
            };


            if (this->has_modifier("wireframe_render")) {
                char u2_fg[32], u1_bg[32];
                TERM_RGB_FG(u2_fg, u2_color_rgb.r, u2_color_rgb.g, u2_color_rgb.b);
                TERM_RGB_BG(u1_bg, u1_color_rgb.r, u1_color_rgb.g, u1_color_rgb.b);
                std::string final = std::string(u1_bg) +
                    (nu ? u2_fg + std::string(u1_bg) : u1_bg) + (current_unit.collidable || this->has_modifier("noclip") ? "x" : "X") + (use_resetcolor ? Gmeng::resetcolor : "");
                return final;
            };

			std::string final = ( nu ?
                    ( std::string(u2_colorcode) + u1_colorcode + Gmeng::c_outer_unit_floor ) :
                                                     ( std::string(u1_colorcode) + Gmeng::c_unit ) ) + (use_resetcolor ? Gmeng::resetcolor : "");
			return final;
		};

        /// rewrites the position given as parameter to the function
        /// from the raw_unit_map.
        ///
        /// this is useful for changes for only a single frame like
        /// a flashing or rapidly color changing unit because this
        /// method allows updating the screen without updating the
        /// entire screen buffer.
		inline void rewrite_mapping(const std::vector<int>& positions) {
            __functree_call__(Gmeng::Camera::rewrite_mapping);
			for (std::size_t i=0;i<positions.size();i++) {
				int curid = positions[i];
				Objects::coord cpos = this->get_xy(curid);
				this->set_curXY(cpos.x, cpos.y);
				std::cout << this->raw_unit_map[curid];
			};
		};

        /// clears the terminal screen buffer
		inline void clear_screen() {
            __functree_call__(Gmeng::Camera::clear_screen);
			std::cout << "\033[2J\033[1;1H";
		};

        /// @deprecated
		[[deprecated]] inline void rewrite_full() {
            __functree_call__(Gmeng::Camera::rewrite_full);
			this->clear_screen();
			this->update();
            std::cout << repeatString("\n", 20) << std::endl;
			std::cout << this->draw() << std::endl;
		};

	};

};

#ifndef GMENG_NO_SOL

#include "../utils/scripts.h"
static int __gmeng_define_lua_camera_method = ([]() -> int {
    using namespace Gmeng;
    using namespace Scripts;

    gmeng_lua_namespace.new_usertype<Camera<0, 0>>("Camera",
        "width", &Camera<0, 0>::w,
        "height", &Camera<0, 0>::h,
        "frame_time", &Camera<0, 0>::frame_time,
        "draw_time", &Camera<0, 0>::draw_time,
        "entity_count", &Camera<0, 0>::entity_count,
        "model_count", &Camera<0, 0>::model_count,
        "modifiers", &Camera<0, 0>::modifiers,
        "ui_manager", &Camera<0, 0>::ui_manager,
        "display_map", &Camera<0, 0>::display_map,
        "renderer", &Camera<0, 0>::renderer,
        "raw_unit_map", &Camera<0, 0>::raw_unit_map,
        "frame_draw_list", &Camera<0, 0>::frame_draw_list,
        "after_draw_list", &Camera<0, 0>::after_draw_list
    );
    return 0;
})();
#endif

