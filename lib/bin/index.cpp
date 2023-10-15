/* modules */
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
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
    std::string line;
    int number;

    // Read data from stdin
    while (true) {
        std::vector<std::string> commandList = {
		"r_update", "echo", "p_setpos", "kb_help", "p_showDCoords", "gm_modify", "gm_quit"
	};
        if (std::getline(std::cin, line)) {
			if (startsWith(line, "[posy] ")) {
				std::vector<std::string> p_coordY = g_splitStr(line, " ");
				if (p_coordY[1] == "i1") test.MovePlayer(0, test.player.coords.x, test.player.coords.y-1);
				if (p_coordY[1] == "d1") test.MovePlayer(0, test.player.coords.x, test.player.coords.y+1);
				test.update();
				std::cout << "\033[2J\033[1;1H";
				std::cout << test.draw() << endl;
				continue;
			}
			if (startsWith(line, "[posx] ")) {
				std::vector<std::string> p_coordX = g_splitStr(line, " ");
				if (p_coordX[1] == "i1") test.MovePlayer(0, test.player.coords.x+1, test.player.coords.y);
				if (p_coordX[1] == "d1") test.MovePlayer(0, test.player.coords.x-1, test.player.coords.y);
				test.update();
				std::cout << "\033[2J\033[1;1H";
				std::cout << test.draw() << endl;
				continue;
			}
			if (!startsWith(line, "[dev-c] ")) continue;
			std::vector<std::string> command = g_splitStr(g_joinStr(g_splitStr(line, "[dev-c] "), ""), " ");
			std::cout << Gmeng::colors[6];
			if (command[0] == "r_update") {
				test.update();
				std::cout << "\033[2J\033[1;1H";
				std::cout << test.draw() << endl;
				continue;
			};
			if (command[0] == "p_showDCoords") {
				std::cout << "dY:" << test.player.coords.y << " dX:" << test.player.coords.x <<endl;
			}
			if (command[0] != "echo") std::cout << Gmeng::colors[6] << "[gmeng:0/core] recieved command: " << Gmeng::colors[1] << g_joinStr(command, " ") << Gmeng::colors[6] << "." << endl;			
			if (command[0] == "p_setpos") {
				std::vector<std::string> rPos = g_splitStr(command[1], ",");
				int x = std::stoi(rPos[0]); int y = std::stoi(rPos[1]);
				test.MovePlayer(0, y,x);
				test.update();
				std::cout << "[gmeng:0/core] entity[0](player) moved to x,y: " << rPos[0] << "," << rPos[1] << endl;
			};
			if (command[0] == "kb_help") {
				std::cout << "< Gmeng keybind list >" << endl;
				std::cout << "All keybindings are controlled by GmengSDK's method: " << Gmeng::colors[1] << "TSGmeng::HandleKeyPress" << Gmeng::colors[6] << endl;
				std::cout << "while UI elements such as dev-c are open, keybinds are disregarded. [r_setui controls the variable 'inmenu' that shows the UI data.]" << endl;
				std::cout << "\n\n[W, A, S, D] Move entity[0](player) around / navigate entity 0." << endl;
				std::cout << "[shift+f1] show DEV-C Developer Command-line tools" << endl;
				std::cout << "[shift+f2] force execute r_update / force-render WorldMap\n" << endl;
			}
			if (command[0] == "echo") {
				std::cout << g_splitStr(g_joinStr(command, " "), "echo ")[1] << endl;
			}
			if (command[0] == "gm_modify") {
				test.set_modifier(command[1], std::stoi(command[2]));
				std::cout << "Gmeng::WorldMap::ModifierList Gmeng::WorldMap::modifiers at index (0) / value of " << Gmeng::colors[1] << command[1] << Gmeng::colors[6] << " was changed to" << Gmeng::colors[2] << command[2] << Gmeng::colors[6] << endl;
			}
			if (command[0] == "gm_quit") {
				exit(0);
			}
			if (std::find(commandList.begin(), commandList.end(), command[0]) == commandList.end()) {
				std::cout << Gmeng::colors[4] << "error: " << Gmeng::colors[1] << command[0] << Gmeng::colors[4] << " is not a valid command." << Gmeng::colors[6] << endl;
			}
			std::cout << "[gmeng:0/core] press any key to execute: { r_update & r_setui 0 & kb_controller gmeng:1/player }" << endl;
        }
    }
	return 0;
	for ( ;; ) { 
		test.update();
		std::cout << "\033[2J\033[1;1H";
		std::cout << test.draw() << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	};
	return 0;
};

int main1(int argc, char** argv) {

		cout << "yo";
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
