#pragma once
/// FOR GMENG_SDL.



#include <SDL2/SDL.h>
#if GMENG_SDLIMAGE
#include <SDL2/SDL_image.h>
#endif
#include <cstdint>

#include "../gmeng.h"
#include "../src/renderer.cpp"
#include "SDL_rect.h"
#include "SDL_ttf.h"

namespace Gmeng {
    /// (Gmeng) SDL Image
    struct sImage {
        int width, height;
        std::vector<color_t> content;
    };

    /// RGB_UINT Colors
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
    /// SDL Colors
    static SDL_Color scolors[9] = {
        {255,255,255  ,255},
        {131, 165, 152,255},
        {184, 187, 38 ,255},
        {134, 180, 117,255},
        {244, 73, 52  ,255},
        {211, 134, 155,255},
        {249, 188, 47 ,255},
        {40, 40, 40   ,255},
        {249, 128, 25 ,255},
    };
};

/// (Gmeng) SDL-Color to uint32 color.
static uint32_t color_to_uint32(const SDL_Color& color) {
    return (color.r << 24) | (color.g << 16) | (color.b << 8) | color.a;
}

/// (Gmeng) Function to create an SDL texture from a vector of SDL_Color / Gmeng::sImage layer
static SDL_Texture* make_texture(SDL_Renderer* renderer, Gmeng::sImage image) {
    uint32_t width = image.width;
    uint32_t height = image.height;
    std::vector<SDL_Color> units;
    for (auto color : image.content) units.push_back(scolors[color]);

    if (units.size() != width * height) return nullptr;

    // Create an SDL surface
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
        units.data(), // Pixel data
        width,        // Width of the surface
        height,       // Height of the surface
        32,           // Bits per pixel
        width * sizeof(uint32_t), // Pitch (bytes per row)
        0xFF000000,   // Red mask
        0x00FF0000,   // Green mask
        0x0000FF00,   // Blue mask
        0x000000FF    // Alpha mask
    );

    if (!surface) {
        std::cerr << "Error creating surface: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    // Create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Free the surface after creating the texture
    if (!texture) {
        std::cerr << "Error creating texture: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    return texture;
}

#if GMENG_SDLIMAGE
/// (Gmeng) returns an SDL Texture from `.png` files.
static SDL_Texture* from_png(SDL_Renderer* renderer, const char* file_path) {
    __functree_call__(gmeng_external::__optional_utils__::libsdl2::from_png);
    // Load the PNG file into an SDL_Surface
    SDL_Surface* surface = IMG_Load(file_path);
    if (!surface) {
        std::cerr << "Error loading image: " << IMG_GetError() << std::endl;
        return nullptr;
    }

    // Create an SDL_Texture from the SDL_Surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Free the surface after creating the texture
    if (!texture) {
        std::cerr << "Error creating texture: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    return texture;
}
#endif

#define GMENG_SDL_ENABLED true
