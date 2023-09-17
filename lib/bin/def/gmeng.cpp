#include <iostream>

#ifdef __GMENG_INIT__

namespace Gmeng {
	template<std::size_t _w, std::size_t _h>
	class WorldMap {
	  public:
		std::size_t w = _w; std::size_t h = _h;
		Objects::G_Entity entitymap[32767] = {};
		Objects::G_Player player = {};
		Gmeng::Unit playerunit = {};
		int entitytotal = 0;
		Gmeng::DisplayMap<_w, _h> display_map; std::string raw_unit_map[32767];

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
		inline void SetPlayer(int entityId, Objects::G_Player player, int x, int y = -1) {
			for (int i = 0; i < this->entitytotal; i++) {
				Objects::G_Entity entity = this->entitymap[i];
				if (entity.entityId == entityId) throw std::invalid_argument("entity already exists: cannot create player");
			};
			int goto_loc = (y != -1) ? ((y*this->w)+x) : (x); 
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
			Gmeng::Unit current_unit = this->display_map.unitmap[(current_coords.y*_w)+current_coords.x];
			if (!exists) throw std::invalid_argument("recieved invalid entityId: no such entity");
			if (move_to_in_map > this->w*this->h) return;
			Gmeng::Unit location_in_map = this->display_map.unitmap[move_to_in_map];
			if (!location_in_map.collidable) return;
			this->playerunit = this->display_map.unitmap[(height*_w)+width];
			this->display_map.unitmap[(current_coords.y*_w)+current_coords.x] = this->playerunit;
			this->display_map.unitmap[move_to_in_map] = Gmeng::Unit{.color=player.colorId,.collidable=false,.is_player=true,.player=entity};
			this->player.coords.x = width;
			this->player.coords.y = height;
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
