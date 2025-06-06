#pragma once
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <string>
#include <array>
#include <cstring>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "../../../include/noble/lib/scripts/arch.cc" // NOBLE: No Bad Language Esoterics (https://github.com/catriverr/noble)
                                                      // used for .dylib prebuilt C++ script handling.
#include "../gmeng.h"

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

#if GMENG_SDL_ENABLED == true
/// SDL Imports
#include "../types/window.h" // typedef
#include "../utils/window.cpp" // util

#endif

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

    /// Camera Instance of any viewport in the game engine.
    /// All displays are rendered through this class.
	template<std::size_t _w, std::size_t _h>
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
                /// noclip modifier, disables all collision
				modifier { .name="noclip",                  .value=0 },
                /// force_update disallows draw() from just writing the
                /// output in raw_unit_map, forcing a call to update() first.
				modifier { .name="force_update",            .value=0 },
                /// should be moved to the Game Event Loop.
				modifier { .name="allow_plugins",           .value=1 },
                /// deprecated
				modifier { .name="cast_events",             .value=1 },
                /// for deprecated logging methods
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
            }
		};

        // Display map, contains the current
        // screen data in units
		DisplayMap<_w, _h> display_map;
        // raw unit map, contains the rendered units
        // to be written to the console output
		std::string raw_unit_map[GMENG_MAX_MAP_SIZE];


        /// #region DEPRECATED

        // @deprecated - kept for backwards compatibility
        // contains the total entity count
        uint32_t entitytotal = 0;

        // @deprecated entities will be handled as individual models
        // instead of individual units in the future.
        // This change is expected to be shipped in version 12.0.0.
        // Kept for backwards compatibility.
		Objects::G_Entity entitymap[GMENG_MAX_MAP_SIZE] = {};

        // @deprecated Event handlers are now internal, So they are not passed to
        // an input pipe for a parent program, but it's kept for backwards compatibility
        // with older versions of gmeng (the 1.1 framework) which use this functionality.
		EventHandler event_handler;

        // @deprecated Player objects ( 1.1 entity definitions ) are deprecated
        // since we now use models instead of specific units for players.
		Objects::G_Player player = {};

        // @deprecated kept only for backwards compatibility
		Unit playerunit = {};
        // @deprecated kept only for backwards compatibility
		bool player_init = false;

        /// #endregion DEPRECATED


        /// Sets the resolution of the camera.
		inline void SetResolution(std::size_t w, std::size_t h) {
            __functree_call__(Gmeng::Camera::SetResolution);
			display_map.__h = h; display_map.__w = w;
			this->w = w; this->h = h;
		};

        /// initializes the display's unitmap
		inline void constructor(Gmeng::Unit unitmap[_w*_h]) {
            __functree_call__(Gmeng::Camera::constructor);
			for (int i = 0; i < (w*h); i++) {
				this->display_map.unitmap[i] = unitmap[i];
			};
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
			for (int i = 0; i < (cc); i++) {
				if (i % this->w == 0) {
                    /// appends the frame's left and right outline to the output.
                    if (global.debugger) gm_slog(YELLOW, "DEBUGGER", "append_newline__" + v_str( (int)(i / cubic_height) ));
					if (i > 1) final += "\x1B[38;2;246;128;25m",  final += Gmeng::c_unit;
					final += "\n\x1B[38;2;246;128;25m"; final += Gmeng::c_unit;
				};
                /// if the unit is empty, make it a void pixel. should not happen though.
				final += this->raw_unit_map[i].empty() ? colors[BLACK] + Gmeng::c_unit + colors[WHITE] : this->raw_unit_map[i];
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

        /// @deprecated players will not be handled this way in coming versions.
        /// sets the unit at x,y point to a player object with an entityId
		inline void SetPlayer(int entityId, Objects::G_Player player, int x= 0, int y = -1, bool force = false) {
			__functree_call__(Gmeng::Camera::SetPlayer);

            for (int i = 0; i < this->entitytotal; i++) {
				Objects::G_Entity entity = this->entitymap[i];
				if (entity.entityId == entityId) throw std::invalid_argument("entity already exists: cannot create player");
			};

			int goto_loc = (y != -1) ? ((y*this->w)+x) : (x);
			if (goto_loc > this->w*this->h) throw std::invalid_argument("entity cannot be placed in the provided x-y coordinates @ pos(" +v_str(x)+ "," +v_str(y)+ ") [" + v_str(goto_loc) + "/" + v_str(this->w*this->h) +" - " + (y == -1 ? "1d local" : "2d local") +"]");
			if (!this->display_map.unitmap[goto_loc].collidable && !force) throw std::invalid_argument("entity cannot be placed in the provided x-y coordinates: the unit at location " + v_str(x) + "," + v_str(y) + " is not collidable");

            this->entitymap[entityId] = player;
			int pos = goto_loc;

			this->playerunit = this->display_map.unitmap[pos];

            this->display_map.unitmap[goto_loc] = Gmeng::Unit{
				.color=player.colorId,.collidable=false,.is_player=true,.is_entity=false,
				.player=player
			};

			this->player = player;
			this->player.coords.x = x; //FIX
			this->player.coords.y = y; //FIX
			this->entitytotal++;
			this->player_init = true;
		};

        /// sets the cursor's position (accounted for the frame)
		inline void set_curXY(int x, int y) {
            //__functree_call__(Gmeng::Camera::set_curXY);
   			// this functree call caused so much logging that VIM would crash the OS when opening it.
            // so it is disabled now.

            std::cout << "\033[" << x+2 << ";" << y+2 << "H"; return; // extra numbers account for the border around the map.
		};

        /// resets the cursor to the 0,0 position of the terminal
		inline void reset_cur() {
            __functree_call__(Gmeng::Camera::reset_cur);
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
            this->set_curXY(y+1,x);
            WRITE_PARSED("[ frame_time: ~g~"$(this->frame_time)"ms~n~, draw_time: ~y~"$(this->draw_time)"ms~n~ ]   ");
			this->set_curXY(y+2,x);
            WRITE_PARSED("[ viewport_size: ~p~"$(this->w)"x"$(this->h)"~n~ ]   ");
            this->set_curXY(y+3, x);
            WRITE_PARSED("[ entities: ~b~"$(this->entity_count)"~n~, models: ~c~"$(this->model_count)"~n~ ]   ");
            this->set_curXY( y+4, x );
            WRITE_PARSED("~_~" + repeatString("-", 40) + "~n~");
            this->set_curXY( y+5, x );
            WRITE_PARSED("modifiers:");
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
            this->set_curXY( y+8, x );
            WRITE_PARSED("~_~" + repeatString("-", 40) + "~n~");
        };

        /// draws a 'Unit' object and returns it as a printable string
		inline std::string draw_unit(Gmeng::Unit __u, Gmeng::Unit __nu = Unit { .is_entity=1 }, bool prefer_second = false) {
			Gmeng::Unit current_unit = __u;
            // check if cubic render is preferred, and a next unit is provided
            bool nu = this->has_modifier("cubic_render") && !__nu.is_entity;
            Gmeng::Unit next_unit = __nu;
            // by default, colors are transparent (void/black)
            std::string funit_color = Gmeng::colors[current_unit.color];
            std::string bunit_color = nu ? Gmeng::bgcolors[next_unit.color] : Gmeng::bgcolors[BLACK];
            /*if (current_unit.color == next_unit.color) {
                if (current_unit.color == WHITE) return colors[WHITE] + Gmeng::c_unit + Gmeng::resetcolor;
                return bgcolors_bright[current_unit.color] + " " + Gmeng::resetcolor;
            };*/ // v8.2.2-d: this expects the units to be in Y-index ordered
            if (current_unit.color == next_unit.color) {
                if (this->has_modifier("wireframe_render")) {
                    if (prefer_second) return bgcolors[next_unit.color] + colors[BLACK] + ((current_unit.collidable || this->has_modifier("noclip") ? "x" : "X")) + Gmeng::resetcolor;
                    else return bgcolors[current_unit.color] + colors[BLACK] + ((next_unit.collidable || this->has_modifier("noclip") ? "x" : "X")) + Gmeng::resetcolor;
                };
                return bgcolors_bright[current_unit.color] + colors[current_unit.color] + (current_unit.color != BLACK ? boldcolor : "") + Gmeng::c_outer_unit + Gmeng::resetcolor;
            };

            if (current_unit.special && !prefer_second) {
                return bgcolors_bright[current_unit.color] + boldcolor + colors[current_unit.special_clr] + current_unit.special_c_unit + resetcolor;
            } else if (nu && next_unit.special && ( !current_unit.special || prefer_second )) {
                return bgcolors_bright[next_unit.color] + boldcolor + colors[next_unit.special_clr] + next_unit.special_c_unit + resetcolor;
            };

			if (current_unit.transparent) {
				funit_color = Gmeng::colors[7];
            }; if (nu && next_unit.transparent) {
                bunit_color = Gmeng::bgcolors[7];
            };

            if (this->has_modifier("wireframe_render")) {
                std::string final = "\x1B[4"+std::string(Gmeng::colorids[current_unit.color])+"m" + (nu ? Gmeng::colors[next_unit.color] : "") + (current_unit.collidable || this->has_modifier("noclip") ? "x" : "X") + Gmeng::resetcolor;
                return final;
            };

			std::string final = ( nu ? ( (next_unit.color == WHITE ? bgcolors_bright[WHITE] : bgcolors_bright[next_unit.color]) + (current_unit.color != BLACK ? boldcolor : "") + colors[current_unit.color] + Gmeng::c_outer_unit_floor ) : ( funit_color + Gmeng::c_unit ) ) + Gmeng::resetcolor;
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
            if (this->has_modifier("debug_info")) this->draw_info();
			this->reset_cur();
		};

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

        /// @deprecated - kept for 1.1 compatibility.
		inline void MovePlayer(int entityId, int width, int height) {
            __functree_call__(Gmeng::Camera::MovePlayer);
			int move_to_in_map = (height*this->w)+width;
			bool exists = false;
			Objects::G_Player entity;
			for (int i = 0; i < this->entitytotal; i++) {
				Objects::G_Entity ent = this->entitymap[i];
				if (ent.entityId == entityId) { exists = true; break; };
				continue;
			};
			entity = this->player;
			Objects::coord current_coords = this->player.coords;
			int current_pos_in_map = (current_coords.y*this->w)+current_coords.x;
			Gmeng::Unit current_unit = this->display_map.unitmap[(current_coords.y*this->w)+current_coords.x];
			if (!exists) throw std::invalid_argument("recieved invalid entityId: no such entity");
			if (move_to_in_map > this->w*this->h) return;
			Gmeng::Unit location_in_map = this->display_map.unitmap[move_to_in_map];
			if (!location_in_map.collidable && !this->has_modifier("noclip")) return;
			this->raw_unit_map[(current_coords.y*this->w)+current_coords.x] = this->draw_unit(this->playerunit);
			this->display_map.unitmap[(current_coords.y*this->w)+current_coords.x] = this->playerunit;
			Gmeng::Unit oldPlayerUnit = this->playerunit;
			this->playerunit = this->display_map.unitmap[(height*this->w)+width];
			this->display_map.unitmap[move_to_in_map] = Gmeng::Unit{.color=player.colorId,.collidable=false,.is_player=true,.player=entity,.special=true,.special_clr=oldPlayerUnit.color};
			this->player.coords.x = width; this->player.coords.y = height;
			this->event_handler.cast_ev(Gmeng::CONSTANTS::C_PlugEvent, this->event_handler.gen_estr(
			Gmeng::event {
			.name="player_move",
			.id=Gmeng::CONSTANTS::PE_Type0,
			.params={
				"dX=" + std::to_string(current_coords.x) + ",dY=" + std::to_string(current_coords.y), // old player
				"dX=" + std::to_string(this->player.coords.x) + ",dY=" + std::to_string(this->player.coords.y) // new player
			}
			}));
			if (this->has_modifier("force_update")) { this->rewrite_full(); return; };
			this->raw_unit_map[move_to_in_map] = this->draw_unit(this->display_map.unitmap[move_to_in_map]);
			this->rewrite_mapping({move_to_in_map, current_pos_in_map});
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


/// Event Hook System
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
        "MOUSE_CLICK_END_ANY",
        "MOUSE_SCROLL_UP",
        "MOUSE_SCROLL_DOWN",
        "MOUSE_MOVE",
        "KEYPRESS",
        "UNKNOWN"
    };

    const std::string& get_event_name(Event event) {

        if (event < 0 || event >= EVENT_COUNT) {
            static const std::string unknown = "UNKNOWN";
            return unknown;
        };

        return event_names[event];
    };

    string list_events(vector<Event> events) {
        string final;
        for (const auto ev : events) {
            final += get_event_name(ev) + ", ";
        };
        final.pop_back(); /// remove final ' '
        return final;
    };

    typedef struct EventInfo {
        /// The name of the Event
        Event EVENT;
        /// If it is a KEYPRESS event, this determines the keycode.
        char KEYPRESS_CODE;
        /// If it is a mouse event, the X position of the mouse.
        int MOUSE_X_POS;
        /// If it is a mouse event, the Y position of the mouse.
        int MOUSE_Y_POS;

        /// Prevents default behaviour
        /// from executing
        bool prevent_default = false;
        /// Alternative usage.
        /// usually triggered by holding shift.
        /// one example is using shift + scroll for left-right movement instead of usual down-up movement.
        bool alternative = false;
    } EventInfo;

    EventInfo NO_EVENT_INFO = EventInfo { Gmeng::UNKNOWN, 0, -1, -1, false };
    EventInfo INIT_INFO = EventInfo { Gmeng::INIT, 0, -1, -1, false };

    using handler_function_type = std::function<void(Gmeng::Level*, EventInfo*)>;
    typedef struct {
        int id; vector<Event> events;
        handler_function_type handler;
        bool locked;
    } EventHook;

#if GMENG_SDL
    /// (Gmeng) WindowState for SDL-Based External screen
    /// state type. Contains important information for SDL-Based
    /// typings.
    typedef struct SDL_EventLoop_State {
        /// SDL Game Window container.
        GameWindow* window = NULL;
        /// SDL Image container. type `Gmeng::sImage` is required
        /// for GameWindow::draw( image ), so the container for the camera
        /// output is stored in the window state, see `types/window.h`.
        sImage image;
    } WindowState;
#endif

    /// Gmeng EventLoop Object.
    /// EventLoop allows for a continuous running with very in-depth
    /// default behaviour as well as error handling, so the engine can
    /// do the heavy-lifting of rendering, running, optimizing and handling
    /// a game instance in Gmeng.
    ///
    /// Only one EventLoop object can be ran per gmeng instance,
    /// so no two games can be run at the same time with the same executable.
    typedef struct EventLoop {
        int id; Gmeng::Level* level; std::vector<std::string>* logstream = GAME_LOGSTREAM;

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
        vector<EventHook> defaults;

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
            this->id = g_mkid();
            gm_log("" $(id) ": created main game eventloop with id " $(this->id) ".");

#ifndef _WIN32
            TerminalUtil::enable_mouse_tracking();
            gm_log("" $(id) ": enabled mouse tracking (1006-rawmode)");
            TerminalUtil::set_raw_mode(true);
            gm_log("" $(id) ": set terminal input state to 1006-rawmode.");
#endif
        };

#if GMENG_SDL_ENABLED == true
        /// SDL Game window of the GameLoop. Can be NULLPTR
        /// if it is not used, see EventLoop::uses_sdl.
        Gmeng::GameWindow* window = nullptr;
        /// EventLoop constructor, this will set the SDL mode to TRUE, so if you're not
        /// going to enable it use the EventLoop constructor for the terminal instead.
        EventLoop( vector<EventHook> _hooks = {}, Gmeng::GameWindow* _window = nullptr ) : hooks( _hooks ), window( _window ), uses_sdl( _window != nullptr ) {
            this->id = g_mkid();
            gm_log(""$(id)": created main game eventloop with id "$(this->id)".");
            gm_log(""$(id)": ENABLED EXTERNAL_SDL_WINDOW: This EventLoop will use an SDL-Based external window to draw game state.");
            gm_log("-----");
#ifndef _WIN32
            gm_log(""$(id)": Enabling Terminal-Util functions as well as the SDL instance for dev-console system.");
            TerminalUtil::enable_mouse_tracking();
            gm_log(""$(id)": enabled mouse tracking (1006-rawmode)");
            TerminalUtil::set_raw_mode(true);
            gm_log(""$(id)": set terminal input state to 1006-rawmode.");
#endif
        };
#endif // sdl_enabled?


        ~EventLoop() {
#ifndef _WIN32
            gm_log("" $(id) ": destroyed eventloop " $(this->id) ".");
            gm_log("" $(id) ": disabled mouse tracking");
            TerminalUtil::set_non_blocking(false);
            gm_log(""$(id)": disabled non blocking input mode");
            TerminalUtil::disable_mouse_tracking();

            server.stop();
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

        void add_hook(vector<Event> events, handler_function_type handler) {
            int id = g_mkid();
            this->hooks.push_back({ id, events, handler });
            gm_log("registered an EventHook("$(id)") for the following events:");
            gm_log(list_events(events) + ".");
        };

        void call_event( Event ev, EventInfo& info ) {
            for (auto& hook : this->hooks) {
                if (hook.locked) continue;
                if ( std::find(hook.events.begin(), hook.events.end(), ev) != hook.events.end() ) {
                    if ((global.dev_mode || global.debugger)) {
                        if ((ev == UPDATE || ev == FIXED_UPDATE) && !Gmeng::global.dont_hold_back); /// dont log update calls unless dont_hold_back is enabled
                        else gm_log("call to external event hook(id="$(hook.id)") for event " + get_event_name(ev));
                    };
                    hook.locked = true;
                    /// Runs the handler for the hook
                    /// Keep in mind that hooks may have more than one handler,
                    /// so it may need to check for the EVENT value (in EventInfo)
                    /// since v10.0.0
                    hook.handler( this->level, &info );
                    hook.locked = false;
                } else continue;
            };

            /// If the event is cancelled via &EventInfo.prevent_default = true,
            /// do not run the default hook for it (if one exists)
            if (info.prevent_default) return;

            for (auto& hook : this->defaults) {
                if ( std::find(hook.events.begin(), hook.events.end(), ev) != hook.events.end() ) {
                if (global.dev_mode || global.debugger) gm_log("call to default event hook(id="$(hook.id)") for event " + get_event_name(ev));
                    hook.locked = true;
                    /// Runs the handler for the hook
                    /// Keep in mind that hooks may have more than one handler,
                    /// so it may need to check for the EVENT value (in EventInfo)
                    /// since v10.0.0
                    hook.handler( this->level, &info );
                    hook.locked = false;
                } else continue;
            };
        };

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

        int RCON_REMOTE_COUNT = 0;
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
        }                                                          \
    } while (0)

#else

#define GAME_LOG(str)                                                                                                             \
    do {                                                                                                                          \
        auto splitEntries = g_splitStr(str, "\n");                                                                                \
        for (const auto& entry : splitEntries) {                                                                                  \
            ev->logstream->push_back("[" + get_filename(std::string(__FILE__)) + ":" + std::to_string(__LINE__) + "] " + entry); \
        }                                                                                                                         \
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
/// for a list of commands, run the 'help' or 'helpcmd' command in a game instance.
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
            ev->level->display.set_cursor_visibility(true);
#ifndef _WIN32
            Gmeng::TerminalUtil::set_raw_mode(false);
            Gmeng::TerminalUtil::set_non_blocking(false);
            Gmeng::TerminalUtil::disable_mouse_tracking();
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
                GAME_LOG("figure it out lol");
                GAME_LOG("just kidding just run `helpcmd`");
                return 0;
            } },
        { "helpcmd", [](vector<string>, Gmeng::EventLoop* ev) -> int {
                for (auto& cmd : commands) {
                    string name; std::function<int(vector<string>, Gmeng::EventLoop*)> handler;
                    std::tie(name, handler) = cmd;
                    GAME_LOG("cmd: `" + name + "`");
                };
                GAME_LOG("");
                GAME_LOG("p.s.");
                GAME_LOG("gmeng internally doesn't have descriptions for any of these functions");
                GAME_LOG("try running the command or guess from the name");
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
        { "save", [](vector<string>, Gmeng::EventLoop* ev) -> int {
                GAME_LOG("saving current gamestate & level data.");
                // Exit should always save. It'd be the developer's fault if this didn't work, not ours.
                // Even though it's extremely annoying to the user for the engine to do this.
                ev->call_event(Gmeng::Event::EXIT, Gmeng::NO_EVENT_INFO);
                return 0;
            } }
};

/// executes a developer command to the target of an EventLoop pointer '* ev'
/// 'noecho' means no INTERNAL output will be generated by the command to the console.
/// the command can still echo text. It just will not generate logs like 'unknown command', or splitter lines
int gmeng_run_dev_command(Gmeng::EventLoop* ev, std::string command, bool noecho = false) {
    __functree_call__(gmeng_run_dev_command);
#define _GAME_LOG(x) if (!noecho) do { GAME_LOG(x); } while(0)
    using namespace Gmeng;
    using namespace Gmeng::Renderer;
    using std::string, std::vector, std::deque, std::cout;

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


void dev_console_animation(Gmeng::EventLoop* ev) {
    Gmeng::Camera<0, 0>* camera = &ev->level->display.camera;
    Gmeng::Renderer::Display* display = &ev->level->display;
    Gmeng::Renderer::drawpoint delta_xy = { Gmeng::_vcreate_vp2d_deltax(display->viewpoint), Gmeng::_vcreate_vp2d_deltay(display->viewpoint) };

    int total_lines = 15;
    for (int i = 0; i < total_lines; i++) {
        camera->clear_screen();
    };
};

static bool gmeng_console_state_change_modifier = false;

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

    if (!gmeng_console_state_change_modifier) {
        gmeng_run_dev_command(ev, "refresh", true);
    };

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

    if ( term_prog != nullptr && std::string(term_prog) == "tmux" ) {
        gmeng_show_warning("tmux");
    };

    if (Gmeng::main_event_loop != nullptr) return 1;
    Gmeng::main_event_loop = ev;

    char buf[64];

    if (ev->cancelled) return -1;
    else ev->call_event(Gmeng::INIT, Gmeng::INIT_INFO);

    ev->call_event(Gmeng::UPDATE, Gmeng::NO_EVENT_INFO);
    state.console_open = false;
    bool curstate = state.console_open;

    /// handler thread for raw UPDATE and developer console.
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
        };
    });
