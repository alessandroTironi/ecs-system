#include "ProfilerApp.h"
#include <iostream>
#include "ImGuiFileDialog.h"


ProfilerApp::ProfilerApp()
{

}

ProfilerApp::~ProfilerApp()
{

}

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void ProfilerApp::Run()
{
    // Setup GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) 
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    m_window = glfwCreateWindow(1280, 720, "Profiler", nullptr, nullptr);
    if (m_window == nullptr) 
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool show_demo_window = true;
    bool show_test_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    float counter_value = 0.0f;
    int counter_int = 0;

    std::cout << "Dear ImGui Test Application Started" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // Main loop
    while (!glfwWindowShouldClose(m_window)) 
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        RenderMainWindow();

        /*
        // 1. Show the big demo window
        if (show_demo_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }
        
        // 2. Show a simple test window
        if (show_test_window) {
            ImGui::Begin("Test Window", &show_test_window);

            ImGui::Text("This is a test of Dear ImGui with OpenGL!");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                       1000.0f / io.Framerate, io.Framerate);

            ImGui::Separator();

            // Some basic widgets
            ImGui::Checkbox("Show Demo Window", &show_demo_window);
            
            ImGui::SliderFloat("Float Counter", &counter_value, 0.0f, 100.0f);
            if (ImGui::Button("Increment Int Counter"))
            {
                counter_int++;
            }

            ImGui::SameLine();
            ImGui::Text("Counter = %d", counter_int);

            ImGui::Separator();

            // Color picker
            ImGui::ColorEdit3("Clear Color", (float*)&clear_color);

            ImGui::Separator();

            // Display some system info
            ImGui::Text("Dear ImGui version: %s", IMGUI_VERSION);
            ImGui::Text("OpenGL version: %s", glGetString(GL_VERSION));
            ImGui::Text("Window size: %.0fx%.0f", io.DisplaySize.x, io.DisplaySize.y);
            
            if (ImGui::Button("Quit Application"))
                glfwSetWindowShouldClose(m_window, true);

            ImGui::End();
        }
        */

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, 
                     clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void ProfilerApp::RenderMainWindow()
{
    static bool showTimeline = true;
    static bool showStatistics = false;
    static bool showSettings = false;
    static bool enablePausing = false;
    static float targetFrameTime = 16.67f;
    static int selectedFrameHistory = 0;

    // Get the main viewport and set window to fill it
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_NoTitleBar;
    windowFlags |= ImGuiWindowFlags_NoCollapse;
    windowFlags |= ImGuiWindowFlags_NoResize;
    windowFlags |= ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_MenuBar;
    
    if (ImGui::Begin("Frame Profiler", nullptr, windowFlags)) {
        // Menu Bar
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Save Profile Data", "Ctrl+S")) 
                {
                    // In a real implementation, save profiler data to file
                }
                if (ImGui::MenuItem("Load Profile Data", "Ctrl+O")) 
                {
                    IGFD::FileDialogConfig config;
                    config.path = ".";
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseSessionFile", "Choose Session File", ".bin,.", config);
                }

                ImGui::Separator();
                if (ImGui::MenuItem("Export to CSV")) {
                    // Export timing data to CSV format
                }
                if (ImGui::MenuItem("Export Screenshot")) {
                    // Save timeline as image
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4")) {
                    // Close profiler window
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Timeline", nullptr, &showTimeline);
                ImGui::MenuItem("Statistics", nullptr, &showStatistics);
                ImGui::MenuItem("Settings", nullptr, &showSettings);
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Zoom")) {
                    // Reset timeline zoom to fit all data
                }
                if (ImGui::MenuItem("Fit to Window")) {
                    // Adjust timeline to fit current window size
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Capture")) {
                ImGui::MenuItem("Pause/Resume", "Space", &enablePausing);
                ImGui::Separator();
                if (ImGui::MenuItem("Single Frame Capture", "F1")) {
                    // Capture next single frame
                }
                if (ImGui::MenuItem("Clear History", "F2")) {
                    // Clear all captured frame data
                }
                ImGui::Separator();
                if (ImGui::BeginMenu("Frame History")) {
                    const char* frameOptions[] = {"Current", "Frame -1", "Frame -2", "Frame -3", "Frame -4"};
                    for (int i = 0; i < 5; i++) {
                        if (ImGui::MenuItem(frameOptions[i], nullptr, selectedFrameHistory == i)) {
                            selectedFrameHistory = i;
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Tools")) {
                if (ImGui::MenuItem("Memory Profiler")) {
                    // Open memory profiler window
                }
                if (ImGui::MenuItem("GPU Profiler")) {
                    // Open GPU profiler window
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Performance Counters")) {
                    // Show performance counters window
                }
                if (ImGui::MenuItem("Call Stack Viewer")) {
                    // Show call stack for selected function
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    // Show about dialog
                }
                if (ImGui::MenuItem("Keyboard Shortcuts")) {
                    // Show shortcuts help
                }
                if (ImGui::MenuItem("Documentation")) {
                    // Open documentation
                }
                ImGui::EndMenu();
            }
            
            ImGui::EndMenuBar();
        }

        
        
        
        // Example profiler data
        ecs::profiling::frame_data_t frameData;
        frameData.frameBeginTime = 0.0;
        frameData.frameEndTime = 16.67;
        frameData.countersData["Frame"] = { 16.67, 5.0, 4.0, 5.0, 1.0, 0 };
        frameData.countersData["Update"] = { 16.66, 5.0, 4.0, 5.0, 16.66 / 16.67, 1 };
        frameData.countersData["Physics"] = { 8.0, 5.0, 4.0, 5.0, 0.5, 2 };
        frameData.countersData["AI"] = { 8.67, 5.0, 4.0, 5.0, 0.5, 2 };
        
        // Status bar with frame info
        ImGui::Text("Frame Time: 16.2ms (Target: %.2fms @ %.0f FPS) %s", 
                   targetFrameTime, 1000.0f/targetFrameTime, 
                   enablePausing ? "[PAUSED]" : "");
        
        if (selectedFrameHistory > 0) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "(Viewing Frame -%d)", selectedFrameHistory);
        }
        
        ImGui::Separator();
        
        
        // Show timeline if enabled
        if (showTimeline) 
        {
            DrawTimeline(frameData, targetFrameTime);
        }
        
        
        /*
        // Show statistics panel if enabled
        if (showStatistics) 
        {
            ImGui::Separator();
            if (ImGui::CollapsingHeader("Statistics", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Columns(3, "StatsColumns");
                ImGui::Text("Function");
                ImGui::NextColumn();
                ImGui::Text("Time (ms)");
                ImGui::NextColumn();
                ImGui::Text("Percentage");
                ImGui::NextColumn();
                ImGui::Separator();
                
                float totalTime = 16.2f;
                for (const auto& entry : sampleEntries) {
                    if (entry.depth > 0) { // Skip root frame entry
                        ImGui::Text("%s", entry.name.c_str());
                        ImGui::NextColumn();
                        ImGui::Text("%.3f", entry.duration);
                        ImGui::NextColumn();
                        ImGui::Text("%.1f%%", (entry.duration / totalTime) * 100.0f);
                        ImGui::NextColumn();
                    }
                }
                ImGui::Columns(1);
            }
        }
        */
        
        
        // Show settings panel if enabled
        if (showSettings) {
            ImGui::Separator();
            if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::SliderFloat("Target Frame Time (ms)", &targetFrameTime, 8.33f, 33.33f, "%.2f");
                ImGui::Text("Target FPS: %.0f", 1000.0f / targetFrameTime);
                
                ImGui::Checkbox("Auto-pause on frame spikes", &enablePausing);
                
                static int maxHistoryFrames = 60;
                ImGui::SliderInt("History Buffer Size", &maxHistoryFrames, 10, 300);
                
                static bool showTooltips = true;
                ImGui::Checkbox("Show tooltips", &showTooltips);
            }
        }
        
        // Quick control buttons
        ImGui::Separator();
        if (ImGui::Button("Refresh Data")) 
        {
            // In a real implementation, you'd update the profiler data here
        }

        ImGui::SameLine();
        if (ImGui::Button(enablePausing ? "Resume" : "Pause")) 
        {
            enablePausing = !enablePausing;
        }

        ImGui::SameLine();
        if (ImGui::Button("Clear History")) 
        {
            
            // Clear frame history
        }
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseSessionFile", 
            ImGuiWindowFlags_None, ImVec2(800, 600)))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string fileName = ImGuiFileDialog::Instance()->GetFilePathName();
                m_session = std::shared_ptr<ecs::profiling::Session>(
                    ecs::profiling::Session::CreateFromFile(fileName));
            }

            ImGuiFileDialog::Instance()->Close();
        }

    ImGui::End();

    
}

