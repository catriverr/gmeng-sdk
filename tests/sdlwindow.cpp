#include <iostream>
#include "../lib/bin/gmeng.h"
#include "../lib/bin/utils/serialization.cpp"
#include "../lib/bin/utils/window.cpp"

using namespace Gmeng;

static Gmeng::EventLoop ev;
static Gmeng::Level lvl;

static int VIEWPOINT_WIDTH = 99;
static int VIEWPOINT_HEIGHT = 99;

int main(int argc, char** argv) {
    patch_argv_global(argc, argv); // gmeng init
    _uread_into_vgm("envs/models"); // gmeng texture&models init

    GameWindow window( "GMENG", 800, 600 ); // game window


    read_level_data("envs/proto_level.glvl", lvl); // load level
    ev.level = &lvl;

    ev.level->display.set_resolution(VIEWPOINT_HEIGHT, VIEWPOINT_HEIGHT);
    ev.level->display.viewpoint = { { 0, 0 }, { VIEWPOINT_WIDTH, VIEWPOINT_HEIGHT } };

    
    std::cout << "lol\n";
    return 0;
};
