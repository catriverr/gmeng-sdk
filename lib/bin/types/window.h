#pragma once

#include <SDL2/SDL.h>

#include "../gmeng.h"
#include "../src/renderer.cpp"
#include "SDL_rect.h"
#include "SDL_ttf.h"

namespace Gmeng {
    struct sImage {
        int width, height;
        std::vector<color_t> content;
    };

    static int rgb_colors[9][3] = {
        {255,255,255},
        {131, 165, 152},
        {184, 187, 38},
        {134, 180, 117},
        {244, 73, 52},
        {211, 134, 155},
        {249, 188, 47},
        {40, 40, 40},
        {249, 128, 25}
    };
};



#define GMENG_SDL_ENABLED true