#ifndef GMENG_COMPILING_SCRIPT
    /// handler thread for all server utils.
    /// not exactly for multiplayer, but for a server instance
    /// that allows for external connection to the server.
    create_thread([&]() -> void {
        auto server_handle = noble_file_open("scripts/server.dylib");

        int port = noble_function(server_handle, gmeng_script_command, ev);
        GAME_LOG("server at port "$(port)" was closed");
    });
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
#endif

/// @deprecated use level write functions instead
/// see 'util/serialization.cpp'
typedef struct {
    int DEF_DELTAX;
    int DEF_DELTAY;

    int SKY_WIDTH;
    int SKY_HEIGHT;

    Gmeng::color_t SKY_COLOR;

    std::unordered_map<std::string, Gmeng::Renderer::drawpoint> model_positions;

    int A00_CAKE_INTERACT_LOOPC;
} gmeng_properties_t;


/// @deprecated use level write functions instead
/// see 'util/serialization.cpp'
///
/// (this object was originally for a specific scope in the tests/event_loop.cpp prototype anywaws)
static gmeng_properties_t default_properties = {
    50, 25,

    100, 100,

    Gmeng::BLUE,

    {
        { "player", { 0,0 } },

        { "table1", { 13, 21 } },
        { "table2", { 24, 21 } },

        { "cake", { 16, 17 } },

        { "gift1", { 0, 22 } },
        { "gift2", { 44, 22 } },

        { "balloon1", { 4, 4 } },
        { "balloon2", { 37, 7 } },


        { "CAKE_INTERACT_TIMES", { 1, 20 } },
    },

    100
};

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


