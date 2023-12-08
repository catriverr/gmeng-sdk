#include <iostream>
#include <string>
#include <array>
#include <cstring>
#include <vector>
#include <algorithm>

#ifdef __GMENG_INIT__

std::vector<Objects::coord> g_trace_trajectory(int x1, int y1, int x2, int y2) {
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
	template<std::size_t _w, std::size_t _h>
	class CameraView {
	  public:
		Gmeng::ModifierList modifiers = {
			.values = std::vector<Gmeng::modifier> {
				Gmeng::modifier { .name="noclip",             .value=0 },
				Gmeng::modifier { .name="force_update",       .value=0 },
				Gmeng::modifier { .name="allow_plugins",      .value=1 },
				Gmeng::modifier { .name="cast_events",        .value=1 },
				Gmeng::modifier { .name="allow_teleporting",  .value=1 },
				Gmeng::modifier { .name="allow_dev_commands", .value=1 },
				Gmeng::modifier { .name="allow_writing_plog", .value=1 }
			}
		};
		Objects::G_Entity entitymap[32767] = {};
		std::size_t w = _w; std::size_t h = _h;
		Gmeng::DisplayMap<_w, _h> display_map;
		Gmeng::EventHandler event_handler;
		std::string raw_unit_map[32767];
		Objects::G_Player player = {};
		Gmeng::Unit playerunit = {};
		bool player_init = false;
		int entitytotal = 0;
		inline void SetResolution(std::size_t w, std::size_t h) {
			display_map.__h = h; display_map.__w = w;
			this->w = w; this->h = h;
		};
		inline void constructor(Gmeng::Unit unitmap[_w*_h]) {
			for (int i = 0; i < (w*h); i++) {
				this->display_map.unitmap[i] = unitmap[i];
			};
		};
		inline void update() {
			for (int i = 0; i < this->h; i++) {
				for (int j = 0; j < this->w; j++) {
					if (i*j == this->h*this->w) break;
					int current_unit_id = (i * this->w) + j;
					Gmeng::Unit current_unit = this->display_map.unitmap[current_unit_id];
					std::string final = this->draw_unit(current_unit);
					this->raw_unit_map[current_unit_id] = final;
				};
			};
		};
        inline void temp_displacement(int __pX, int __pY, Gmeng::Unit u) {
            this->set_curXY(__pX, __pY);
            int pos_in_map = (__pY * this->w) + __pX;
            this->raw_unit_map[pos_in_map] = this->draw_unit(u);
            this->rewrite_mapping({ pos_in_map });
        };
		inline std::string draw() {
            gm_log("Gmeng::CameraView job_render *draw -> total drawpoints available at this->cam::vp_mem0: " + v_str(sizeof(this->raw_unit_map)) + " , v_addr " + _uconv_1ihx(0) + " -> " + _uconv_1ihx(sizeof(this->raw_unit_map)));
			gm_log("Gmeng::CameraView job_render *draw -> total drawpoints allocated for job_render at this->cam::vp_mem0: " + v_str(this->w*this->h) + " | " + _uconv_1ihx(this->w*this->h));
            this->clear_screen();
            std::string final = "";
			for (int i = 0; i < (this->w*this->h); i++) {
				if (i % w == 0) {
					if (i > 1) final += "\x1B[38;2;246;128;25m",  final += Gmeng::c_unit;
					final += "\n\x1B[38;2;246;128;25m"; final += Gmeng::c_unit;
				};
				final += this->raw_unit_map[i];
			};
			std::string __cu = "\x1B[38;2;246;128;25m";
			std::string __cf = "\x1B[38;2;246;128;25m";
			for (int i = 0; i < this->w+2; i++) { __cu += Gmeng::c_outer_unit; __cf += Gmeng::c_outer_unit_floor; };
			final += ("\x1B[38;2;246;128;25m"); final += (Gmeng::c_unit);
			final = __cu + "" + final + "\n" + __cf;
			return final;
		};
		inline bool has_modifier(std::string name) {  for (const Gmeng::modifier& modifier : modifiers.values) if (modifier.name == name && modifier.value == 1) return true; return false; };
		inline void update_modifier(Gmeng::modifier& modifier, int value) { modifier.value = value; };
		inline void set_modifier(std::string name, int value) {
			int vi = g_find_modifier(this->modifiers.values, name);
    			if (vi != -1) this->update_modifier(this->modifiers.values[vi], value);
    			else this->modifiers.values.emplace_back(Gmeng::modifier { .value=value, .name=name });
		};
		inline void SetPlayer(int entityId, Objects::G_Player player, int x= 0, int y = -1, bool force = false) {
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
		inline void AddEntity(int entityId, Objects::G_Entity entity) {
			//working on
		};
		inline void RemoveEntity(int entityId) {
			//working on
		};
		inline Objects::coord GetPos(int entityId) {
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
		inline void set_curXY(int x, int y) {
   			 std::cout << "\033[" << x+2 << ";" << y+2 << "H"; return; // extra numbers account for the border around the map.
		};
		inline void reset_cur() {
			this->set_curXY(-3, this->h);
		};
		inline Objects::coord get_xy(int __p1) {
			int __p1_x = __p1 / this->w;
			int __p1_y = __p1 % this->w;
			return { .x=__p1_x,.y=__p1_y };
		};
        inline void draw_info() {
            this->set_curXY(0,0);
            std::cout << "[ pos: " << std::to_string(this->player.coords.x) << "," << std::to_string(this->player.coords.y) << " ] ";
            this->set_curXY(1,0);
            std::cout << "[ unit_color: " << std::to_string(this->playerunit.color) << ", " << std::to_string(this->playerunit.special_clr ? this->playerunit.special_clr : 0) << " ] ";
            this->set_curXY(2,0);
            std::cout << "[ env: " << std::string("$!__GMENG_WMAP ] ");
			this->set_curXY(3,0);
			std::cout << "[ gmeng: " << std::string("version $!__VERSION") << " - build " << std::string("$!__BUILD") << " ]";
			this->set_curXY(4,0);
			std::cout << "[ last engine build: $!__BUILD | current framework: 4.0_glvl ]";
        };
		inline std::string draw_unit(Gmeng::Unit __u) {
			Gmeng::Unit current_unit = __u;
			if (current_unit.is_player) {
				std::string final = "\x1B[4"+Gmeng::colorids[this->playerunit.color]+"m" + (this->playerunit.color == 0 ? Gmeng::colors[8] : Gmeng::colors[current_unit.color]) + current_unit.player.c_ent_tag + Gmeng::resetcolor;
				return final;
			};
			if (current_unit.special) {
				std::string final = "\x1B[4"+Gmeng::colorids[current_unit.color]+"m" + Gmeng::boldcolor + Gmeng::colors[current_unit.special_clr] + current_unit.special_c_unit + Gmeng::resetcolor;
				return final;
			};
            if (this->has_modifier("wireframe_render")) {
                std::string final = "\x1B[4"+std::string(Gmeng::colorids[current_unit.color])+"m" + (current_unit.collidable || this->has_modifier("noclip") ? "x" : "X") + Gmeng::resetcolor;
                return final;
            };
			std::string color = Gmeng::colors[current_unit.color];
			std::string final = color + (Gmeng::c_unit) + Gmeng::resetcolor;
			return final;
		};
		inline void rewrite_mapping(const std::vector<int>& positions) {
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
			std::cout << "\033[2J\033[1;1H";
		};
        inline void set_entTag(std::string __nt) {
            this->player.c_ent_tag = __nt;
        };
        inline std::string get_entTag() {
            return this->player.c_ent_tag;
        };
		inline void rewrite_full() {
			this->clear_screen();
			this->update();
            std::cout << repeatString("\n", 20) << endl;
			std::cout << this->draw() << std::endl;
		};
		inline void MovePlayer(int entityId, int width, int height) {
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
	inline CameraView<_w, _h> UseRenderer(Gmeng::G_Renderer<_w, _h> __r) {
		Gmeng::CameraView<_w, _h> wrldmp;
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
#endif
