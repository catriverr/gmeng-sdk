#pragma once
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <set>
#include <string>
#include <array>
#include <cstring>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>
#include <algorithm>

#include "../../../include/noble/lib/scripts/arch.cc" // NOBLE: No Bad Language Esoterics (https://github.com/catriverr/noble)
                                                      // used for .dylib prebuilt C++ script handling.
#include "../gmeng.h"
#include "../utils/gridmap.cc"

/// MAX MAP SIZE
/// can be set in the makefile
/// version 10.1.1 will add this option to `make configure`
#ifndef GMENG_MAX_MAP_SIZE
    #define GMENG_MAX_MAP_SIZE 32767
#endif

/// in-text number variable input like `"hi user"$(id)""`
#define $(x) + v_str(x) +

#ifdef _WIN32
#include <windows.h> // at some point it will be important
#include <conio.h>  // For _getch() on Windows
#else
#include <termios.h>
#include <unistd.h>
#endif

#define RGB_ASCII_CODE_FG(r, g, b) std::string("\x1b[38;2;"$(r)";"$(g)";"$(b)"m")
#define RGB_ASCII_CODE_BG(r, g, b) std::string("\x1b[48;2;"$(r)";"$(g)";"$(b)"m")
#define BCOLOR32_TO_ASCII( col32_obj ) RGB_ASCII_CODE_BG( col32_obj.r, col32_obj.g, col32_obj.b )
#define FCOLOR32_TO_ASCII( col32_obj ) RGB_ASCII_CODE_FG( col32_obj.r, col32_obj.g, col32_obj.b )

/// sets the current terminal foreground color to the RGB value
#define TERM_RGB_FG(buf, r, g, b) std::snprintf(buf, 32, "\x1b[38;2;%d;%d;%dm", (r), (g), (b))
/// sets the current terminal background color to the RGB value
#define TERM_RGB_BG(buf, r, g, b) std::snprintf(buf, 32, "\x1b[48;2;%d;%d;%dm", (r), (g), (b))
/// resets the terminal color with ansi escape code
#define TERM_RGB_RESET "\x1b[0m"

#if GMENG_SDL_INIT == true
    // ignore coc-nvim errors, this compiles
    #include "../utils/window.cpp"
#endif


class GmengImGuiTextureEditor {
  public:
    std::string current_filename;
    bool loaded = false;
    Gmeng::texture tex;
    Gmeng::Unit current_brush;
    float brush_color_float[3] = { 1.0f, 1.0f, 1.0f }; // For ImGui ColorEdit3
    std::set<uint32_t> texture_palette;

    float zoom = 57.2f;
    bool is_open = true;

    GmengImGuiTextureEditor(const std::string& default_filename = "custom.gt")
        : current_filename(default_filename)
    {
        // Initialize an empty texture to prevent rendering a 0x0 grid
        tex.name = v_str(g_mkid());
        ResizeTexture(8, 8);
    }
    void ResizeTexture(std::size_t, std::size_t);
    void Save(const std::string& fname);
    void Load(const std::string& fname);
    void Draw();
};



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

namespace Gmeng {

    /// v10.1.0
    struct Positioned_Unit {
        Unit unit;
        Objects::coord position;
    };
    /// v10.1.0
    struct Overlay {
        std::vector<Positioned_Unit> units;
    };

    /// todo: documentation
    struct LightingState {
        color32_t surface_color;
        color32_t light_color;
        float r_factor;
        float g_factor;
        float b_factor;
        float modulation_factor;

        int brightness;
    };

    color32_t apply_lighting(const LightingState& state) {
        auto clamp = [](int v) { return std::clamp(v, 0, 255); };
        DEBUGGER gm_log("random_ass_color("$( uint32_from_color32(state.surface_color) )"): "$(state.r_factor)", "$(state.g_factor)","$(state.b_factor)" modf: "$(state.modulation_factor)".");
        return {
            static_cast<uint8_t>(clamp(static_cast<int>(state.surface_color.r * state.r_factor * state.modulation_factor))),
            static_cast<uint8_t>(clamp(static_cast<int>(state.surface_color.g * state.g_factor * state.modulation_factor))),
            static_cast<uint8_t>(clamp(static_cast<int>(state.surface_color.b * state.b_factor * state.modulation_factor)))
        };
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
        ui_element( unsigned int expire_after, std::string text, color32_t col_bg = (uint32_t)BLACK, color32_t col_fg = (uint32_t)WHITE ) :
        active( true ), expire_after( expire_after ), text( text ), color_bg( col_bg ), color_fg( col_fg ) {
            gm_log("UI element created with ID " + v_str(id) + ". Expiration is t+"$(expire_after)"ms.");
            this->created_at = GET_TIME();
            if (expire_after != 0) create_thread([&]() {
                std::this_thread::sleep_for( std::chrono::milliseconds( this->expire_after ) );
                this->active = false;
            });
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
            std::map<unsigned int, ui_clickable> clickables;
            std::vector<formatted_text_obj> text_formatted;
            Objects::coord position;
        };

        std::map<int, ui_element> elements;
        std::map<int, formatted_element> cache;


        #include "../strings/trim.cc"
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

        formatted_element format_element(ui_element element_full) {
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

            final.position = final.base.position;
            final.base = element_full;
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
            __functree_call__(Gmeng::Camera_UI_Manager::add_element);
            gm_log("registering new UI element with id "$(elem.id)" to the UI element manager.");
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
        uint32_t frame_time = 0;
        // draw time, currently not internally calculated
        // time in milliseconds for frame drawing (to the output)
        uint32_t draw_time = 0;
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
            }
		};

        /// UI manager of the camera. Handles all UI elements
        /// including clickables and textblobs.
        Camera_UI_Manager ui_manager;
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

        // Display map, contains the current
        // screen data in units
		DisplayMap<_w, _h> display_map;
        // raw unit map, contains the rendered units
        // to be written to the console output
		std::string raw_unit_map[GMENG_MAX_MAP_SIZE];
        /// override map, for overriding specific units
        /// used by UI functions.
        std::map<int, std::string> overrides;

