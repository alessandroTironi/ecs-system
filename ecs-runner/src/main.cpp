#include <iostream>
#include <thread>
#include <string>
#include <chrono>
#include <filesystem>

#include "World.h"
#include "ArchetypeQuery.h"

#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

bool s_keepUpdating = true;

namespace comps 
{
    struct Rect : public ecs::IComponent 
    {
        SDL_Rect rect;
    };

    struct Color : public ecs::IComponent 
    {
        SDL_Color color;
    };

    struct Velocity : public ecs::IComponent 
    {
        ecs::real_t x = 0.0f;
        ecs::real_t y = 0.0f;
    };
}

namespace systems 
{
    class MovementSystem : public ecs::ISystem 
    {
        void Update(std::weak_ptr<ecs::World> world, ecs::real_t deltaTime) override
        {
            ecs::query<comps::Rect, comps::Velocity>::MakeQuery(world).forEach(
                [deltaTime](ecs::EntityHandle entity, comps::Rect& rect, comps::Velocity& velocity)
                {
                    rect.rect.x += static_cast<int>(velocity.x * deltaTime);
                    rect.rect.y += static_cast<int>(velocity.y * deltaTime);

                    // Simple bouncing off the edges of the screen
                    if (rect.rect.x < 0 || rect.rect.x > 1024)
                    {
                        velocity.x *= -1.0f;
                        if (rect.rect.x < 0)
                        {
                            rect.rect.x = 3;
                        }
                        else
                        {
                            rect.rect.x = 1021;
                        }
                    }

                    if (rect.rect.y < 0 || rect.rect.y > 720)
                    {
                        velocity.y *= -1.0f;
                        if (rect.rect.y < 0)
                        {
                            rect.rect.y = 3;
                        }
                        else
                        {
                            rect.rect.y = 717;
                        }
                    }
                });
        }
    };

    class RenderSystem : public ecs::ISystem 
    {
    public:
        RenderSystem() {}
        RenderSystem(SDL_Renderer* renderer) : m_renderer(renderer) {}

        void Update(std::weak_ptr<ecs::World> world, ecs::real_t deltaTime) override
        {
            ecs::query<comps::Rect, comps::Color>::MakeQuery(world).forEach(
                [&](ecs::EntityHandle entity, comps::Rect& rect, comps::Color& color)
                {
                    SDL_SetRenderDrawColor(m_renderer, color.color.r, color.color.g, color.color.b, color.color.a);
                    SDL_RenderFillRect(m_renderer, &rect.rect);
                });
        }

        inline void SetRenderer(SDL_Renderer* renderer) { m_renderer = renderer; }

    private:
        SDL_Renderer* m_renderer;
    };
}


int main()
{
    using namespace std::chrono_literals;

    std::cout << "Initializing ECS instance..." << std::endl;
    std::shared_ptr<ecs::World> world = std::make_shared<ecs::World>();

    // Initialize SDL
    std::cout << "Initializing SDL instance..." << std::endl;
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not be initialized!\n"
               "SDL_Error: " << SDL_GetError() << std::endl;
        return 0;
    }

    // Initialize TTF
    std::cout << "Initializing TTF instance..." << std::endl;
    if(TTF_Init() < 0)
    {
        std::cerr << "TTF could not be initialized!\n"
               "TTF_Error: " << TTF_GetError() << std::endl;
        return 0;
    }

    // Create window
    SDL_Window *window = SDL_CreateWindow("ECS Test Project",
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

    // Set blend mode for proper alpha blending
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    std::cout << "Running ECS instance..." << std::endl;

    // Setup entities 
    SDL_Rect rect(0, 0, 5, 5);
    SDL_Color color(0, 128, 135, 255);
    ecs::real_t maxVelocity = 500.0f;
    ecs::real_t minVelocity = 100.0f;
    size_t numEntities = 2000;

    // Create 20 entities with random positions, velocities, and colors
    for (size_t i = 0; i < numEntities; ++i)
    {
        const ecs::entity_id entity = world->CreateEntity<comps::Velocity, comps::Rect, comps::Color>();
        ecs::EntityHandle handle = world->GetEntity(entity);
        rect.x = 512;
        rect.y = 360;
        handle.GetComponent<comps::Rect>().rect = rect;
        handle.GetComponent<comps::Color>().color = color;
        handle.GetComponent<comps::Velocity>().x = -1.0f + (2.0f * static_cast<float>(rand()) / RAND_MAX);
        handle.GetComponent<comps::Velocity>().y = -1.0f + (2.0f * static_cast<float>(rand()) / RAND_MAX);
        handle.GetComponent<comps::Velocity>().x *= maxVelocity;
        handle.GetComponent<comps::Velocity>().x = std::clamp(handle.GetComponent<comps::Velocity>().x, minVelocity, maxVelocity);
        handle.GetComponent<comps::Velocity>().y *= maxVelocity;
        handle.GetComponent<comps::Velocity>().y = std::clamp(handle.GetComponent<comps::Velocity>().y, minVelocity, maxVelocity);  
    }

    // Setup systems 
    world->AddSystem<systems::MovementSystem>();
    world->AddSystem<systems::RenderSystem>()->SetRenderer(renderer);
    
    auto previousTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    const float targetFrameTime = 1.0f / 60.0f;  // Target 60 FPS

    TTF_Font* font = TTF_OpenFont("fonts/arial.ttf", 24);
    if (!font)
    {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        std::cerr << "Current working directory: " << std::filesystem::current_path() << std::endl;
        return 0;
    }

    // Create text texture
    SDL_Color textColor = { 255, 255, 255, 255 };
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Frame rate", textColor);
    if (!textSurface)
    {
        std::cerr << "Failed to render text: " << TTF_GetError() << std::endl;
        return 0;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    if (!textTexture)
    {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        return 0;
    }

    SDL_Rect textLocation = { 800, 100, 0, 0 };
    SDL_QueryTexture(textTexture, NULL, NULL, &textLocation.w, &textLocation.h);

    while (s_keepUpdating)
    {
        currentTime = std::chrono::high_resolution_clock::now();
        const auto elapsedTime = std::chrono::duration<float>(currentTime - previousTime);
        const float deltaTime = elapsedTime.count();
        previousTime = currentTime;

        // Handle events
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                s_keepUpdating = false;
            }
        }

        // Clear screen with blue background
        SDL_SetRenderDrawColor(renderer, 0x0A, 0x0A, 0xFF, 0xFF);
        SDL_RenderClear(renderer);

        // Draw frame rate texture
        std::string fpsText = "FPS: " + std::to_string(static_cast<int>(1.0f / deltaTime));
        std::cout << fpsText << "  (" << deltaTime << ")" <<std::endl;
        SDL_DestroyTexture(textTexture);
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, fpsText.c_str(), textColor);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface);
        SDL_RenderCopy(renderer, textTexture, NULL, &textLocation);

        // Update world
        world->Update(deltaTime);

        // Present the rendered frame
        SDL_RenderPresent(renderer);

        // Cap frame rate
        if (deltaTime < targetFrameTime)
        {
            std::this_thread::sleep_for(std::chrono::duration<float>(targetFrameTime - deltaTime));
        }
    }

    // Cleanup
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "Terminating ECS instance..." << std::endl;

    return 0;
}