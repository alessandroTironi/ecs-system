#include "Profiling/ProfilerGraphicsApp.h"

#include <iostream>
#include <SDL.h>
#include <SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

using namespace ecs::profiling::gui;

void ProfilerGraphicsApp::Open()
{
    if (!m_running.load(std::memory_order_relaxed))
    {
        m_running.store(true, std::memory_order_relaxed);
    
        m_guiThread = std::thread(&ProfilerGraphicsApp::MainLoop, this, &m_sharedState);
    }
}

void ProfilerGraphicsApp::Close()
{
    if (m_running.load(std::memory_order_relaxed))
    {
        m_running.store(false, std::memory_order_relaxed);

        if (m_guiThread.joinable())
        {
            m_guiThread.join();
        }
    }
}

void ProfilerGraphicsApp::SetupWindow()
{

}

void ProfilerGraphicsApp::MainLoop(shared_state_t* sharedState)
{
    
}