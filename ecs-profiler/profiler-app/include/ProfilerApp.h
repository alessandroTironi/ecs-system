#pragma once 

#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Session.h"

class ProfilerApp
{
public:
    ProfilerApp();
    ~ProfilerApp();

    void Run();
private:
    void RenderMainWindow();
    void DrawTimeline(const ecs::profiling::frame_data_t& frameData, double frameTimeMs = 16.67);

    GLFWwindow* m_window;
    std::shared_ptr<ecs::profiling::Session> m_session;
};