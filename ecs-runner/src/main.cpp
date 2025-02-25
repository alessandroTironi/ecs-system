#include <iostream>
#include <thread>
#include <string>
#include <chrono>

#include "ECSInstance.h"

#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "SDL_image.h"

bool s_keepUpdating = true;

void StopAfter10Seconds()
{
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(5000ms);

    s_keepUpdating = false;
}

int main()
{
    using namespace std::chrono_literals;

    std::cout << "Initializing ECS instance..." << std::endl;
    ecs::Instance instance;
    instance.Initialize();

    // Initialize SDL
    std::cout << "Initializing SDL instance..." << std::endl;
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not be initialized!\n"
               "SDL_Error: " << SDL_GetError() << std::endl;
        return 0;
    }

    // Create window
    SDL_Window *window = SDL_CreateWindow("Basic C SDL project",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          1024, 720,
                                          SDL_WINDOW_SHOWN);
                                          if(!window)
    if (window == nullptr)
    {
        std::cerr << "Window could not be created!\n"
               "SDL_Error: "<< SDL_GetError() << std::endl;
        return 0;
    }

    // Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer)
    {
        std::cerr << "Renderer could not be created!\n"
                "SDL_Error: " << SDL_GetError() << std::endl;
        return 0;
    }

    std::cout << "Running ECS instance..." << std::endl;

    auto previousTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    while (s_keepUpdating)
    {
        currentTime = std::chrono::high_resolution_clock::now();
        const auto elapsedTime = std::chrono::duration<float>(currentTime - previousTime);
        const float deltaTime = elapsedTime.count();
        instance.Update(deltaTime);

        previousTime = currentTime;

        SDL_Event e;

        // Wait indefinitely for the next available event
        SDL_WaitEvent(&e);

        // User requests quit
        if(e.type == SDL_QUIT)
        {
            s_keepUpdating = false;
        }

        // Initialize renderer color white for the background
        SDL_SetRenderDrawColor(renderer, 0x0A, 0x0A, 0xFF, 0xFF);

        // Clear screen
        SDL_RenderClear(renderer);

        // Set renderer color red to draw the square
        SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);

        // Update screen
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "Terminating ECS instance..." << std::endl;

    return 0;
}