        /// Sets the resolution of the camera.
		inline void SetResolution(std::size_t w, std::size_t h) {
            __functree_call__(Gmeng::Camera::SetResolution);
			display_map.__h = h; display_map.__w = w;
			this->w = w; this->h = h;
		};
        /// Updates the raw_unit_map to match the display_map's
        /// units. This means rendering the camera's viewport
        /// fully. This method is not deprecated, but it is
        /// not being used in the latest framework for gmeng.
		inline void update() {
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
            __functree_call__(Gmeng::Camera::draw);
            if (Gmeng::global.dont_hold_back && !Gmeng::global.shush) {
                gm_log("Gmeng::Camera job_render *draw -> total drawpoints available at this->cam::vp_mem0: " + v_str(sizeof(this->raw_unit_map)) + " , v_addr " + _uconv_1ihx(0) + " -> " + _uconv_1ihx(sizeof(this->raw_unit_map)));
		    	gm_log("Gmeng::Camera job_render *draw -> total drawpoints allocated for job_render at this->cam::vp_mem0: " + v_str(this->w*this->h) + " | " + _uconv_1ihx(this->w*this->h));
                gm_log("Gmeng::Camera job_render *draw -> resolution: " + v_str(this->w) + "x" + v_str(this->h));
            };
            ///this->clear_screen(); // disabled since v6.0.0: since draw() does not output anything, it should not interfere with the screen.
            std::string final = "";
            /// when cubic render is on, in case the height is not even, extend the height by 1 and fill with void.
            int cubic_height = (this->h % 2 == 0) ? (this->h/2) : (this->h/2)+1;
            /// actual character size of the output frame.
            int cc = ( this->has_modifier("cubic_render") ) ? ( this->w*(cubic_height) ) : ( this->w*this->h );
            /// handle overrides
            /// loop through the raw unit map and draw the final image
			for (int i = 0; i < (cc); i++) {
				if (i % this->w == 0) {
                    /// appends the frame's left and right outline to the output.
                    if (global.debugger) gm_slog(YELLOW, "DEBUGGER", "append_newline__" + v_str( (int)(i / cubic_height) ));
					if (i > 1) final += "\x1B[38;2;246;128;25m",  final += Gmeng::c_unit;
					final += "\n\x1B[38;2;246;128;25m"; final += Gmeng::c_unit;
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
			for (int i = 0; i < this->w+2; i++) { __cu += Gmeng::c_outer_unit; __cf += Gmeng::c_outer_unit_floor; };
			/// append the outline
            final += ("\x1B[38;2;246;128;25m"); final += (Gmeng::c_unit);
			final = __cu + "" + final + "\n" + __cf;
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
        inline void apply_ui() {
            std::vector<int> to_erase;
            /// loop through all UI elements getting their id and element object
            for ( auto [ e_id, e ] : this->ui_manager.elements ) {
                /// skip inactive elements and erase them from
                /// cache and element maps. Elements can't be
                /// reactivated or modified so this is fine.
                if ( !e.active ) {
                    /// reserve for deletion after loop
                    to_erase.push_back( e_id );
                    /// continue drawing other items.
                    continue;
                };

                /// final formatted element
                Camera_UI_Manager::formatted_element elem;

                /// check if the cache contains this element
                /// Since elements can't be modified we can just
                /// take the cached state
                if ( !this->ui_manager.cache.contains( e_id ) ) {
                    gm_log("UI element with id "$(e_id)" is not registered to element cache. registering...");
                    auto rem = this->ui_manager.format_element( e );

                    this->ui_manager.cache[e_id] = elem = rem;

                    /// since this is the first caching of the element,
                    /// We can begin the countdown for the expiration of it
                    /// now that we are drawing it unless it's indefinite ( expiration = 0 )
                    if (e.expire_after != 0) create_thread([this, e_id, e]() {
                        std::this_thread::sleep_for( std::chrono::milliseconds( e.expire_after ) );
                        this->ui_manager.elements.at( e_id ).active = false;
                    });
                } else elem = this->ui_manager.cache.at(e_id);


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
                    for ( auto sp : spots )
                        text.insert( text.begin()+logical_to_physical_index(text, sp), '\n' );
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
                    int pos_x = pos.x, pos_y = pos.y+line;

                    auto split_l = split_with_ansi( l );
                    for ( auto s_ch : split_l )
                        this->raw_unit_map[ pos_y*this->w + pos_x++ ] = s_ch;
                    line++;
                };
            };
            /// Erase all UI elements that have expired.
            for ( auto v : to_erase ) {
                this->ui_manager.cache.erase( v );
                this->ui_manager.elements.erase( v );
            };
        };

        /// checks for internal camera modifiers
		inline bool has_modifier(std::string name) {
            //__functree_call__(Gmeng::Camera::update_modifier);
            for (const Gmeng::modifier& modifier : modifiers.values)
                if (modifier.name == name && modifier.value == 1) return true;
            return false;
        };

        /// updates internal camera modifiers
		inline void update_modifier(Gmeng::modifier& modifier, int value) {
            __functree_call__(Gmeng::Camera::update_modifier);
            modifier.value = value;
        };

        /// sets internal camera modifiers
		inline void set_modifier(std::string name, int value) {
            __functree_call__(Gmeng::Camera::set_modifier);

			int vi = g_find_modifier(this->modifiers.values, name);
    			if (vi != -1) this->update_modifier(this->modifiers.values[vi], value);
    			else this->modifiers.values.emplace_back(Gmeng::modifier { .name=name, .value=value });
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
        inline void draw_info(int x = 0, int y = 0) {
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

            this->set_curXY( y+8, x );         WRITE_PARSED("~_~" + repeatString("-", 40) + "~n~");
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
		inline void rewrite_full() {
            __functree_call__(Gmeng::Camera::rewrite_full);
			this->clear_screen();
			this->update();
            std::cout << repeatString("\n", 20) << endl;
			std::cout << this->draw() << std::endl;
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
    static std::thread* thread;
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

#include <unistd.h>
#ifndef _WIN32
#include <termios.h>
#endif
#include <fcntl.h>


#ifndef _WIN32
/// Terminal Utility for raw mode, text drawing, mouse tracking, etc
namespace Gmeng::TerminalUtil {
    void enable_mouse_tracking()  { std::cout << "\033[?1006h\033[?1003h\n" << std::flush; };
    void disable_mouse_tracking() { std::cout << "\033[?1006l\033[?1003l\n" << std::flush; };

    struct termios orig_termios;
    bool already_set = false;

    void cleanup() {
        disable_mouse_tracking();
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    };

    void set_raw_mode(bool state) {
        if (state) {
            std::atexit(cleanup);
            if (already_set) return;
            already_set = true;
            struct termios t;
            tcgetattr(STDIN_FILENO, &t);
            orig_termios = t;
            t.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &t);
        } else {
            already_set = false;
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        };
    }

    // not a function that can be disabled.
    // I tried.
    void set_non_blocking(bool state) {
        if (state) {
            int flags = fcntl(STDIN_FILENO, F_GETFL);
            flags |= O_NONBLOCK;
            fcntl(STDIN_FILENO, F_SETFL, flags);
        } else {
            int flags = fcntl(STDIN_FILENO, F_GETFL);
            flags &= ~O_NONBLOCK;
            fcntl(STDIN_FILENO, F_SETFL, flags);
        };
    };
};
#endif


// Game log
static std::vector<std::string>  PROP_LOGSTREAM = { "gmeng debug & development console.", "> 'help' for commands." };
static std::vector<std::string>* GAME_LOGSTREAM = &PROP_LOGSTREAM;
static std::stringstream GAME_LOGSTREAM_STR;

/// Event Hook System
namespace Gmeng {

    // type decl
    struct EventLoop;
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
        /// SDL-Only
        MOUSE_CLICK_LEFT_END,
        /// SDL-Only
        MOUSE_CLICK_RIGHT_END,
        /// SDL-Only
        MOUSE_CLICK_MIDDLE_END,
        /// All platforms
        MOUSE_CLICK_END_ANY,

        MOUSE_SCROLL_UP,
        MOUSE_SCROLL_DOWN,
        MOUSE_MOVE, /// with 1006 raw input mode, we can capture mouse move events.

        KEYPRESS,

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
        "UNKNOWN"
    };

    /// returns the name of a Gmeng::Event as a string
    const std::string& get_event_name(Event event) {

        if (event < 0 || event >= EVENT_COUNT) {
            static const std::string unknown = "UNKNOWN";
            return unknown;
        };

        return event_names[event];
    };

    /// returns a list of every event in a vector as a string
    string list_events(vector<Event> events) {
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
        int MOUSE_X_POS;
        /// If it is a mouse event, the Y position of the mouse.
        int MOUSE_Y_POS;
        /// SDL-Only, mouse drag relative position X
        int MOUSE_REL_X_POS;
        /// SDL-Only, mouse drag relative position Y
        int MOUSE_REL_Y_POS;

        /// Prevents default behaviour
        /// from executing
        bool prevent_default = false;
        /// Alternative usage.
        /// usually triggered by holding shift.
        /// one example is using shift + scroll for left-right movement instead of usual down-up movement.
        bool alternative = false;
        /// Keypress WITH ctrl keydown
        /// SDL-Only
        bool ctrl = false;
        /// Keypress WITH alt keydown
        /// SDL-Only
        bool alt = false;
        /// optional EventLoop object reference
        EventLoop* event_loop = nullptr;
    } EventInfo;

    EventInfo NO_EVENT_INFO = EventInfo { Gmeng::UNKNOWN, 0, -1, -1, false };
    EventInfo INIT_INFO     = EventInfo { Gmeng::INIT, 0, -1, -1, false };

    /// EventHook handler function type
    using handler_function_type = std::function<void(Gmeng::Level*, EventInfo*)>;
    /// EventHook handler type for event_loop
    typedef struct {
        int id; vector<Event> events;
        handler_function_type handler;
        bool locked;
    } EventHook;

    /// Default Event Hooks & handlers of Gmeng.
    /// Individual hooks defined in this dictionary
    /// can be disabled by external event hooks via
    /// (EventInfo*) info->prevent_default(true).
    vector<EventHook> DEFAULT_EVENT_HOOKS = {};

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
          void init_default_event_hooks() {
            DEFAULT_EVENT_HOOKS.push_back(         EventHook { .id=-100, .events= { MOUSE_CLICK_LEFT_START, MOUSE_CLICK_LEFT_END, MOUSE_CLICK_RIGHT_START, MOUSE_CLICK_RIGHT_END,
                                MOUSE_CLICK_MIDDLE_START, MOUSE_CLICK_MIDDLE_END, MOUSE_CLICK_END_ANY },
                        .handler=[](Gmeng::Level* lvl, EventInfo* info) {

                    } } );
          };
      public:
        int id; Gmeng::Level* level; std::vector<std::string>* logstream = GAME_LOGSTREAM;
                                     std::stringstream* logstream_str = &GAME_LOGSTREAM_STR;
        /// EventLoop Modifiers.
        ModifierList modifiers = { {
            modifier { "allow_console", 1 },
            modifier { "server_passkey", 738867 } /// Should be changed for better password-protection in custom servers.
        } };



        /// Processes, used for registering event calls for the
        /// next tick of the event loop. Called with `UPDATE` event
        vector<EventHook> processes;

        /// Event Hooks of the Loop.
        vector<EventHook> hooks;
        /// Default Event Hooks of the Loop. Can be cancelled by
        /// external hooks to prevent default behaviour.
        vector<EventHook> defaults = Gmeng::DEFAULT_EVENT_HOOKS;

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

        /// EventLoop Constructor, Parses external EventHooks if any is provided.
        EventLoop( vector<EventHook> _hooks = {} ) : hooks( _hooks ), uses_sdl(false) {
            this->init_default_event_hooks();

            this->id = g_mkid();
            gm_log("" $(id) ": created main game eventloop with id " $(this->id) ".");

#ifndef _WIN32
#ifndef GMENG_SDL
            TerminalUtil::enable_mouse_tracking();
            gm_log("" $(id) ": enabled mouse tracking (1006-rawmode)");
            TerminalUtil::set_raw_mode(true);
            gm_log("" $(id) ": set terminal input state to 1006-rawmode.");
#endif
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

            server.stop();
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

        /// Adds an EventHook handler for handling the specified handlers at @param events
        void add_hook(vector<Event> events, handler_function_type handler) {
            int id = g_mkid();
            this->hooks.push_back({ id, events, handler });
            gm_log("registered an EventHook("$(id)") for the following events:");
            gm_log(list_events(events) + ".");
        };

        /// Emits an Event of type 'ev' with the specified EventInfo
        void call_event( Event ev, EventInfo& info ) {
            info.event_loop = this;
            /// Search for event handlers, call each hook that uses it
            for (auto& hook : this->hooks) {
                if (hook.locked) continue; /// check if the event handler is busy
                if ( std::find(hook.events.begin(), hook.events.end(), ev) != hook.events.end() ) { /// if the event handler
                                                                                                                        /// handles the specified event
                    if ((global.dev_mode || global.debugger)) { /// log info about the event if the debugger or devmode is enabled
                        if ((ev == UPDATE || ev == FIXED_UPDATE) && !Gmeng::global.dont_hold_back); /// dont log update calls unless dont_hold_back is enabled
                        else gm_log("call to external event hook(id="$(hook.id)") for event " + get_event_name(ev));
                    };
                    /// lock the event, prevent being recalled
                    /// while the handler is busy.
                    hook.locked = true;
                    /// Run the handler for the hook.
                    hook.handler( this->level, &info );
                    /// unlock the handler
                    hook.locked = false;
                } else continue;
            };

            /// If the event has prevent_default enabled via &EventInfo.prevent_default = true,
            /// do not run the default hook for it (if one exists)
            if (info.prevent_default) return;

            /// Search for default event handlers
            for (auto& hook : this->defaults) {
                if ( std::find(hook.events.begin(), hook.events.end(), ev) != hook.events.end() ) { /// if the handler has the event
                if (global.dev_mode || global.debugger) gm_log("call to default event hook(id="$(hook.id)") for event " + get_event_name(ev));
                    hook.locked = true;
                    /// Runs the handler for the hook
                    hook.handler( this->level, &info );
                    hook.locked = false;
                } else continue;
            };
        };

        /// will fuck the application up
        void init_server(Networking::rcon_server_def_t server) {
            this->rcon_opt = server;
            this->server.port = server.port;

            this->server.create_path(path_type_t::GET, "/",
            [&](request& req, response& res) {
                res.status_code = 200;
                res.body = this->level->name + "/" + this->level->desc;
            });

            this->server.create_path(path_type_t::POST, "/",
            [&](request& req, response& res) {
                res.status_code = 200;
                res.body = this->level->name + "/" + this->level->desc;
            });
        };

#ifdef GMENG_SDL
        /// ImGui Texture editor map.
        std::map<std::string, std::unique_ptr<GmengImGuiTextureEditor>> editors;

#endif

        /// unless ran in a different worker thread,
        /// this method will implement an infinite
        /// listener while loop. (as per gmserver_t::run)
        ///
        /// know what you are doing when using this function.
        /// it may pause your game code entirely.
        void init_server(bool state) {
            if (!state) server.stop();
            else server.run();
        };

        /// Will crash the game.
        void reset_server() {
            server.stop();
            server.run();
        };

        Networking::rcon_server_def_t rcon_opt;
        gmserver_t server;

        /// Ends the EventLoop, setting this->cancelled to true
        void cancel() { this->cancelled = true; };

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
        private:
          bool tick_handler = false;
    } EventLoop;

    /// Main Gmeng EventLoop, for accessibility from everywhere.
    EventLoop* main_event_loop = nullptr;

    typedef struct EventLoop_Controller_State {
        Event last_event = INIT;
        bool console_open = false;
    } EventLoop_Controller_State;
};

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
#define SELECT_MOUSE_EVENT(x) x == 64 ? Gmeng::MOUSE_SCROLL_UP : MOUSE_REST_1_CHECKER(x)

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

    return lines;
};


#ifndef GMENG_COMPILING_SCRIPT

#define GAME_LOG(str)                                              \
    do {                                                           \
        auto splitEntries = g_splitStr(str, "\n");                 \
        for (const auto& entry : splitEntries) {                   \
            GAME_LOGSTREAM->push_back(entry);                      \
            GAME_LOGSTREAM_STR << entry << '\n';                   \
        }                                                          \
    } while (0)

#else

#define GAME_LOG(str)                                                                                                                   \
    do {                                                                                                                                \
        auto splitEntries = g_splitStr(str, "\n");                                                                                      \
        for (const auto& entry : splitEntries) {                                                                                        \
            ev->logstream->push_back("[" + get_filename(std::string(__FILE__)) + ":" + std::to_string(__LINE__) + "] " + entry);        \
            *ev->logstream_str << "[" + get_filename(std::string(__FILE__)) + ":" + std::to_string(__LINE__) + "] " << entry << '\n';    \
        }                                                                                                                               \
    } while (0)

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
static int CONSOLE_WIDTH = 80;
static int CONSOLE_HEIGHT = 20;



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
// Periodic, executed with FIXED_UPDATE
extern "C" int gmeng_script_periodic( Gmeng::EventLoop* );

#endif


/// commands for the in-game developer console/
/// the commands defined below (the default implementations)
/// are part of the gmeng internal toolkit.
///
/// more commands can be added later on with Noble dylib scripts.
/// see 'github.com/catriverr/noble' and 'include/noble' on how they work.
///
/// for a list of commands, run the 'help' command in a game instance.
/// the developer console is handled internally, so no set-up functions need to be called.
///
/// a do_event_loop() instance handles the internal workings of the engine automatically.
static vector< std::tuple<string, std::function<int(vector<string>, Gmeng::EventLoop*)>> > commands = {
        { "echo", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
            params.erase(params.begin());
            GAME_LOG(g_joinStr(params, " "));
            return 0;
        } },
        { "stopserver", [](vector<string>, Gmeng::EventLoop* ev) -> int {
            ev->server.stop();
            return 0;
        } },
        { "force_update", [](vector<string>, Gmeng::EventLoop* ev) -> int {
            ev->call_event(Gmeng::FIXED_UPDATE, Gmeng::NO_EVENT_INFO);
            return 0;
        } },
        { "restart", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
            GAME_LOG("[gmeng_internal] restarting...");
            GAME_LOG("replacing current executable image...\nexecvp()");
            int state = restart_program();
            if (state != 0) {
                GAME_LOG("an error occurred while restarting\nthe current program.\n");
                GAME_LOG("gmeng state: "$(state)" - errno: "$(errno)"");
            };

            return 0;
        } },
        { "setwidth", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
            params.erase(params.begin());
            if (params.size() < 1) {
                GAME_LOG("usage: setwidth <console_width(int)>");
                GAME_LOG("this command will crash the game\nif you use a\nnon-integer-conversible parameter");
                return 1;
            } else CONSOLE_WIDTH = std::stoi(params.at(0));
            return 0;
        } },
        { "setheight", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
            params.erase(params.begin());
            if (params.size() < 1) {
                GAME_LOG("usage: setheight <console_height(int)>");
                GAME_LOG("this command will crash the game\nif you use a\nnon-integer-conversible parameter");
                return 1;
            } else CONSOLE_HEIGHT = std::stoi(params.at(0));
            return 0;
        } },
        { "crash", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
            /// this will crash with a segmentation fault.
            params.erase(params.begin());
            if (!crash_protector && !(params.size() > 0 && params.at(0) == "now")) {
                crash_protector = true;
                GAME_LOG("kaboom? run again to confirm.");
            } else {
                /// deliberately causes a crash with a segmentation fault.
                int* ptr = nullptr;
                *ptr = 42;
            };
            return 1;
        } },
        { "exit", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
            GAME_LOG("quiting...");
#ifndef GMENG_SDL
            ev->level->display.set_cursor_visibility(true);
#ifndef _WIN32
            Gmeng::TerminalUtil::set_raw_mode(false);
            Gmeng::TerminalUtil::set_non_blocking(false);
            Gmeng::TerminalUtil::disable_mouse_tracking();
#endif
#endif
            exit(0);
            return 0;
        } },
        { "mod", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
            params.erase(params.begin());
            if (params.size() < 1) { GAME_LOG("usage: mod modifier_name val<int>"); return 1; };
            if (params.size() < 2) {
                GAME_LOG("modifier " + std::string(params.at(0)) + std::string(" = ") + v_str(ev->level->display.camera.modifiers.get_value(params.at(0)))); return 0; };
            int val = std::stoi(params.at(1));
            ev->level->display.camera.set_modifier(params.at(0), val);
            return 0;
        } },
        { "clear", [](vector<string>, Gmeng::EventLoop* ev) -> int { GAME_LOGSTREAM->clear(); return 0; } },
        { "refresh", [](vector<string>, Gmeng::EventLoop* ev) -> int {
                ev->level->display.camera.clear_screen();
                return -13; // -13 means no vertical line to divide the console after the command is ran
            } },
        { "help", [](vector<string>, Gmeng::EventLoop* ev) -> int {
                for (auto& cmd : commands) {
                    string name; std::function<int(vector<string>, Gmeng::EventLoop*)> handler;
                    std::tie(name, handler) = cmd;
                    GAME_LOG("cmd: `" + name + "`");
                };
                GAME_LOG("\nSDL keybinds:");
                GAME_LOG("\t|shift + `| : toggle developer console");
                GAME_LOG("\t|alt + TAB| : toggle vgm model editor");
                GAME_LOG("\t|alt + L  | : toggle level inspector");
                return 0;
            } },
        { "info", [](vector<string>, Gmeng::EventLoop* ev) -> int {
                GAME_LOG("level " + ev->level->name + "");
                GAME_LOG("desc: " + ev->level->desc);
                GAME_LOG("chunks: " + v_str(ev->level->chunks.size()));
                int model_count = 0;
                for (const auto ch : ev->level->chunks) {
                    model_count += ch.models.size();
                };
                GAME_LOG("models: "$(model_count)"F");
                GAME_LOG("frame time: "$(ev->level->display.camera.frame_time)"ms");
                GAME_LOG("draw time: "$(ev->level->display.camera.draw_time)"ms");
                return 0;
            } },

        { "runscript", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
                if (params.size() < 2) { GAME_LOG("usage: runscript <script name>"); GAME_LOG("runs a NOBLE prebuilt script."); return 1; };
                std::string filename = params.at(1);
                if (!filesystem::exists(filename)) {
                    GAME_LOG("ERR: script \"" + filename + "\" doesn't exist, filesystem::exists() nonzero");
                    return 1;
                };
                #ifndef GMENG_COMPILING_SCRIPT
                try {
                    GAME_LOG("file exists, pass");
                    GAME_LOG("initializing script...");
                    GAME_LOG("importing noble dylib handle");
                    // actual import
                    // handle of the dylib
                    auto scr_handle = noble_file_open(filename);
                    GAME_LOG("script import OK");

                    GAME_LOG("attempting to execute <script_obj_dir>::gmeng_script_command()");
                    int comm_execution_result = noble_function(scr_handle, gmeng_script_command, ev);
                    GAME_LOG("execution complete, command result: "$(comm_execution_result)".");

                    dlclose(scr_handle);
                    return 0;
                } catch (const std::exception& e) {
                    GAME_LOG("error at some point: " + std::string(e.what()) + " - can not execute script");
                    return 1;
                };

                #else
                GAME_LOG("how did you run this method from a string anyways");
                return -1;
                #endif
            } },
        { "savelvl", [](vector<string>, Gmeng::EventLoop* ev) -> int {
                GAME_LOG("saving current gamestate & level data.");
                GAME_LOG("FOR DEVELOPERS: add a save function in an event hook for EXIT to make this work");
                // Exit should always save. It'd be the developer's fault if this didn't work, not ours.
                // Even though it's extremely annoying to the user for the engine to do this.
                ev->call_event(Gmeng::Event::EXIT, Gmeng::NO_EVENT_INFO);
                return 0;
            } },
        { "newton", [](vector<string> args, Gmeng::EventLoop* ev) -> int {
                Gmeng::LightSource new_light_source;
                int val = 15;
                if (args.size() == 1) {
                        GAME_LOG("\tusage: newton [brightness]");
                        return 1;
                };
                if (args.size() >= 2) {
                    try {
                        val = std::stoi( args.at(1) );
                    } catch (std::exception& ex) {
                        GAME_LOG("invalid argument, using default brightness of 15.");
                        GAME_LOG("anton chigurh:");
                        GAME_LOG("\tusage: newton [brightness]");
                    };
                }
                new_light_source.position = { val, val };
                new_light_source.intensity = val;
                GAME_LOG("God Said 'Let Newton be!' and there was light. (" + args.at(1) + ")");
                ev->level->entities.push_back(std::make_shared<Gmeng::LightSource>(new_light_source));
                return 0;
            } },
        { "embed_engine", [](auto, Gmeng::EventLoop* ev) -> int {
                Gmeng::SERIALIZED_ENGINE_INFO engine_info;

                engine_info.sprite.name = "ENGINE_INFO";

                bool done = false;
                for ( auto& entity : ev->level->entities ) {
                        if ( entity->get_serialization_id() == Gmeng::SERIALIZED_ENGINE_INFO::id ) {
                        GAME_LOG("found previous data, updating.");
                        auto av = std::make_shared<Gmeng::SERIALIZED_ENGINE_INFO>( engine_info );
                        std::dynamic_pointer_cast<Gmeng::SERIALIZED_ENGINE_INFO>( entity ).swap( av );
                        done = true;
                    };
                };

                if (!done) {
                    GAME_LOG("no previous data, applying new.");
                    ev->level->entities.push_back(std::make_shared<Gmeng::SERIALIZED_ENGINE_INFO>( engine_info ));
                };

                GAME_LOG("done");
                return 0;
            } },
        { "lighting_redoall", [](auto, Gmeng::EventLoop* ev) -> int {
                GAME_LOG("deleting & invalidating all lighting cache");
                ev->level->lighting_cache.clear();
                for (auto lightsource : ev->level->light_sources) {
                    lightsource.second->cached = false;
                }
                return 0;
            } },
        { "ent_list", [](auto args, Gmeng::EventLoop* ev) -> int {


                if ( args.size() > 1 && args.at(1) == "types" ) {
                    GAME_LOG("entity types:");
                    for ( auto ent_type_listing : Gmeng::entity_dictionary ) {
                        GAME_LOG("ENTITY("$( ent_type_listing.first )") = " + ent_type_listing.second);
                    };
                    return 0;
                };

                for ( auto entity : ev->level->entities ) {
                    int ent_sID = entity->get_serialization_id();
                    std::string ent_type = ent_sID == Gmeng::Player::id ? "P" : (
                                ent_sID == Gmeng::LightSource::id ? "L" : (
                                    ent_sID == Gmeng::GMENG_ENTITYTYPES_END::id ? "!" : (
                                            ent_sID == Gmeng::SERIALIZED_ENGINE_INFO::id ? "G" : "E"
                                        )
                                )
                            );
                    GAME_LOG(v_str(ent_sID) + " " + ent_type + " " +
                            _uconv_1ihx(entity->entity_id) +
                            " "$(entity->entity_id)" (" + Gmeng::entity_dictionary.at(ent_sID) + ")");
                    GAME_LOG("  position: " + Gmeng::Renderer::conv_dp( entity->position ));
                    GAME_LOG("  active: "$(entity->active)"");
                    GAME_LOG("  interaction proximity: "$(entity->interaction_proximity)"");
                };
                GAME_LOG("\n"$(ev->level->entities.size()) + " entities total");
                return 0;
            } },
        { "ent", [](vector<string> args, Gmeng::EventLoop* ev) -> int {
                if (args.size() < 4) {
                    GAME_LOG("universal entity parameter modifier");
                    GAME_LOG("usage: ent <entity id> <modifier name> <value>");
                    GAME_LOG("this command is only for the base properties of the Entity class.");
                    GAME_LOG("you can not modify derived entity parameters (by default).");
                    return 1;
                };
                auto arg_ent_id = args.at(1);
                auto modifier_name = args.at(2);
                auto new_value = args.at(3);
                int ent_id = 0;
                try { ent_id = std::stoi(arg_ent_id); }
                catch (std::exception& e) { GAME_LOG("invalid entity id entered"); return 1; };

                bool found = false;
                for ( auto ent : ev->level->entities ) {
                    if ( ent->entity_id  == ent_id ) {
                        found = true;
                        // x position modifier
                        if ( modifier_name == "posx" ) {
                            try {
                                int new_pos = std::stoi(new_value);
                                ent->position.x = new_pos;
                            } catch (std::exception& e) { GAME_LOG(e.what()); };
                        }
                        // y position modifier
                        else if ( modifier_name == "posy" ) {
                            try {
                                int new_pos = std::stoi(new_value);
                                ent->position.y = new_pos;
                            } catch (std::exception& e) { GAME_LOG(e.what()); };
                        }
                        else if ( modifier_name == "active" ) {
                            bool should_it_be = stob(new_value);
                            ent->active = should_it_be;
                        }
                        else if ( modifier_name == "prox" ) {
                            try {
                                int proximity = std::stoi(new_value);
                                ent->interaction_proximity = proximity;
                            } catch (std::exception& e) { GAME_LOG(e.what()); };
                        } else {
                            GAME_LOG("unknown modifier '"+ modifier_name + "'.\nthis command only modifies parameters from the base Entity class.\nderived parameters can not be modified (by default).");
                            return 1;
                        };
                        // if the entity has been found, don't search the next entities.
                        break;
                    };
                };
                if (!found) { GAME_LOG("could not find entity with the specified id"); return 1; }
                else {
                    GAME_LOG("modifier " + modifier_name + " of entity "$(ent_id)" set to " + new_value + ".");
                    return 0;
                };
            } },
        { "ent_create", [](vector<string> args, Gmeng::EventLoop* ev) -> int {
                int ent_type;
                if (args.size() < 2) {
                    GAME_LOG("usage: ent_create <entity derived class id>");
                    return 1;
                };
                try { ent_type = std::stoi( args.at(1) ); } catch(std::exception) {
                    GAME_LOG("invalid parameter. pass a number value.");
                    return 1;
                };

                auto entity_factory = Gmeng::EntityBase::get_derived_factory();
                auto derived_type = entity_factory.find( ent_type );

                if ( derived_type == entity_factory.end() ) {
                    GAME_LOG("invalid derived class id. run 'ent_list types' to see all class ids.");
                    return 1;
                };

                auto entity_ = derived_type->second();
                std::shared_ptr<Gmeng::EntityBase> entity = std::move(entity_);
                ev->level->entities.push_back(entity);
                return 0;
            } },
