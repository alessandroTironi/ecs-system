#include <iostream>
#include <thread>
#include <string>
#include <chrono>

#include "ECSInstance.h"

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

    std::cout << "Running ECS instance..." << std::endl;
    
    std::thread terminationThread(StopAfter10Seconds);
    terminationThread.detach();

    auto previousTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    while (s_keepUpdating)
    {
        currentTime = std::chrono::high_resolution_clock::now();
        const auto elapsedTime = std::chrono::duration<float, std::milli>(currentTime - previousTime);
        const float deltaTime = elapsedTime.count() * 0.001f;
        instance.Update(deltaTime);

        previousTime = currentTime;
    }

    std::cout << "Terminating ECS instance..." << std::endl;
    if (terminationThread.joinable())
    {
        terminationThread.join();
    }

    return 0;
}