void ProfilerApp::DrawTimeline(const ecs::profiling::frame_data_t& frameData, double frameTimeMs)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    
    // Timeline configuration
    const float timelineHeight = 200.0f;
    const float entryHeight = 18.0f;
    const float entrySpacing = 2.0f;
    const float leftMargin = 100.0f;
    const float rightMargin = 20.0f;
    const float topMargin = 30.0f;
    
    // Ensure minimum width for timeline
    if (canvasSize.x < 300.0f) canvasSize.x = 300.0f;
    
    // Calculate timeline bounds
    ImVec2 timelineStart = ImVec2(canvasPos.x + leftMargin, canvasPos.y + topMargin);
    float timelineWidth = canvasSize.x - leftMargin - rightMargin;
    
    // Draw background
    drawList->AddRectFilled(canvasPos, 
                           ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + timelineHeight),
                           IM_COL32(30, 30, 30, 255));
    
    // Draw frame time reference line
    float frameEndX = timelineStart.x + (frameTimeMs / frameTimeMs) * timelineWidth;
    drawList->AddLine(ImVec2(frameEndX, timelineStart.y - 10),
                     ImVec2(frameEndX, timelineStart.y + timelineHeight - topMargin),
                     IM_COL32(255, 255, 0, 150), 2.0f);
    
    // Draw time scale
    const int numTicks = 8;
    for (int i = 0; i <= numTicks; ++i) {
        float t = (float)i / numTicks;
        float x = timelineStart.x + t * timelineWidth;
        float timeMs = t * frameTimeMs;
        
        // Tick mark
        drawList->AddLine(ImVec2(x, timelineStart.y - 5),
                         ImVec2(x, timelineStart.y),
                         IM_COL32(150, 150, 150, 255));
        
        // Time label
        char timeStr[32];
        snprintf(timeStr, sizeof(timeStr), "%.1fms", timeMs);
        drawList->AddText(ImVec2(x - 15, timelineStart.y - 25), 
                         IM_COL32(200, 200, 200, 255), timeStr);
    }
    
    // Find maximum depth for layout
    size_t maxDepth = 0;
    for (auto it = frameData.countersData.begin(); it != frameData.countersData.end(); ++it) 
    {
        maxDepth = std::max(maxDepth, it->second.depth);
    }
    
    // Draw profiler entries
    double startTime = 0.0;
    static std::vector<ImU32> colors =
    {
        IM_COL32(100, 150, 200, 255),
        IM_COL32(150, 200, 100, 255),
        IM_COL32(200, 150, 100, 255),
        IM_COL32(200, 100, 150, 255),
        IM_COL32(150, 100, 200, 255),
        IM_COL32(200, 100, 100, 255),
        IM_COL32(180, 120, 80, 255),
        IM_COL32(120, 180, 80, 255),
        IM_COL32(80, 120, 180, 255),
        IM_COL32(180, 80, 120, 255),
        IM_COL32(120, 80, 180, 255)
    };
    int colorIdx = 0;
    for (auto it = frameData.countersData.begin(); it != frameData.countersData.end(); ++it) 
    {
        const std::string& name = it->first;
        const auto& counter = it->second;

        // Calculate rectangle position and size
        double startX = timelineStart.x + (startTime / frameTimeMs) * timelineWidth;
        double endX = timelineStart.x + ((startTime + counter.totalTimeMs) / frameTimeMs) * timelineWidth;
        double width = endX - startX;
        
        // Ensure minimum width for visibility
        if (width < 2.0f) 
        {
            width = 2.0f;
            endX = startX + width;
        }
        
        // Calculate Y position based on depth
        float yPos = timelineStart.y + counter.depth * (entryHeight + entrySpacing);
        
        // Draw rectangle
        ImVec2 rectMin = ImVec2(startX, yPos);
        ImVec2 rectMax = ImVec2(endX, yPos + entryHeight);
        
        drawList->AddRectFilled(rectMin, rectMax, colors[colorIdx++]);
        if (colorIdx >= colors.size())
        {
            colorIdx = 0;
        }

        drawList->AddRect(rectMin, rectMax, IM_COL32(255, 255, 255, 100));
        
        // Draw text label if rectangle is wide enough
        if (width > 60.0f) {
            ImVec2 textSize = ImGui::CalcTextSize(name.c_str());
            ImVec2 textPos = ImVec2(startX + 4, yPos + (entryHeight - textSize.y) * 0.5f);
            
            // Clip text to rectangle bounds
            drawList->PushClipRect(rectMin, rectMax);
            drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), name.c_str());
            drawList->PopClipRect();
        }
        
        // Tooltip on hover
        if (ImGui::IsMouseHoveringRect(rectMin, rectMax)) {
            ImGui::BeginTooltip();
            ImGui::Text("Function: %s", name.c_str());
            ImGui::Text("Duration: %.3f ms", counter.totalTimeMs);
            ImGui::Text("Start: %.3f ms", startTime);
            ImGui::Text("Depth: %d", counter.depth);
            ImGui::EndTooltip();
        }

        startTime += counter.totalTimeMs;
    }
    
    // Draw function names on the left
    for (auto it = frameData.countersData.begin(); it != frameData.countersData.end(); ++it) 
    {
        const auto& entry = it->second;
        float yPos = timelineStart.y + it->second.depth * (entryHeight + entrySpacing);
        ImVec2 textPos = ImVec2(canvasPos.x + 5, yPos + (entryHeight - ImGui::GetTextLineHeight()) * 0.5f);
        
        /*
        // Only draw if not overlapping with other entries at same depth
        bool shouldDraw = true;
        for (auto it2 = frameData.countersData.begin(); it2 != frameData.countersData.end(); ++it2)
        {
            const auto& other = it2->second;
            if (&other != &entry && other.depth == entry.depth && 
                abs(other.startTime - entry.startTime) < 0.1f) {
                shouldDraw = entry.startTime <= other.startTime;
                break;
            }
        }
        */
        
        drawList->AddText(textPos, IM_COL32(200, 200, 200, 255), it->first.c_str());
    }
    
    // Reserve space for the timeline
    ImGui::Dummy(ImVec2(canvasSize.x, timelineHeight));
}