#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h> //This is an sample library not included with stock SDL2. https://www.libsdl.org/projects/SDL_ttf/release-1.2.html

const char* WINDOW_TITLE = "Hello World SDL2 + TTF";
const char* FONT_NAME = "roboto.ttf";
const int FONT_SIZE = 24;
const int WINDOW_WIDTH = 1280, WINDOW_HEIGHT = 720;

SDL_Window* Window; // Window created by SDL.
SDL_Renderer* Renderer; // The renderer that shows our textures.
SDL_Event WindowEvent; // Event capturer from SDL Window.
SDL_Color TextColor = { 255, 0, 0, 255}; // Red SDL color.
TTF_Font* Font; // The font to be loaded from the ttf file.
SDL_Surface* TextSurface; // The surface necessary to create the font texture.
SDL_Texture* TextTexture; // The font texture prepared for render.
SDL_Rect TextRect; // Text rectangle area with the position for the texture text.

void CreateWindow() {
    Window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
    if (!Window)
        std::cout << "There was a problem creating the window.";
    Renderer = SDL_CreateRenderer(Window, -1, 0);
    if (!Renderer)
        std::cout << "There was a problem creating the renderer.";
}

void CreateText(const char* Message, int x, int y) {
    TTF_Init();
    TTF_Font *font = TTF_OpenFont(FONT_NAME, FONT_SIZE);
    if (!font)
        std::cout << "Couldn't find/init open ttf font." << std::endl;
    TextSurface = TTF_RenderText_Solid(font, Message, TextColor);
    TextTexture = SDL_CreateTextureFromSurface(Renderer, TextSurface);
    TextRect.x = x; // Center horizontaly
    TextRect.y = y; // Center verticaly
    TextRect.w = TextSurface->w;
    TextRect.h = TextSurface->h;
    // After you create the texture you can release the surface memory allocation because we actually render the texture not the surface.
    SDL_FreeSurface(TextSurface);
    TTF_Quit();
}

bool IsPollingEvent() {
    while(SDL_PollEvent(&WindowEvent)) {
        switch (WindowEvent.type) {
            case SDL_QUIT: return false;
        }
    }
    return true;
}

void RenderText() {
    SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255); // Make window bg black.
    SDL_RenderClear(Renderer); // Paint screen black.
    SDL_RenderCopy(Renderer, TextTexture, NULL, &TextRect); // Add text to render queue.
    SDL_RenderPresent(Renderer); // Render everything that's on the queue.
    SDL_Delay(10); // Delay to prevent CPU overhead as suggested by the user `not2qubit`
}

void ClearMemory() {
    SDL_DestroyTexture(TextTexture);
    SDL_DestroyRenderer(Renderer);
    SDL_DestroyWindow(Window);
    SDL_Quit();
    std::cout << "Clear proccess done." << std::endl;
}

int main() {
    CreateWindow();
    CreateText("Hello SDL_Ttf", 0, 0);
    while (IsPollingEvent()) {
        RenderText();
    }
    ClearMemory();
    return EXIT_SUCCESS;
}