/// @deprecated use level write functions instead
/// see 'utils/serialization.cpp'
void writeout_properties(const std::string& filename, const gmeng_properties_t& properties) {
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile) {
        throw std::ios_base::failure("Failed to open file for writing");
    }

    // Write the int fields
    outFile.write(reinterpret_cast<const char*>(&properties.DEF_DELTAX), sizeof(properties.DEF_DELTAX));
    outFile.write(reinterpret_cast<const char*>(&properties.DEF_DELTAY), sizeof(properties.DEF_DELTAY));
    outFile.write(reinterpret_cast<const char*>(&properties.SKY_WIDTH), sizeof(properties.SKY_WIDTH));
    outFile.write(reinterpret_cast<const char*>(&properties.SKY_HEIGHT), sizeof(properties.SKY_HEIGHT));
    outFile.write(reinterpret_cast<const char*>(&properties.SKY_COLOR), sizeof(properties.SKY_COLOR));
    outFile.write(reinterpret_cast<const char*>(&properties.A00_CAKE_INTERACT_LOOPC), sizeof(properties.A00_CAKE_INTERACT_LOOPC));

    size_t map_size = properties.model_positions.size();
    outFile.write(reinterpret_cast<const char*>(&map_size), sizeof(map_size));

    for (const auto& [key, drawpoint] : properties.model_positions) {
        size_t key_size = key.size();
        outFile.write(reinterpret_cast<const char*>(&key_size), sizeof(key_size));
        outFile.write(key.c_str(), key_size);

        // Write the drawpoint struct (x and y)
        outFile.write(reinterpret_cast<const char*>(&drawpoint.x), sizeof(drawpoint.x));
        outFile.write(reinterpret_cast<const char*>(&drawpoint.y), sizeof(drawpoint.y));
    }

    outFile.close();
};

