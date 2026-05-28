#include <SDL.h>
#include <iostream>
#include "config.hpp"
#include "game.hpp"

int main(int argc, char *argv[])
{
    // Prevent compiler warnings about unused variables
    (void)argc;
    (void)argv;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create Window
    SDL_Window *window = SDL_CreateWindow(
        "NeonVoid",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);

    if (!window)
    {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create Renderer with VSync enabled for smooth updates on high refresh rate displays
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer)
    {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        // Fallback to software renderer if hardware accelerated fails
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        if (!renderer)
        {
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }
    }

    // Initialize Game Engine
    Game game;
    if (!game.init(window, renderer))
    {
        std::cerr << "Failed to initialize Game Engine!" << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Setup loop variables
    Uint32 lastTime = SDL_GetTicks();
    const int MAX_FPS = 250;
    const int MIN_FRAME_TIME = 1000 / MAX_FPS;

    // Main game loop
    while (game.isRunning())
    {
        Uint32 frameStart = SDL_GetTicks();

        // Calculate delta time
        Uint32 currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // Clamp dt to avoid physics bugs during pauses/lag spikes
        if (dt > 0.1f)
            dt = 0.1f;

        // Process inputs
        game.handleEvents();

        // Update physics/gameplay
        game.update(dt);

        // Draw screen
        game.render();

        // Safety cap to prevent GPU/CPU 100% spin if VSync is disabled or fails
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < static_cast<Uint32>(MIN_FRAME_TIME))
        {
            SDL_Delay(MIN_FRAME_TIME - frameTime);
        }
    }

    // Cleanup SDL resources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "NeonVoid shut down successfully. Goodbye!" << std::endl;
    return 0;
}
