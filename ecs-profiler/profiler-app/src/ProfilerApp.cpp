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
    static bool showTimeline = false;
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

        
        
        /*
        // Example profiler data
        static std::vector<ProfilerEntry> sampleEntries = {
            {"Frame", 0.0f, 16.2f, IM_COL32(100, 150, 200, 255), 0},
            {"Update", 0.5f, 3.2f, IM_COL32(150, 200, 100, 255), 1},
            {"Physics", 1.0f, 2.1f, IM_COL32(200, 150, 100, 255), 2},
            {"Collision", 1.2f, 1.5f, IM_COL32(200, 100, 150, 255), 3},
            {"AI Update", 3.8f, 1.8f, IM_COL32(150, 100, 200, 255), 2},
            {"Render", 4.0f, 11.5f, IM_COL32(200, 100, 100, 255), 1},
            {"Scene Prep", 4.2f, 2.1f, IM_COL32(180, 120, 80, 255), 2},
            {"Draw Calls", 6.5f, 8.2f, IM_COL32(120, 180, 80, 255), 2},
            {"Shadows", 7.0f, 3.1f, IM_COL32(80, 120, 180, 255), 3},
            {"Lighting", 10.5f, 3.8f, IM_COL32(180, 80, 120, 255), 3},
            {"Post Process", 14.8f, 1.2f, IM_COL32(120, 80, 180, 255), 2}
        };
        
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
            DrawProfilerTimeline(sampleEntries, targetFrameTime);
        }
        
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