#ifdef GMENG_SDL
        { "edit_texture", [](vector<string> args, Gmeng::EventLoop* ev) -> int {
                std::string fname = "texture_"$(g_mkid())".gt";
                if (args.size() > 1) fname = args.at(1);

                GAME_LOG("creating or summoning instance GmengImGuiTextureEditor(" + fname + ")...");

                if ( ev->editors.contains(fname) ) ev->editors.find( fname )->second->is_open = true;
                else ev->editors[fname] = std::make_unique<GmengImGuiTextureEditor>(GmengImGuiTextureEditor( fname ));

                GAME_LOG("[edit_texture/" + fname + "] editor.create complete");
                return 0;
        } },
#endif
};

/// executes a developer command to the target of an EventLoop pointer '* ev'
/// 'noecho' means no INTERNAL output will be generated by the command to the console.
/// the command can still echo text. It just will not generate logs like 'unknown command', or splitter lines
int gmeng_run_dev_command(Gmeng::EventLoop* ev, std::string command, bool noecho = false, TRACEFUNC) {
    __functree_call__(gmeng_run_dev_command);
#define _GAME_LOG(x) if (!noecho) do { GAME_LOG(x); } while(0)
    using namespace Gmeng;
    using namespace Gmeng::Renderer;
    using std::string, std::vector, std::deque, std::cout;

    if (!noecho) gm_log("command '" + command + "' called from origin " + TRACEFUNC_STR);

    vector<string> params = g_splitStr(command, " ");

    if (params.size() < 1) return -1; // no empty commands like ' ' or ''

    _GAME_LOG("> " + command); // command name
    _GAME_LOG(repeatString("*", CONSOLE_WIDTH-4)); // vertical line seperator

    Level* level = ev->level;
    Display* display = &ev->level->display;
    Camera<0, 0>* camera = &ev->level->display.camera;

    std::string cmdname = params[0];
    bool no_line;

    int state = -11151; /// -11151 is no command found by default

    for (auto &cmd : commands) {
        string name; std::function<int(vector<string>, EventLoop*)> handler;
        std::tie(name, handler) = cmd;
        if (name == cmdname) state = handler(params, ev);
    };

    if (state == -11151) { _GAME_LOG("unknown command: " + params.at(0)); }
    else if (state == -13) no_line = true; // don't draw vertical line seperator

    variables.at(0) = { "errorlevel", state }; // set the errorlevel value
    ev->call_event(FIXED_UPDATE, Gmeng::NO_EVENT_INFO); // fixed update can be called afterwards

    if ( command != "refresh" ) ev->call_event(FIXED_UPDATE, Gmeng::NO_EVENT_INFO);

    if (!no_line && noecho) _GAME_LOG(repeatString("*", CONSOLE_WIDTH-4)); // vertical line seperator
    return state == -11151 ? 1 : (state == -13 ? 0 : state);
};


