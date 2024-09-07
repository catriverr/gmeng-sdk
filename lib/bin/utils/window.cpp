#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <stdexcept>
#include <vector>
#include <iostream>
#include <map>

#include "../gmeng.h"
#include "../src/renderer.cpp"

#include "../types/window.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define FILE_DIRECTORY(file) \
    (std::string(STR(file)).substr(0, std::string(STR(file)).find_last_of("/\\")))


namespace Gmeng {
    class GameWindow {
    public:
        GMENG_NULL_T __ = { not_nullptr };
        GameWindow(const char* title, int width, int height)
            : window(nullptr), renderer(nullptr), width(width), height(height) {
            if (IS_SET PREF("pref.max_screens_reached")) {
                gm_log("wont proceed with Window creation, max screens reached");
                return;
            };
            if (SDL_Init(SDL_INIT_VIDEO) < 0) {
                SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
                return;
            };
            if (TTF_Init() != 0) {
                throw std::runtime_error("TTF_Init Error: " + std::string(TTF_GetError()));
            };

            window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
            if (!window) {
                SDL_Log("Window could not be created! SDL_Error: %s", SDL_GetError());
                SDL_Quit();
                return;
            };

            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if (!renderer) {
                SDL_Log("Renderer could not be created! SDL_Error: %s", SDL_GetError());
                SDL_DestroyWindow(window);
                SDL_Quit();
                return;
            };
            std::string font_path = "./assets/press_start.ttf";
            font = TTF_OpenFont(font_path.c_str(), 24); // Default font and size
            if (!font) {
                throw std::runtime_error("TTF_OpenFont Error: " + std::string(TTF_GetError()));
            };
        };

        ~GameWindow() {
            TTF_Quit();
            if (renderer) SDL_DestroyRenderer(renderer);
            if (window) SDL_DestroyWindow(window);
            SDL_Quit();
        };

        void draw(const Gmeng::sImage& img, SDL_Point position, int pixelSize = 10) {
            this->clear();
            for (int y = 0; y < img.height; ++y) {
                for (int x = 0; x < img.width; ++x) {
                    int index = y * img.width + x;
                    color_t unit = RED;
                    try {
                        unit = img.content.at(index);
                    } catch (std::out_of_range& e) {
                        gm_log("!! buffer_overflow (" + v_str(index) + ")");
                    };
                    auto& color = rgb_colors[unit];
                    SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], 255);
                    SDL_Rect rect = {
                        position.x + x * pixelSize,
                        position.y + y * pixelSize,
                        pixelSize,
                        pixelSize
                    };
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
            SDL_RenderPresent(renderer);
        };

        void clear() {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
        }
    private:
        TTF_Font* font;
        SDL_Window* window;
        SDL_Renderer* renderer;
        int width, height;
    };

    inline GameWindow create_window(const char* title, int width, int height) {
        gm_log("[SDL_FRAME] window creation");
        if (IS_SET PREF("pref.max_screens_reached")) {
            gm_log("[SDL_FRAME] max screens reached assertion is jWRAP::ON, set to OFF or NOT_SET to create this screen. Cancelling GameWindow[constructor]->e");
            return GameWindow("UNUSABLE_WINDOW",0,0);
        };
        if (IS_SET PREF("pref.screens")) gm_log("[SDL_FRAME] a previous window was already created, possible loop\n assert `pref.max_screens_reached` to ON to disable creating more windows");
        ASSERT("pref.screens", Assertions::vd_assert::ON);
        GameWindow w(title, width, height);
        return w;
    };

    inline sImage window_frame(Gmeng::Level& lvl) {
        ASSERT("pref.log", p_yes);
        sImage image;
        image.width = lvl.base.width;
        image.height = lvl.base.height;
        for (int indx = 0; indx < image.width*image.height; indx++) {
            Unit fd = { .color = ((indx % 2) + ( (indx % image.height) % 2 == 0 ? 0 : 1 ) == 0 ? RED : BLACK) };
            if (indx < CONSTANTS::UNITMAP_SIZE) fd = lvl.display.camera.display_map.unitmap[indx];
            image.content.push_back((color_t)fd.color);
            gm_log("losing your mind? unit(" + v_str(indx) + ") color: " + v_str(fd.color));
        };
        return image;
    };
};

#define GMENG_SDL_INIT true
