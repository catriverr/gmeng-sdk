#include <iostream>
#include <string>
#include <array>
#include <cstring>
#include <vector>
#include <algorithm>

#ifdef __GMENG_INIT__

std::string repeatString(const std::string& str, int times) {
    std::string result = "";
    for (int i = 0; i < times; i++) {
        result += str;
    }
    return result;
}


namespace Gmeng {
	template<std::size_t _w, std::size_t _h>
	class WorldMap {
	  public:
		Gmeng::ModifierList modifiers = {
			.values = std::vector<Gmeng::modifier> {
				Gmeng::modifier { .name="noclip",.value=0 },
				Gmeng::modifier { .name="force_update",.value=0 }
			}
		};
		Objects::G_Entity entitymap[32767] = {};
		std::size_t w = _w; std::size_t h = _h;
		Gmeng::DisplayMap<_w, _h> display_map;
		Gmeng::EventHandler event_handler;
		std::string raw_unit_map[32767];
		Objects::G_Player player = {};
		Gmeng::Unit playerunit = {};
		int entitytotal = 0;
		inline void SetResolution(std::size_t w, std::size_t h) {
			display_map.__h = h; display_map.__w = w;
			this->w = w; this->h = h;
		};
		inline void constructor(Unit unitmap[_w*_h]) {
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
					if (current_unit.is_player) {
						std::string final = "\x1B[40m" + Gmeng::colors[current_unit.color] + current_unit.player.c_ent_tag + Gmeng::resetcolor;
						this->raw_unit_map[current_unit_id] = final;
						continue;
					};
					if (current_unit.special) {
						std::string final = "\x1B[4"+std::to_string(current_unit.special_clr)+"m" + Gmeng::colors[current_unit.color] + current_unit.special_c_unit + Gmeng::resetcolor;
						this->raw_unit_map[current_unit_id] = final;
						continue;
					};
					std::string color = Gmeng::colors[current_unit.color];
					std::string final = color + (i == _w-1 ? Gmeng::c_unit : (i == 0 ? Gmeng::c_outer_unit : Gmeng::c_unit)) + Gmeng::resetcolor;
					this->raw_unit_map[current_unit_id] = final;
				};
			};
		};
		inline std::string draw() {
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
		inline void SetPlayer(int entityId, Objects::G_Player player, int x, int y = -1) {
			for (int i = 0; i < this->entitytotal; i++) {
				Objects::G_Entity entity = this->entitymap[i];
				if (entity.entityId == entityId) throw std::invalid_argument("entity already exists: cannot create player");
			};
			int goto_loc = (y != -1) ? ((y*this->w)+x) : (x);
			std::cout << this->w << " " << this->h << " " << goto_loc;
			if (goto_loc > this->w*this->h) throw std::invalid_argument("entity cannot be placed in the provided x-y coordinates");
			if (this->display_map.unitmap[goto_loc].collidable == false) throw std::invalid_argument("entity cannot be placed in the provided x-y coordinates: the unit at the location is not collidable");
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
		inline std::string draw_unit(Gmeng::Unit __u) {
			Gmeng::Unit current_unit = __u;
			if (current_unit.is_player) {
				std::string final = "\x1B[40m" + Gmeng::colors[current_unit.color] + current_unit.player.c_ent_tag + Gmeng::resetcolor;
				return final;
			};
			if (current_unit.special) {
				std::string final = "\x1B[4"+std::to_string(current_unit.special_clr)+"m" + Gmeng::colors[current_unit.color] + current_unit.special_c_unit + Gmeng::resetcolor;
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
			this->reset_cur();
		};
		inline void clear_screen() {
			std::cout << "\033[2J\033[1;1H";
		};
		inline void rewrite_full() {
			this->clear_screen();
			this->update();
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
	inline WorldMap<_w, _h> UseRenderer(Gmeng::Renderer<_w, _h> __r) {
		Gmeng::WorldMap<_w, _h> wrldmp;
		wrldmp.w = __r.width; wrldmp.h = __r.height;
		wrldmp.constructor(__r.display.unitmap);
		for (int i = 0; i < __r.height; i++) {
			for (int j = 0; j < __r.width; j++) {
				int current_unit_id = (i * __r.width) + j;
				Gmeng::Unit current_unit = __r.display.unitmap[current_unit_id];
				if (current_unit.is_player) {
					std::string final = "\x1B[40m" + Gmeng::colors[current_unit.color] + current_unit.player.c_ent_tag + Gmeng::resetcolor;
					wrldmp.raw_unit_map[current_unit_id] = final;
					continue;
				};
				if (current_unit.special) {
					std::string final = "\x1B[4"+std::to_string(current_unit.special_clr)+"m" + Gmeng::colors[current_unit.color] + current_unit.special_c_unit + Gmeng::resetcolor;
					wrldmp.raw_unit_map[current_unit_id] = final;
					continue;
				};
				std::string color = Gmeng::colors[current_unit.color];
				std::string final = color + (i == _w-1 ? Gmeng::c_outer_unit_floor : (i == 0 ? Gmeng::c_outer_unit : Gmeng::c_unit)) + Gmeng::resetcolor;
				wrldmp.raw_unit_map[current_unit_id] = final;
			};
		};
		return wrldmp;
	};
};
#endif