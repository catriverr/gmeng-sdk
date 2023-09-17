#include <iostream>
#include "utils/textures.cpp"
#include "objects.cpp"

#ifdef __GMENG_OBJECTINIT__
namespace Gmeng {
	struct Coordinate {
		int x; int y;
	};
	std::string colors[] = {
		"\x1B[39m", "\x1B[34m", "\x1B[32m", "\x1B[36m", "\x1B[31m", "\x1B[35m", "\x1B[33m", "\x1B[30m"
	};
	std::string resetcolor = "\033[0m";
	const char c_unit[4] = "\u2588";
	const char c_outer_unit[4] = "\u2584";
	const char c_outer_unit_floor[4] = "\u2580";
	struct Unit {
		public:
			int color; bool collidable;
			bool is_entity = false; bool is_player = false; bool special = false; int special_clr = 0;
			Objects::G_Entity entity={}; Objects::G_Player player={}; std::string special_c_unit = "";
	};
	struct RendererOptions {
		public:
		bool countObjects; bool useTitle;
		std::string title;
	};
	template<std::size_t d_width, std::size_t d_height>
	class DisplayMap {
		public:
		int __h = d_width; int __w = d_height; int pool_size = (sizeof unitmap / 8);
		Gmeng::Unit unitmap[32767] = {};
	};
	template<std::size_t _width, std::size_t _height>
	class Renderer {
		public:
		std::size_t width = _width; std::size_t height = _height;
		int totalObjects; Gmeng::DisplayMap<_width, _height> display; Gmeng::RendererOptions options;
		inline void setUnits(Unit unitmap[_width * _height]) {
			for (int i = 0; i < _width * _height; i++) {
				this->display.unitmap[i] = unitmap[i];
			};
		};
	};
};
inline Gmeng::Unit g_unit(int color = 0, bool collidable = false) {
	return Gmeng::Unit{.collidable=collidable,.color=color};
};
inline Gmeng::Unit g_spike(int color = 0, int bgcolor = 0, bool big = false) {
	return Gmeng::Unit{.collidable=false,.color=color,.special=true,.special_c_unit=(big ? "X" : "x"),.special_clr=bgcolor};
};
#define __GMENG_INIT__ true
#include "def/gmeng.cpp"
#include "utils/envs/map.hpp"
#endif