[[deprecated]] void dev_console_animation(Gmeng::EventLoop* ev) {
    Gmeng::Camera<0, 0>* camera = &ev->level->display.camera;
    Gmeng::Renderer::Display* display = &ev->level->display;
    Gmeng::Renderer::drawpoint delta_xy = { Gmeng::_vcreate_vp2d_deltax(display->viewpoint), Gmeng::_vcreate_vp2d_deltay(display->viewpoint) };

    int total_lines = 15;
    for (int i = 0; i < total_lines; i++) {
        camera->clear_screen();
    };
};

static bool gmeng_console_state_change_modifier = false;

/// Runs the developer console for Gmeng.
/// This utility is for Terminal-mode only.
/// It should not be used with Gmeng SDL.
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

    using namespace Gmeng;
    using namespace Gmeng::Renderer;
    using std::string, std::vector, std::deque, std::cout;

    EventInfo dd_info = *info;
    bool run = false;
    std::string cmd = "";


    if (info->EVENT == KEYPRESS) {
        switch (info->KEYPRESS_CODE) {
            case 27: /// esc
                dd_info.KEYPRESS_CODE = '~';
                ev->call_event(KEYPRESS, dd_info);
                return;
                break;
            case 10: /// enter
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
                if (dev_console_input.length() >= CONSOLE_WIDTH-7) return;
                dev_console_input += info->KEYPRESS_CODE;
                break;
        };
    };

    int last_result = -1;

    if (run == true) run = false, last_result = gmeng_run_dev_command(ev, cmd);
    if (ev->cancelled) return;

    int CUR_COLOR = last_result == -1 ? color_t::YELLOW : ( last_result == 0 ? GREEN : RED );

    Display* display = &ev->level->display;
    drawpoint delta_xy = { Gmeng::_vcreate_vp2d_deltax(display->viewpoint), Gmeng::_vcreate_vp2d_deltay(display->viewpoint) };
    Camera<0, 0>* camera = &ev->level->display.camera;

    color_t CONSOLE_OUTLINE_COLOR = CYAN;


    #define OUTLINE "+" + repeatString("-", CONSOLE_WIDTH-2) + "+"

    deque<string> log_last = gmeng_log_get_last_lines(CONSOLE_HEIGHT);


    log_last.push_front(OUTLINE);
    log_last.push_front("| gmeng " + version + "-" + GMENG_BUILD_NO + " debugger | " + g_splitStr(Gmeng::func_last.back(), " >>").front() + " " + g_splitStr(Gmeng::func_last.back(), "::").back() );
    while(log_last.at(0).length() < CONSOLE_WIDTH-1) log_last.at(0) += " ";
    log_last.at(0) += "|";

    camera->set_curXY( -1, delta_xy.x+2 );
    cout << colors[CONSOLE_OUTLINE_COLOR] << OUTLINE << resetcolor;
    int i = 0;

    for (int j = 0; i < log_last.size(); i++) {
        string data = log_last.at(i);

        camera->set_curXY(i, delta_xy.x+2);

        if (i != 0 && i != 1 &&
            data.length() > CONSOLE_WIDTH-4) data = data.substr(0, CONSOLE_WIDTH-7) + "...";

        if (i == 0 || i == 1) { cout << colors[CONSOLE_OUTLINE_COLOR] << data << resetcolor; continue; };

        while (data.length() < CONSOLE_WIDTH-4) data += " ";

        cout << colors[CONSOLE_OUTLINE_COLOR] << "| " << resetcolor << data << colors[CONSOLE_OUTLINE_COLOR] << " |" << resetcolor;
    };

    /// unused
    int d = 0;
    while (i < CONSOLE_HEIGHT+2) {
        camera->set_curXY(i, delta_xy.x+2);
        cout << colors[CONSOLE_OUTLINE_COLOR] << "|" << resetcolor << repeatString(" ", CONSOLE_WIDTH-2) << colors[CONSOLE_OUTLINE_COLOR] << "|" << resetcolor;
        i++;
    };

    camera->set_curXY(i+d, delta_xy.x+2);
    cout << colors[CONSOLE_OUTLINE_COLOR] << OUTLINE << resetcolor;
    camera->set_curXY(1+i+d, delta_xy.x+2);

    std::string commandline = dev_console_input + Gmeng::c_unit;
    while(commandline.length() < CONSOLE_WIDTH-4) commandline += " ";

    cout << colors[CONSOLE_OUTLINE_COLOR] << "| " << boldcolor << colors[CUR_COLOR] << ">> " << resetcolor << commandline << colors[CONSOLE_OUTLINE_COLOR] << '|' << resetcolor;

    camera->set_curXY(2+i+d, delta_xy.x+2);
    cout << colors[CONSOLE_OUTLINE_COLOR] << OUTLINE << resetcolor;

    already_writing = false;
};

