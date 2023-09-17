/* modules */
#include <iostream>
#include <chrono>
#include <thread>
/* files */
#include "utils/conio.hpp"
#include "objects.cpp"
#include "gmeng.hpp"
#include "utils/textures.cpp"
#include "utils/envs/map.hpp"
/* index */
// this file is currently used for testing purposes
using std::endl;

int main( int argc, char** argv ) {
	Gmeng::ModifiedWorldData wdata = Gmeng::MapParser::GetInfo("world.dat", "player.dat");
	const std::size_t _w = 1; const std::size_t _h = 1;
	Gmeng::WorldMap<_w, _h> test = Gmeng::MapParser::LoadAsset<1, 1>("world.mpd", wdata._w, wdata._h);
	test.SetResolution(wdata._w, wdata._h);
	test.SetPlayer(0, Objects::G_Player {.c_ent_tag = wdata.player.c_ent_tag, .colored = wdata.player.colored, .colorId=wdata.player.colorId, .entityId=0, .textured=false, .textureId=0 }, wdata.player.startDX, wdata.player.startDY);
	test.update();
	std::cout << test.draw() << endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	return 0;
};

int main1(int argc, char** argv) {

	Gmeng::Renderer<10, 10> renderer;
	Gmeng::Unit test[100] = {
		g_unit(1), g_unit(1), g_unit(1), g_unit(1), g_unit(1), g_unit(1), g_unit(1), g_unit(1), g_unit(1), g_unit(1), 
		g_unit(1), g_unit(7, true), g_unit(7, true), g_spike(4, 0, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(1), 
		g_unit(1), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(1), 
		g_unit(1), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(1), 
		g_unit(1), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(1), 
		g_unit(1), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(1), 
		g_unit(1), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(1), 
		g_unit(1), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(1), 
		g_unit(1), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(7, true), g_unit(1), 
		g_unit(1), g_unit(1), g_unit(1), g_unit(1), g_unit(1), g_unit(1), g_unit(1), g_unit(1), g_unit(1), g_unit(1), 
	};
	renderer.setUnits(test);
	Gmeng::WorldMap<10, 10> map = Gmeng::UseRenderer(renderer);
	Objects::G_Player player = {.c_ent_tag="o",.colored=true,.colorId=2,.entityId=0,.textured=false,.textureId=0};
	map.SetPlayer(0, player, 2, 2);
	for ( ;; ) {
		map.update();
		std::cout << "\033[2J\033[1;1H";
		std::cout << map.draw() << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		int keypress = getch();
		if (keypress == 9) { // TAB
			std::cout << "exit game." << std::endl;
			break; 
		};
		if (keypress == 119) { // W
			Objects::coord coords = map.player.coords;
			if (map.display_map.unitmap[((coords.y-1)*map.w)+coords.x].special_c_unit == "X") { std::cout << "game over" << std::endl; break;}
			map.MovePlayer(0, coords.x, coords.y-1);
		};
		if (keypress == 115) { // S
			Objects::coord coords = map.player.coords;
			if (map.display_map.unitmap[((coords.y+1)*map.w)+coords.x].special_c_unit == "X") { std::cout << "game over" << std::endl; break;}
			map.MovePlayer(0, coords.x, coords.y+1);
		};
		if (keypress == 97) { // A
			Objects::coord coords = map.player.coords;
			if (map.display_map.unitmap[((coords.y)*map.w)+coords.x-1].special_c_unit == "X") { std::cout << "game over" << std::endl; break;}
			map.MovePlayer(0, coords.x-1, coords.y);
		};
		if (keypress == 100) { // D
			Objects::coord coords = map.player.coords;
			if (map.display_map.unitmap[((coords.y)*map.w)+coords.x+1].special_c_unit == "X") { std::cout << "game over" << std::endl; break;}
			map.MovePlayer(0, coords.x+1, coords.y);
		};
	};
	return 0;
};
