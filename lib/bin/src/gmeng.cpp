#pragma once
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <array>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "../gmeng.h"

/// MAX MAP SIZE
/// can be set in the makefile
/// version 10.1.1 will add this option to `make configure`
#ifndef GMENG_MAX_MAP_SIZE
    #define GMENG_MAX_MAP_SIZE 32767
#endif

#define $(x) + v_str(x) +

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
    }
    // Include the final point (x2, y2)
    Objects::coord point;
    point.x = x2; point.y = y2;
    coordinates.push_back(point);
return coordinates;
}

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
        std::size_t w = _w; std::size_t h = _h;
        uint32_t frame_time = 0;
        uint32_t draw_time = 0;

        ModifierList modifiers = {
			.values = std::vector<modifier> {
                /// should be moved to the Game Event Loop.
				modifier { .name="noclip",             .value=0 },
                /// force_update disallows draw() from just writing the
                /// output in raw_unit_map, forcing a call to update() first.
				modifier { .name="force_update",       .value=0 },
                /// should be moved to the Game Event Loop.
				modifier { .name="allow_plugins",      .value=1 },
                /// deprecated
				modifier { .name="cast_events",        .value=1 },
                /// for deprecated logging methods
				modifier { .name="allow_writing_plog", .value=1 },
                /// cubic render, squishes units into squares [|] instead of
                /// writing them as 1x2 full unit characters [I].
                modifier { .name="cubic_render",       .value=1 } // enabled since v8.2.2-d
			}
		};

		DisplayMap<_w, _h> display_map;
		std::string raw_unit_map[GMENG_MAX_MAP_SIZE];
		uint32_t entitytotal = 0;

        /// #region DEPRECATED

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

		inline bool has_modifier(std::string name) { for (const Gmeng::modifier& modifier : modifiers.values) if (modifier.name == name && modifier.value == 1) return true; return false; };
		inline void update_modifier(Gmeng::modifier& modifier, int value) {
            __functree_call__(Gmeng::Camera::update_modifier);
            modifier.value = value;
        };

		inline void set_modifier(std::string name, int value) {
            __functree_call__(Gmeng::Camera::set_modifier);
			int vi = g_find_modifier(this->modifiers.values, name);
    			if (vi != -1) this->update_modifier(this->modifiers.values[vi], value);
    			else this->modifiers.values.emplace_back(Gmeng::modifier { .name=name, .value=value });
		};

        /// @deprecated players will not be handled this way in coming versions.
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

        /// @deprecated do not use
		inline void AddEntity(int entityId, Objects::G_Entity entity) {
            __functree_call__(Gmeng::Camera::__no_impl__::AddEntity);
			//working on
		};

        /// @deprecated
		inline void RemoveEntity(int entityId) {
            __functree_call__(Gmeng::Camera::__no_impl__::RemoveEntity);
			//working on
		};

        /// @deprecated
		inline Objects::coord GetPos(int entityId) {
            __functree_call__(Gmeng::Camera::GetPos);
			bool exists;
			Objects::G_Entity entity;
			for (int i = 0; i < this->entitytotal; i++) {
				Objects::G_Entity ent = this->entitymap[i];
				if (ent.entityId == entityId) { exists = true; entity = this->entitymap[i]; break; };
				continue;
			};
			if (!exists) throw std::invalid_argument("no such object: cannot get location");
			return entity.coords;
		};

        /// sets the cursor's position (accounted for the frame)
		inline void set_curXY(int x, int y) {
            __functree_call__(Gmeng::Camera::set_curXY);
   			 std::cout << "\033[" << x+2 << ";" << y+2 << "H"; return; // extra numbers account for the border around the map.
		};

		inline void reset_cur() {
            __functree_call__(Gmeng::Camera::reset_cur);
			this->set_curXY(-2, -2);
		};

		inline Objects::coord get_xy(int __p1) {
            __functree_call__(Gmeng::Camera::get_xy);
			int __p1_x = __p1 / this->w;
			int __p1_y = __p1 % this->w;
			return { .x=__p1_x,.y=__p1_y };
		};

        /// draws some infographics to the screen.
        inline void draw_info(int x = 0, int y = 0) {
            /// __functree_call__(Gmeng::Camera::draw_info);
            this->set_curXY(y,x);
            std::cout << Gmeng::resetcolor;
			WRITE_PARSED("[ gmeng $!__VERSION - build $!__BUILD ]   ");
            this->set_curXY(y+1,x);
            WRITE_PARSED("[ frame_time: "$(this->frame_time)"ms, draw_time: "$(this->draw_time)"ms ]   ");
			this->set_curXY(y+2,x);
            WRITE_PARSED("[ viewport_size: "$(this->w)"x"$(this->h)" ]   ");
        };

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

        inline void set_entTag(std::string __nt) {
            __functree_call__(Gmeng::Camera::set_entTag);
            this->player.c_ent_tag = __nt;
        };

        inline std::string get_entTag() {
            __functree_call__(Gmeng::Camera::get_entTag);
            return this->player.c_ent_tag;
        };

		inline void rewrite_full() {
            __functree_call__(Gmeng::Camera::rewrite_full);
			this->clear_screen();
			this->update();
            std::cout << repeatString("\n", 20) << endl;
			std::cout << this->draw() << std::endl;
		};

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

		inline void MoveEntity(int entityId, int width, int height) {
            __functree_call__(Gmeng::Camera::MoveEntity);
			int move_to_in_map = (height*this->w)+width;
			bool exists = false;
			Objects::G_Entity entity;
			for (int i = 0; i < this->entitytotal; i++) {
				Objects::G_Entity ent = this->entitymap[i];
				if (ent.entityId == entityId) { exists = true; entity = this->entitymap[i]; break; };
				continue;
			};
			if (!exists) throw std::invalid_argument("recieved invalid entityId: no such entity");
			if (move_to_in_map > this->_w*this->_h) return;
			Gmeng::Unit location_in_map = this->display_map.unitmap[move_to_in_map];
			if (!location_in_map.collidable) return;
			this->display_map.unitmap[move_to_in_map] = Gmeng::Unit{.color=0,.collidable=false,.is_entity=true};

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

namespace Gmeng::RemoteServer {
    static bool state = false;
    static gmserver_t server(7385);
    static std::string aplpass = v_str(g_mkid());
    static std::thread* thread;
};

static void _gremote_server_apl(bool state, std::string aplpass) {
    __functree_call__(_gremote_server_apl);
    Gmeng::RemoteServer::state = state;
    Gmeng::RemoteServer::aplpass = aplpass;
    auto thread_t = Gmeng::_ucreate_thread([&]() {
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
#include <errno.h>


#ifndef _WIN32
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

    void set_non_blocking(bool state) { /// DO NOT USE, IT CAN NOT BE DISABLED
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


/// Event Hook System
namespace Gmeng {
    using std::vector, std::string;

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

    const std::string& get_event_name(Event event) {
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
        if (event < 0 || event >= EVENT_COUNT) {
            static const std::string unknown = "UNKNOWN";
            return unknown;
        }
        return event_names[event];
    }

    string list_events(vector<Event> events) {
        string final;
        for (const auto ev : events) {
            final += get_event_name(ev) + ", ";
        };
        return final;
    };

    typedef struct EventInfo {
        Event EVENT;
        char KEYPRESS_CODE;
        int MOUSE_X_POS;
        int MOUSE_Y_POS;

        bool prevent_default = false;
    } EventInfo;

    EventInfo NO_EVENT_INFO = EventInfo { Gmeng::UNKNOWN, 0, -1, -1, false };
    EventInfo INIT_INFO = EventInfo { Gmeng::INIT, 0, -1, -1, false };

    using handler_function_type = std::function<void(Gmeng::Level*, EventInfo*)>;
    typedef struct {
        int id; vector<Event> events;
        handler_function_type handler;
        bool locked;
    } EventHook;

    typedef struct EventLoop {
        int id; Gmeng::Level* level;

        /// Processes, used for registering event calls for the
        /// next tick of the event loop. Called with `UPDATE` event
        vector<EventHook> processes;

        vector<EventHook> hooks;
        vector<EventHook> defaults;

        bool cancelled = false;

        EventLoop( vector<EventHook> hooks_ ) : hooks(hooks_) {
            this->id = g_mkid();
            gm_log("" $(id) ": created main game eventloop with id " $(this->id) ".");

#ifndef _WIN32
            TerminalUtil::enable_mouse_tracking();
            gm_log("" $(id) ": enabled mouse tracking");
            TerminalUtil::set_raw_mode(true);
            gm_log("" $(id) ": set terminal state to raw mode.");
#endif
        };

        ~EventLoop() {
#ifndef _WIN32
            gm_log("" $(id) ": destroyed eventloop " $(this->id) ".");
            gm_log("" $(id) ": disabled mouse tracking");
            TerminalUtil::set_non_blocking(false);
            gm_log(""$(id)": disabled non blocking input mode");
            TerminalUtil::disable_mouse_tracking();
#endif
        };

        void next_tick(handler_function_type handler) {
            int id = g_mkid();
            this->processes.push_back({ id, { UPDATE }, handler });
        };

        void progress_tick() {
            int i = 0;
            for (auto proc : this->processes) {
                /// delete the process
                this->processes.erase(this->processes.begin()+i);
                proc.handler(this->level, &Gmeng::NO_EVENT_INFO);
                i++;
            };
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

            if (info.prevent_default) return;
            /// If the event is cancelled via &EventInfo.prevent_default = true,
            /// do not run the default hook for it (if one exists)

            for (auto& hook : this->defaults) {
                if ( std::find(hook.events.begin(), hook.events.end(), ev) != hook.events.end() ) {
                if (global.dev_mode || global.debugger) gm_log("call to default event hook(id="$(hook.id)") for event " + get_event_name(ev));
                    hook.handler( this->level, &info );
                    /// Runs the handler for the hook
                    /// Keep in mind that hooks may have more than one handler,
                    /// so it may need to check for the EVENT value (in EventInfo)
                    /// since v10.0.0
                } else continue;
            };
        };
    } EventLoop;

    EventLoop* main_event_loop = nullptr;

    typedef struct EventLoop_Controller_State {
        Event last_event = INIT;
    } EventLoop_Controller_State;
};

#define MOUSE_REST_2_CHECKER(x) x == 0 ? Gmeng::MOUSE_CLICK_LEFT_START : (\
                                        x == 1 ? Gmeng::MOUSE_CLICK_MIDDLE_START : (\
                                                x == 2 ? Gmeng::MOUSE_CLICK_RIGHT_START : (\
                                                        x == 3 ? Gmeng::MOUSE_CLICK_END_ANY : (\
                                                                x == 35 ? Gmeng::MOUSE_MOVE : Gmeng::UNKNOWN) )))
#define MOUSE_REST_1_CHECKER(x) x == 65 ? Gmeng::MOUSE_SCROLL_DOWN : MOUSE_REST_2_CHECKER(x)
#define SELECT_MOUSE_EVENT(x) x == 64 ? Gmeng::MOUSE_SCROLL_UP : MOUSE_REST_1_CHECKER(x)

#ifndef _WIN32
/// runs an event loop instance
/// (this means handling the level as the main event loop / the instance of the game)
int do_event_loop(Gmeng::EventLoop* ev) {
    Gmeng::EventLoop_Controller_State state;

    if (Gmeng::main_event_loop != nullptr) return 1;
    Gmeng::main_event_loop = ev;

    char buf[64];

    if (!ev->cancelled) ev->call_event(Gmeng::INIT, Gmeng::INIT_INFO);

    Gmeng::_ucreate_thread([&]() {
        while (!ev->cancelled) {
            ev->call_event(Gmeng::UPDATE, Gmeng::NO_EVENT_INFO);
            ev->progress_tick();
        };
    });

    while (!ev->cancelled) {
        ssize_t n = read(STDIN_FILENO, buf, sizeof(buf)); /// total bytes read
        Gmeng::Event t_event = Gmeng::UNKNOWN;
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

                Gmeng::EventInfo info = {
                    .EVENT = event_tp,
                    .KEYPRESS_CODE = 0,
                    .MOUSE_X_POS = x-1,
                    .MOUSE_Y_POS = y-1,
                    .prevent_default = false
                };

                ev->call_event(event_tp, info);
                t_event = event_tp;
            } else {
                /// KEYBOARD EVENTS (keypress)
                /// KEYCODE IS buf[0];
                Gmeng::EventInfo info = {
                    .EVENT = Gmeng::KEYPRESS,
                    .KEYPRESS_CODE = buf[0],
                    .MOUSE_X_POS = -1,
                    .MOUSE_Y_POS = -1,
                    .prevent_default = false,
                };
                ev->call_event(Gmeng::KEYPRESS, info);
                t_event = Gmeng::KEYPRESS;
            };
        };
        Gmeng::EventInfo scope = {
            .EVENT = t_event
        };
        ev->call_event(Gmeng::FIXED_UPDATE, scope);
    };

    gm_log("main game event loop (with id "$(ev->id)") closed");
    Gmeng::main_event_loop = nullptr;
    Gmeng::TerminalUtil::set_non_blocking(false);
    Gmeng::TerminalUtil::set_raw_mode(false);
    Gmeng::TerminalUtil::disable_mouse_tracking();
    Gmeng::_uclear_threads();
    return 0;
};
#endif

typedef struct {
    int DEF_DELTAX;
    int DEF_DELTAY;

    int SKY_WIDTH;
    int SKY_HEIGHT;

    Gmeng::color_t SKY_COLOR;

    std::unordered_map<std::string, Gmeng::Renderer::drawpoint> model_positions;

    int A00_CAKE_INTERACT_LOOPC;
} gmeng_properties_t;

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
    return -1;
};

#endif

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
