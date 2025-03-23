#include <iostream>
#include <thread>
#include <string>
#include <chrono>

#include "World.h"
#include "ArchetypeQuery.h"

#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "SDL_image.h"

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
                    }

                    if (rect.rect.y < 0 || rect.rect.y > 720)
                    {
                        velocity.y *= -1.0f;
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

    std::cout << "Running ECS instance..." << std::endl;

    // Setup entities 
    SDL_Rect rect(0, 0, 5, 5);
    SDL_Color color(0, 128, 135, 255);
    ecs::real_t maxVelocity = 200.0f;
    size_t numEntities = 20;

    // Create 20 entities with random positions, velocities, and colors
    for (size_t i = 0; i < numEntities; ++i)
    {
        const ecs::entity_id entity = world->CreateEntity<comps::Velocity, comps::Rect, comps::Color>();
        ecs::EntityHandle handle = world->GetEntity(entity);
        rect.x = rand() % 1024; 
        rect.y = rand() % 720;
        handle.GetComponent<comps::Rect>().rect = rect;
        handle.GetComponent<comps::Color>().color = color;
        handle.GetComponent<comps::Velocity>().x = -1.0f + (2.0f * static_cast<float>(rand()) / RAND_MAX);
        handle.GetComponent<comps::Velocity>().y = -1.0f + (2.0f * static_cast<float>(rand()) / RAND_MAX);
        handle.GetComponent<comps::Velocity>().x *= maxVelocity;
        handle.GetComponent<comps::Velocity>().y *= maxVelocity;
    }

    // Setup systems 
    world->AddSystem<systems::MovementSystem>();
    world->AddSystem<systems::RenderSystem>()->SetRenderer(renderer);

    auto previousTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    while (s_keepUpdating)
    {
        currentTime = std::chrono::high_resolution_clock::now();
        const auto elapsedTime = std::chrono::duration<float>(currentTime - previousTime);
        const float deltaTime = elapsedTime.count();
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

        world->Update(deltaTime);

        // Update screen
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "Terminating ECS instance..." << std::endl;

    return 0;
}