#ifndef _WIN32
#if GMENG_SDL == false
/// runs an event loop instance
/// (this means handling the level as the main event loop / the instance of the game)
///
/// all internal workings of the engine are handled automatically within this method.
///
/// for a correct instance of a Gmeng game, do_event_loop is the standard practice.
int do_event_loop(Gmeng::EventLoop* ev) {
    __functree_call__(do_event_loop);
    using namespace Gmeng;

    EventLoop_Controller_State state;

    char* term_prog = getenv("TERM_PROGRAM");

    if ( term_prog != nullptr && ( std::string(term_prog) == "tmux"
                                || std::string(term_prog) == "iTerm.app" ) ) {
        gmeng_show_warning("bad_terminal");
    };

    if ( ev->level->display.camera.modifiers.get_value("lighting") == 1 &&
         term_prog != nullptr && ( std::string(term_prog) == "tmux" ))
            gmeng_show_warning("lighting_with_tmux");

    if (Gmeng::main_event_loop != nullptr) return 1;
    Gmeng::main_event_loop = ev;

    char buf[64];

    if (ev->cancelled) return -1;
    else ev->call_event(Gmeng::INIT, Gmeng::INIT_INFO);

    ev->call_event(Gmeng::UPDATE, Gmeng::NO_EVENT_INFO);
    state.console_open = false;
    bool curstate = state.console_open;

    std::unordered_map<std::shared_ptr<Gmeng::EntityBase>,
    std::vector<std::shared_ptr<Gmeng::EntityBase>>> interaction_map;


    /// handler thread for raw UPDATE
    create_thread([&] {
        while( !ev->cancelled ) {
            ev->call_event( UPDATE, Gmeng::NO_EVENT_INFO );
            if ( !dev_console_open ) ev->call_event( FIXED_UPDATE, Gmeng::NO_EVENT_INFO );
        };
    });

    /// handler! thread for and developer console.
    /// this also handles the EventLoop::next_tick processes.
    /// this also handles dev console processes.
    create_thread([&]() {
        while (!ev->cancelled) {

            ev->level->display.camera.entity_count = ev->level->entities.size();
            unsigned int model_count = 0;
            for ( auto chunk : ev->level->chunks )
                model_count += chunk.models.size();
            ev->level->display.camera.model_count = model_count;

            if (state.console_open) gmeng_dev_console(ev, &Gmeng::NO_EVENT_INFO); // developer console is on, no raw update.
            else if (gmeng_console_state_change_modifier) {
                gmeng_console_state_change_modifier = false;
                gmeng_run_dev_command(ev, "refresh", true); // refresh the screen
            };


            for ( auto entity : ev->level->entities ) {
                ev->level->entity_grid.update_entity( entity->entity_id, entity->position.x, entity->position.y, entity->interaction_proximity, entity );
            };
            interaction_map = ev->level->entity_grid.build_interaction_map();
        };

    });
#ifndef GMENG_COMPILING_SCRIPT
    /// handler thread for all server utils.
    /// not exactly for multiplayer, but for a server instance
    /// that allows for external connection to the server.
    /* disabled 12.0.0
    create_thread([&]() -> void {
        auto server_handle = noble_file_open("scripts/server.dylib");

        int port = noble_function(server_handle, gmeng_script_command, ev);
        GAME_LOG("server at port "$(port)" was closed");
    });*/
#endif
    /// main handler thread, for the game events
    while (!ev->cancelled) {
        ssize_t n = read(STDIN_FILENO, buf, sizeof(buf)); /// total bytes read
        Gmeng::Event t_event = Gmeng::UNKNOWN; // current event
        Gmeng::EventInfo info; // filled later, event info

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
                char eventType;

                auto scan_out = sscanf(buf, "\033[<%d;%d;%d%c", &button, &x, &y, &eventType);
                if (scan_out != 4) {
                    gm_log("while parsing STDIN raw input, a determined mouse event did not match the parsed sscanf output.");
                    gm_log("ignoring this event.");
                    continue;
                };

                Gmeng::Event event_tp = SELECT_MOUSE_EVENT(button);

                if (event_tp == Gmeng::UNKNOWN) {
                    gm_log("received an unknown supposed mouse event: SELECT_MOUSE_EVENT(x): { UNKNOWN, .button="$(button)", .posX="$(x)", .posY="$(y)" }");
                    gm_log("cancelling this event call.               ^ ~~~~~~~~~~~~~~~~~~");
                    continue;
                };

                bool is_alternative = (
                    button == 39 || button == 4 || button == 6 || button == 71 || button == 70
                //  shift+move      shift+LMB      shift+RMB      shift+SCROLLUP  shift+SCROLLDOWN
                );

                info = {
                    .EVENT = event_tp,
                    .KEYPRESS_CODE = 0,
                    .MOUSE_X_POS = x-1,
                    .MOUSE_Y_POS = y-1,
                    .prevent_default = false,
                    .alternative = is_alternative
                };

                t_event = event_tp;
            } else {
                /// KEYBOARD EVENTS (keypress)
                /// KEYCODE IS buf[0];
                info = {
                    .EVENT = Gmeng::KEYPRESS,
                    .KEYPRESS_CODE = buf[0],
                    .MOUSE_X_POS = -1,
                    .MOUSE_Y_POS = -1,
                    .prevent_default = false,
                };

                if (ev->modifiers.get_value("allow_console") == 1) {
                    if (info.KEYPRESS_CODE == '~') {
                        if (state.console_open) ev->level->display.camera.clear_screen();
                        state.console_open = !state.console_open;
                        dev_console_open = state.console_open;
                        gmeng_dev_console(ev, &info);
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

        if (!state.console_open) ev->call_event(info.EVENT, info), ev->call_event(Gmeng::FIXED_UPDATE, scope);
        else gmeng_dev_console(ev, &info);
    };
    // exit scene, since the while loop broke, ev->cancelled is true.
    gm_log("main game event loop (with id "$(ev->id)") closed");
    Gmeng::main_event_loop = nullptr;

    // disable all terminal modes
    Gmeng::TerminalUtil::set_non_blocking(false);
    Gmeng::TerminalUtil::set_raw_mode(false);
    Gmeng::TerminalUtil::disable_mouse_tracking();

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
                if ( key_sym == SDLK_BACKQUOTE && shift ) {
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

        ev->level->display.camera.entity_count = ev->level->entities.size();

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
        vector<std::string> names = { "gmeng log", "gmeng functree" };
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
        if (ev->level->display.camera.modifiers.get_value("draw_info") == 1 && dev_console_open)
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
        ev->call_event(Gmeng::Event::FIXED_UPDATE, Gmeng::NO_EVENT_INFO);
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