/// @deprecated use level write functions instead
/// see 'util/serialization.cpp'
gmeng_properties_t read_properties(const std::string& filename) {
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        throw std::ios_base::failure("Failed to open file for reading");
    }

    gmeng_properties_t properties;

    // Read the int fields
    inFile.read(reinterpret_cast<char*>(&properties.DEF_DELTAX), sizeof(properties.DEF_DELTAX));
    inFile.read(reinterpret_cast<char*>(&properties.DEF_DELTAY), sizeof(properties.DEF_DELTAY));
    inFile.read(reinterpret_cast<char*>(&properties.SKY_WIDTH), sizeof(properties.SKY_WIDTH));
    inFile.read(reinterpret_cast<char*>(&properties.SKY_HEIGHT), sizeof(properties.SKY_HEIGHT));
    inFile.read(reinterpret_cast<char*>(&properties.SKY_COLOR), sizeof(properties.SKY_COLOR));
    inFile.read(reinterpret_cast<char*>(&properties.A00_CAKE_INTERACT_LOOPC), sizeof(properties.A00_CAKE_INTERACT_LOOPC));

    // Read the map (model_positions) size
    size_t map_size;
    inFile.read(reinterpret_cast<char*>(&map_size), sizeof(map_size));

    // Read each key-value pair in the map
    for (size_t i = 0; i < map_size; ++i) {
        // Read the string length, then the string (key)
        size_t key_size;
        inFile.read(reinterpret_cast<char*>(&key_size), sizeof(key_size));
        std::string key(key_size, '\0');
        inFile.read(&key[0], key_size);

        // Read the drawpoint struct (x and y)
        Gmeng::Renderer::drawpoint drawpoint;
        inFile.read(reinterpret_cast<char*>(&drawpoint.x), sizeof(drawpoint.x));
        inFile.read(reinterpret_cast<char*>(&drawpoint.y), sizeof(drawpoint.y));

        // Insert into the map
        properties.model_positions[key] = drawpoint;
    }

    inFile.close();


    return properties;
};
