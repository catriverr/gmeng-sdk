/// GMENG 13.1.0 Developer Console General Utilities Library
/// Attaches to the global <commands> variable of Gmeng, adding
/// many useful new commands to the system.

#include "../../lib/bin/easy.h"
#include <vector>

using namespace Gmeng;

extern "C" int gmeng_script_command(EventLoop* ev) {
    /// Initial logging
    GAME_LOG("the NOBLE gmeng:scripts:devc_util");
    GAME_LOG("successfully attached to the Gmeng instance.");
};
