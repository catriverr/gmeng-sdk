/* modules */
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
/* files */
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
	Gmeng::WorldMap<_w, _h> world = Gmeng::MapParser::LoadAsset<1, 1>("world.mpd", wdata._w, wdata._h);
	world.SetResolution(wdata._w, wdata._h);
	world.SetPlayer(0, Objects::G_Player {.c_ent_tag = wdata.player.c_ent_tag, .colored = wdata.player.colored, .colorId=0, .entityId=0, .textured=false, .textureId=0 }, wdata.player.startDX, wdata.player.startDY);
	world.update();
	std::cout << world.draw() << endl;
    std::string line;
    int number;

    // Read data from stdin
    std::vector<std::string> commandList = {
		"r_update", 	 "echo",	  "p_setpos", "kb_help", "gm_modstatus",
		"p_coordinfo",   "gm_modify", "gm_quit",  "kb_resetcur", "help",
	};
    while (true) {
        if (std::getline(std::cin, line)) {
			if (startsWith(line, "[posy] ")) {
				// vectoral updates ( screen updates will not reset the screen entirely, it will only change the current pixels - performance boost by sizexy / sizex % )
				// vectoral updates are enabled by default, as it saves a lot of performance and computing time.
				// disabling vectoral updates will slow the game down. Down to 0.7fps. Tried on the following GPUs:
				// - MSI GTX 1660 : 0.4 fps
				// - Apple M1 : 4 fps
				// - MSIRTX 3060 : 5 fps
				// - Apple M1 Max : 7 fps
				// - MSI RTX 4090: 15 fps
				// these are not good fps rates, leave this option enabled
				// however if you disable it, your gameplay will be improved.
				// DISABLING IS ABSOLUTELY NOT RECOMMENDED FOR MAPS BIGGER THAN 200x50.
				// to disable, run the command:
				// gm_modify force_update 1
                // on the developer console (shift+tab to open)
				std::vector<std::string> p_coordY = g_splitStr(line, " ");
				if (p_coordY[1] == "i1") world.MovePlayer(0, world.player.coords.x, world.player.coords.y-1);
				if (p_coordY[1] == "d1") world.MovePlayer(0, world.player.coords.x, world.player.coords.y+1);
				continue;
			}
			if (startsWith(line, "[posx] ")) {
				std::vector<std::string> p_coordX = g_splitStr(line, " ");
				if (p_coordX[1] == "i1") world.MovePlayer(0, world.player.coords.x+1, world.player.coords.y);
				if (p_coordX[1] == "d1") world.MovePlayer(0, world.player.coords.x-1, world.player.coords.y);
				continue;
			}
			if (!startsWith(line, "[dev-c] ")) continue;
			std::vector<std::string> command = g_splitStr(g_joinStr(g_splitStr(line, "[dev-c] "), ""), " ");
			std::cout << Gmeng::colors[6];
			if (command[0] == "r_update") {
				world.update();
				std::cout << "\033[2J\033[1;1H";
				std::cout << world.draw() << endl;
				continue;
			};
			world.set_curXY(46, -1);
			if (command[0] != "echo") std::cout << Gmeng::colors[6] << "[gmeng:0/core] recieved command: " << Gmeng::colors[1] << g_joinStr(command, " ") << Gmeng::colors[6] << "." << endl;
			world.event_handler.cast_ev(Gmeng::CONSTANTS::C_PlugEvent,
				world.event_handler.gen_estr(
					Gmeng::event {
						.id=Gmeng::CONSTANTS::PE_Type1,
						.name="command_ran",
						.params={ g_joinStr(command, " ") }
					})
			);
			if (command[0] == "help") {
				std::cout << "Gmeng SDK Developer Console" <<endl;
				std::cout << "list of available commands:" <<endl;
				std::cout << g_joinStr(commandList, ", ")  <<endl;
			}
			if (command[0] == "kb_resetcur") {
				world.reset_cur(); continue;
			}
			if (command[0] == "p_coordinfo") {
				std::cout << "dY:" << world.player.coords.y << " dX:" << world.player.coords.x <<endl;
			}
			if (command[0] == "p_setpos") {
				std::vector<std::string> rPos = g_splitStr(command[1], ",");
				int x = std::stoi(rPos[0]); int y = std::stoi(rPos[1]);
				world.MovePlayer(0, y,x);
				world.update();
				std::cout << "[gmeng:0/core] entity[0](player) moved to x,y: " << rPos[0] << "," << rPos[1] << endl;
			};
			if (command[0] == "gm_modstatus") {
				std::cout << " < Gmeng Modifiers status list >" << endl;
				int i = 0; int activec = 0;
				for ( auto mod : world.modifiers.values ) {
					if (mod.value == 1) activec++;
					std:cout << std::to_string(i) << " " << mod.name << " -> " << std::to_string(mod.value) << endl;
					i++;
				};
				std::cout << "total -> " << std::to_string(i) << ", active -> " << std::to_string(activec) << endl;
			};
			if (command[0] == "kb_help") {
				std::cout << "< Gmeng keybind list >" << endl;
				std::cout << "All keybindings are controlled by GmengSDK's method: " << Gmeng::colors[1] << "TSGmeng::HandleKeyPress" << Gmeng::colors[6] << endl;
				std::cout << "while UI elements such as dev-c are open, keybinds are disregarded. [r_setui controls the variable 'inmenu' that shows the UI data.]" << endl;
				std::cout << "\n\n[W, A, S, D] Move entity[0](player) around / navigate entity 0." << endl;
				std::cout << "[shift+tab] show DEV-C Developer Command-line tools" << endl;
				std::cout << "[shift+f2] force execute r_update / force-render WorldMap\n" << endl;
			}
			if (command[0] == "echo") {
				std::cout << g_splitStr(g_joinStr(command, " "), "echo ")[1] << endl;
			}
			if (command[0] == "gm_modify") {
                int oldvalue = 0;
                if (world.has_modifier(command[1])) oldvalue = g_find_modifier(world.modifiers.values, command[1]);
				world.set_modifier(command[1], std::stoi(command[2]));
                g_setTimeout([&]() {
                    world.event_handler.cast_ev(Gmeng::CONSTANTS::C_PlugEvent,
                        world.event_handler.gen_estr(Gmeng::event {
                            .id=Gmeng::CONSTANTS::PE_Type2,
                            .name="modifier_change",
                            .params = {
                               command[1], // modifier name
                               std::to_string(oldvalue), // old value
                               command[2] // new value
                        }
                    }));
                }, 10);
				std::cout << "Gmeng::WorldMap::ModifierList Gmeng::WorldMap::modifiers at index (0) / value of " << Gmeng::colors[1] << command[1] << Gmeng::colors[6] << " was changed to " << Gmeng::colors[2] << command[2] << Gmeng::colors[6] << endl;
			}
			if (command[0] == "gm_quit") {
				std::cout << "[gmeng:0/core] executing { r_setui 0; gm_force return_console; gm_kill 0; }" << endl;
				exit(0);
			}
			if (std::find(commandList.begin(), commandList.end(), command[0]) == commandList.end()) {
				std::cout << Gmeng::colors[4] << "error: " << Gmeng::colors[1] << command[0] << Gmeng::colors[4] << " is not a valid command." << Gmeng::colors[6] << endl;
			}
			std::cout << "[gmeng:0/core] press any key to execute: { r_update & r_setui 0 & kb_controller gmeng:1/player }" << endl;
        }
    }
	return 0;
};
