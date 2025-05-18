#include <iostream>
#include <thread>
#include <string>
#include <filesystem>
#include <cmath>
#include <algorithm>

#include "Core/World.h"
#include "Core/ArchetypeQuery.h"
#include "Logging/Logger.h"
#include "Profiling/Profiler.h"

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
            SCOPE_CYCLE_COUNTER(MovementSystem);

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
            SCOPE_CYCLE_COUNTER(RenderingSystem);

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

    ecs::Logger* logger = new ecs::Logger();
    logger->Run();

    PROFILER_RUN();

    ECS_LOG(Log, "Initializing ECS instance...");
    std::shared_ptr<ecs::World> world = std::make_shared<ecs::World>();
    world->Initialize();

    // Initialize SDL
    ECS_LOG(Log, "Initializing SDL instance...");
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        ECS_LOG(Error, "SDL could not be initialized!\n"
                "SDL_Error: {}", SDL_GetError());
        return 0;
    }

    // Initialize TTF
    ECS_LOG(Log, "Initializing TTF instance...");
    if(TTF_Init() < 0)
    {
        ECS_LOG(Error, "TTF could not be initialized!\n"
                "TTF_Error: {}", TTF_GetError());
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
        ECS_LOG(Error, "Window could not be created!\n"
                "SDL_Error: {}", SDL_GetError());
        return 0;
    }

    // Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 
          SDL_RENDERER_PRESENTVSYNC 
        | SDL_RENDERER_ACCELERATED);
    if(!renderer)
    {
        ECS_LOG(Error, "Renderer could not be created!\n"
                "SDL_Error: {}", SDL_GetError());
        return 0;
    }

    // Set blend mode for proper alpha blending
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    ECS_LOG(Log, "Running ECS instance...");

    // Setup entities 
    SDL_Rect rect(0, 0, 5, 5);
    SDL_Color color(0, 128, 135, 255);
    ecs::real_t maxVelocity = 500.0f;
    ecs::real_t minVelocity = 100.0f;
    size_t numEntities = 20000;
    srand(static_cast<unsigned> (time(0)));
    const auto generateRandomVelocity = [minVelocity, maxVelocity]()
    {
        ecs::real_t size = minVelocity + static_cast<float>(rand()) 
            / ( static_cast<float> (RAND_MAX/(maxVelocity - minVelocity)));
        float random01 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if (random01 > 0.5f)
        {
            size *= -1.0f;
        }

        return size;
    };

    // Create entities with random positions, velocities, and colors
    auto startTime = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < numEntities; ++i)
    {
        const ecs::entity_id entity = world->CreateEntity<comps::Velocity, comps::Rect, comps::Color>();
        ecs::EntityHandle handle = world->GetEntity(entity);
        rect.x = 512;
        rect.y = 360;
        handle.GetComponent<comps::Rect>().rect = rect;
        handle.GetComponent<comps::Color>().color = color;
        comps::Velocity& velocity = handle.GetComponent<comps::Velocity>();
        velocity.x = generateRandomVelocity();
        velocity.y = generateRandomVelocity();   
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    ECS_LOG(Log, "Created {} entities in {} milliseconds.", numEntities, duration.count());
              // before free list: 11920ms
              // after free list: 163ms

    // Setup systems 
    world->AddSystem<systems::MovementSystem>();
    world->AddSystem<systems::RenderSystem>()->SetRenderer(renderer);
    
    Uint64 previousTime = 0;
    Uint64 currentTime = SDL_GetTicks64();

    TTF_Font* font = TTF_OpenFont("fonts/arial.ttf", 24);
    if (!font)
    {
        ECS_LOG(Error, "Failed to load font: {}", TTF_GetError());
        ECS_LOG(Error, "Current working directory: {}", std::filesystem::current_path().string());
        return 0;
    }

    // Create text texture
    SDL_Color textColor = { 255, 255, 255, 255 };
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Frame rate", textColor);
    if (!textSurface)
    {
        ECS_LOG(Error, "Failed to render text: {}", TTF_GetError());
        return 0;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    if (!textTexture)
    {
        ECS_LOG(Error, "Failed to create texture: {}", SDL_GetError());
        return 0;
    }

    SDL_Rect textLocation = { 800, 100, 0, 0 };
    SDL_QueryTexture(textTexture, NULL, NULL, &textLocation.w, &textLocation.h);

    static constexpr Uint64 targetFrameTime = static_cast<Uint64>((1.0 / 60.0) * 1000);

    while (s_keepUpdating)
    {
        PROFILER_START_NEW_FRAME()

        currentTime = SDL_GetTicks64();
        Uint64 elapsedTicks = currentTime - previousTime;
        const float deltaTime = static_cast<float>(elapsedTicks) / 1000.0f;
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
        Uint64 frameTimeSoFar = SDL_GetTicks64() - currentTime;
        if (frameTimeSoFar < targetFrameTime)
        {
            SDL_Delay(targetFrameTime - frameTimeSoFar);
        }
    }

    PROFILER_STOP();

    // Cleanup
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    ECS_LOG(Log, "Terminating ECS instance...");

    return 0;
}