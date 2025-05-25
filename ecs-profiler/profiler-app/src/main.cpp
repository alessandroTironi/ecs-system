#include <iostream>

#include "CycleCounter.h"
#include "Session.h"

int main()
{
    std::cout << "Starting Profiler App..." << std::endl;

    ecs::profiling::Session* session = ecs::profiling::Session::CreateFromFile("profiler-session.bin");
    
    for (const ecs::profiling::frame_data_t& frameData : session->GetFrameData())
    {
        std::cout << frameData.to_string() << std::endl;
    }

    return EXIT_SUCCESS;
}