#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <cstddef>
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
            std::string font_path = "press_start.ttf";
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

        void draw(const Gmeng::sImage& img, SDL_Point position, int pixelSize = 15) {
            SDL_Texture* txtr = make_texture(this->renderer, img);
            SDL_Rect source_rect = {0, 0, img.width, img.height};
            SDL_Rect destination_rect = {position.x, position.y, img.width*pixelSize, img.height*pixelSize};
            SDL_RenderCopy(renderer, txtr, &source_rect, &destination_rect);
        };

        void text(string message, SDL_Point pos, SDL_Color color, SDL_Color bgcolor = { 0, 0, 0, 255 }) {
            SDL_Surface* text_surface = TTF_RenderText_Solid(this->font, message.c_str(), color);
            SDL_Texture* text_texture = SDL_CreateTextureFromSurface(this->renderer, text_surface);
            SDL_Rect text_rect = { pos.x, pos.y, text_surface->w, text_surface->h };
            SDL_FreeSurface(text_surface);

            SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
        };

        void refresh() {
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
};

#define GMENG_SDL_INIT true
