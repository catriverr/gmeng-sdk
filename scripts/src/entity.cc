#include "../../lib/bin/easy.h" // gmeng import
#include <vector>

/// GMENG 10.4.0 ENTITY CONTROLLER FOR GMENG GAME INSTANCES


using namespace Gmeng;


extern "C" int gmeng_script_command(EventLoop* ev) {
    ev->level->entities.at(0)->position.x = 0;
    return 0;
};
