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
    
    // static bool showTimeline = true;
    // static bool showStatistics = false;
    // static bool showSettings = false;
    // static bool enablePausing = false;
    // static float targetFrameTime = 16.67f;
    // static int selectedFrameHistory = 0;

    // // Get the main viewport and set window to fill it
    // const ImGuiViewport* viewport = ImGui::GetMainViewport();
    // ImGui::SetNextWindowPos(viewport->WorkPos);
    // ImGui::SetNextWindowSize(viewport->WorkSize);

    // ImGuiWindowFlags windowFlags = 0;
    // windowFlags |= ImGuiWindowFlags_NoTitleBar;
    // windowFlags |= ImGuiWindowFlags_NoCollapse;
    // windowFlags |= ImGuiWindowFlags_NoResize;
    // windowFlags |= ImGuiWindowFlags_NoMove;
    // windowFlags |= ImGuiWindowFlags_MenuBar;
    
    // if (ImGui::Begin("Frame Profiler", nullptr, windowFlags)) {
    //     // Menu Bar
    //     if (ImGui::BeginMenuBar()) {
    //         if (ImGui::BeginMenu("File")) {
    //             if (ImGui::MenuItem("Save Profile Data", "Ctrl+S")) 
    //             {
    //                 // In a real implementation, save profiler data to file
    //             }
    //             if (ImGui::MenuItem("Load Profile Data", "Ctrl+O")) 
    //             {
    //                 IGFD::FileDialogConfig config;
    //                 config.path = ".";
    //                 ImGuiFileDialog::Instance()->OpenDialog("ChooseSessionFile", "Choose Session File", ".bin,.", config);
    //             }

    //             ImGui::Separator();
    //             if (ImGui::MenuItem("Export to CSV")) {
    //                 // Export timing data to CSV format
    //             }
    //             if (ImGui::MenuItem("Export Screenshot")) {
    //                 // Save timeline as image
    //             }
    //             ImGui::Separator();
    //             if (ImGui::MenuItem("Exit", "Alt+F4")) {
    //                 // Close profiler window
    //             }
    //             ImGui::EndMenu();
    //         }
            
    //         if (ImGui::BeginMenu("View")) {
    //             ImGui::MenuItem("Timeline", nullptr, &showTimeline);
    //             ImGui::MenuItem("Statistics", nullptr, &showStatistics);
    //             ImGui::MenuItem("Settings", nullptr, &showSettings);
    //             ImGui::Separator();
    //             if (ImGui::MenuItem("Reset Zoom")) {
    //                 // Reset timeline zoom to fit all data
    //             }
    //             if (ImGui::MenuItem("Fit to Window")) {
    //                 // Adjust timeline to fit current window size
    //             }
    //             ImGui::EndMenu();
    //         }
            
    //         if (ImGui::BeginMenu("Capture")) {
    //             ImGui::MenuItem("Pause/Resume", "Space", &enablePausing);
    //             ImGui::Separator();
    //             if (ImGui::MenuItem("Single Frame Capture", "F1")) {
    //                 // Capture next single frame
    //             }
    //             if (ImGui::MenuItem("Clear History", "F2")) {
    //                 // Clear all captured frame data
    //             }
    //             ImGui::Separator();
    //             if (ImGui::BeginMenu("Frame History")) {
    //                 const char* frameOptions[] = {"Current", "Frame -1", "Frame -2", "Frame -3", "Frame -4"};
    //                 for (int i = 0; i < 5; i++) {
    //                     if (ImGui::MenuItem(frameOptions[i], nullptr, selectedFrameHistory == i)) {
    //                         selectedFrameHistory = i;
    //                     }
    //                 }
    //                 ImGui::EndMenu();
    //             }
    //             ImGui::EndMenu();
    //         }
            
    //         if (ImGui::BeginMenu("Tools")) {
    //             if (ImGui::MenuItem("Memory Profiler")) {
    //                 // Open memory profiler window
    //             }
    //             if (ImGui::MenuItem("GPU Profiler")) {
    //                 // Open GPU profiler window
    //             }
    //             ImGui::Separator();
    //             if (ImGui::MenuItem("Performance Counters")) {
    //                 // Show performance counters window
    //             }
    //             if (ImGui::MenuItem("Call Stack Viewer")) {
    //                 // Show call stack for selected function
    //             }
    //             ImGui::EndMenu();
    //         }
            
    //         if (ImGui::BeginMenu("Help")) {
    //             if (ImGui::MenuItem("About")) {
    //                 // Show about dialog
    //             }
    //             if (ImGui::MenuItem("Keyboard Shortcuts")) {
    //                 // Show shortcuts help
    //             }
    //             if (ImGui::MenuItem("Documentation")) {
    //                 // Open documentation
    //             }
    //             ImGui::EndMenu();
    //         }
            
    //         ImGui::EndMenuBar();
    //     }

        
        
        
    //     // Example profiler data
    //     ecs::profiling::frame_data_t frameData;
    //     frameData.frameBeginTime = 0.0;
    //     frameData.frameEndTime = 16.67;
    //     frameData.countersData["Frame"] = { 16.67, 5.0, 4.0, 5.0, 1.0, 0 };
    //     frameData.countersData["Update"] = { 16.66, 5.0, 4.0, 5.0, 16.66 / 16.67, 1 };
    //     frameData.countersData["Physics"] = { 8.0, 5.0, 4.0, 5.0, 0.5, 2 };
    //     frameData.countersData["AI"] = { 8.67, 5.0, 4.0, 5.0, 0.5, 2 };
        
    //     // Status bar with frame info
    //     ImGui::Text("Frame Time: 16.2ms (Target: %.2fms @ %.0f FPS) %s", 
    //                targetFrameTime, 1000.0f/targetFrameTime, 
    //                enablePausing ? "[PAUSED]" : "");
        
    //     if (selectedFrameHistory > 0) {
    //         ImGui::SameLine();
    //         ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "(Viewing Frame -%d)", selectedFrameHistory);
    //     }
        
    //     ImGui::Separator();
        
        
    //     // Show timeline if enabled
    //     if (showTimeline) 
    //     {
    //         DrawTimeline(targetFrameTime);
    //     }
        
        
    //     /*
    //     // Show statistics panel if enabled
    //     if (showStatistics) 
    //     {
    //         ImGui::Separator();
    //         if (ImGui::CollapsingHeader("Statistics", ImGuiTreeNodeFlags_DefaultOpen)) {
    //             ImGui::Columns(3, "StatsColumns");
    //             ImGui::Text("Function");
    //             ImGui::NextColumn();
    //             ImGui::Text("Time (ms)");
    //             ImGui::NextColumn();
    //             ImGui::Text("Percentage");
    //             ImGui::NextColumn();
    //             ImGui::Separator();
                
    //             float totalTime = 16.2f;
    //             for (const auto& entry : sampleEntries) {
    //                 if (entry.depth > 0) { // Skip root frame entry
    //                     ImGui::Text("%s", entry.name.c_str());
    //                     ImGui::NextColumn();
    //                     ImGui::Text("%.3f", entry.duration);
    //                     ImGui::NextColumn();
    //                     ImGui::Text("%.1f%%", (entry.duration / totalTime) * 100.0f);
    //                     ImGui::NextColumn();
    //                 }
    //             }
    //             ImGui::Columns(1);
    //         }
    //     }
    //     */
        
        
    //     // Show settings panel if enabled
    //     if (showSettings) {
    //         ImGui::Separator();
    //         if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
    //             ImGui::SliderFloat("Target Frame Time (ms)", &targetFrameTime, 8.33f, 33.33f, "%.2f");
    //             ImGui::Text("Target FPS: %.0f", 1000.0f / targetFrameTime);
                
    //             ImGui::Checkbox("Auto-pause on frame spikes", &enablePausing);
                
    //             static int maxHistoryFrames = 60;
    //             ImGui::SliderInt("History Buffer Size", &maxHistoryFrames, 10, 300);
                
    //             static bool showTooltips = true;
    //             ImGui::Checkbox("Show tooltips", &showTooltips);
    //         }
    //     }
        
    //     // Quick control buttons
    //     ImGui::Separator();
    //     if (ImGui::Button("Refresh Data")) 
    //     {
    //         // In a real implementation, you'd update the profiler data here
    //     }

    //     ImGui::SameLine();
    //     if (ImGui::Button(enablePausing ? "Resume" : "Pause")) 
    //     {
    //         enablePausing = !enablePausing;
    //     }

    //     ImGui::SameLine();
    //     if (ImGui::Button("Clear History")) 
    //     {
            
    //         // Clear frame history
    //     }
    // }

    // if (ImGuiFileDialog::Instance()->Display("ChooseSessionFile", 
    //         ImGuiWindowFlags_None, ImVec2(800, 600)))
    //     {
    //         if (ImGuiFileDialog::Instance()->IsOk())
    //         {
    //             std::string fileName = ImGuiFileDialog::Instance()->GetFilePathName();
    //             m_session = std::shared_ptr<ecs::profiling::Session>(
    //                 ecs::profiling::Session::CreateFromFile(fileName));
    //         }

    //         ImGuiFileDialog::Instance()->Close();
    //     }

    // ImGui::End();
}

void ProfilerApp::DrawTimeline(double frameTimeMs)
{
    /*
    if (m_session.get() == nullptr)
    {
        return;
    }

    const std::vector<ecs::profiling::frame_data_t>& frameHistory = m_session->GetFrameData();

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
    const float scrollbarHeight = 16.0f;

    // Static variables for scrolling and zooming
    static double timeMarkerPosition = frameTimeMs * 0.0;
    static bool isDraggingMarker = false;
    static float zoomLevel = 1.0f;  // 1.0 = no zoom, > 1.0 = zoomed in
    static float scrollOffset = 0.0f;  // Horizontal scroll offset (0.0 to 1.0)
    
    // Ensure minimum width for timeline
    if (canvasSize.x < 300.0f) 
    {
        canvasSize.x = 300.0f;
    }

    // Calculate timeline bounds (reserve space for scrollbar)
    ImVec2 timelineStart = ImVec2(canvasPos.x + leftMargin, canvasPos.y + topMargin);
    float timelineWidth = canvasSize.x - leftMargin - rightMargin;
    float actualTimelineHeight = timelineHeight - scrollbarHeight - 5.0f; // Leave space for scrollbar
    
    // Handle mouse wheel for zooming
    ImVec2 mousePos = ImGui::GetMousePos();
    bool isMouseOverTimeline = mousePos.x >= timelineStart.x && 
                              mousePos.x <= timelineStart.x + timelineWidth &&
                              mousePos.y >= timelineStart.y && 
                              mousePos.y <= timelineStart.y + actualTimelineHeight;
    
    if (isMouseOverTimeline && ImGui::GetIO().MouseWheel != 0.0f)
    {
        float mouseTimeRatio = (mousePos.x - timelineStart.x) / timelineWidth;
        float mouseTimeMs = (scrollOffset + mouseTimeRatio / zoomLevel) * frameTimeMs;
        
        // Zoom in/out (support zoom levels from 0.1x to 10x)
        float zoomDelta = ImGui::GetIO().MouseWheel * 0.1f;
        float newZoomLevel = std::max(0.1f, std::min(zoomLevel + zoomDelta, 10.0f));
        
        // Adjust scroll to keep mouse position stable
        float newMouseTimeRatio = mouseTimeMs / frameTimeMs;
        scrollOffset = newMouseTimeRatio - mouseTimeRatio / newZoomLevel;
        
        // For zoom levels < 1.0, we might need to scroll beyond the normal 0-1 range
        float maxScrollOffset = std::max(0.0f, 1.0f - 1.0f / newZoomLevel);
        scrollOffset = std::max(0.0f, std::min(scrollOffset, maxScrollOffset));
        
        zoomLevel = newZoomLevel;
    }
    
    // Calculate visible time range
    double visibleStartTime = scrollOffset * frameTimeMs;
    double visibleEndTime = (scrollOffset + 1.0f / zoomLevel) * frameTimeMs;
    
    // For zoom levels < 1.0, we show more than the full frame time
    if (zoomLevel < 1.0f)
    {
        double totalVisibleDuration = frameTimeMs / zoomLevel;
        visibleStartTime = scrollOffset * (frameTimeMs - totalVisibleDuration);
        visibleEndTime = visibleStartTime + totalVisibleDuration;
    }
    
    double visibleDuration = visibleEndTime - visibleStartTime;
    
    // Draw background
    drawList->AddRectFilled(canvasPos, 
                           ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + actualTimelineHeight + topMargin),
                           IM_COL32(30, 30, 30, 255));
    
    // Draw frame time reference line (only if visible)
    if (frameTimeMs >= visibleStartTime && frameTimeMs <= visibleEndTime)
    {
        float frameEndX = timelineStart.x + ((frameTimeMs - visibleStartTime) / visibleDuration) * timelineWidth;
        drawList->AddLine(ImVec2(frameEndX, timelineStart.y - 10),
                         ImVec2(frameEndX, timelineStart.y + actualTimelineHeight),
                         IM_COL32(255, 255, 0, 150), 2.0f);
    }
    
    // Draw time scale for visible range
    const int numTicks = 8;
    for (int i = 0; i <= numTicks; ++i) {
        float t = (float)i / numTicks;
        float x = timelineStart.x + t * timelineWidth;
        double timeMs = visibleStartTime + t * visibleDuration;
        
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
    size_t maxDepth = 3;
    
    // Draw profiler entries
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

    for (const ecs::profiling::frame_data_t& frameData : frameHistory)
    {
        double currentTime = frameData.frameBeginTime;
        for (auto it = frameData.countersData.begin(); it != frameData.countersData.end(); ++it) 
        {
            const std::string& name = it->first;
            const auto& counter = it->second;
            
            double entryStartTime = currentTime;
            double entryEndTime = currentTime + counter.totalTimeMs;
            
            // Skip entries that are completely outside visible range
            if (entryEndTime < visibleStartTime || entryStartTime > visibleEndTime)
            {
                currentTime += counter.totalTimeMs;
                continue;
            }

            // Calculate rectangle position and size for visible portion
            double visibleEntryStart = std::max(entryStartTime, visibleStartTime);
            double visibleEntryEnd = std::min(entryEndTime, visibleEndTime);
            
            double startX = timelineStart.x + ((visibleEntryStart - visibleStartTime) / visibleDuration) * timelineWidth;
            double endX = timelineStart.x + ((visibleEntryEnd - visibleStartTime) / visibleDuration) * timelineWidth;
            double width = endX - startX;
            
            // Ensure minimum width for visibility
            if (width < 2.0) 
            {
                width = 2.0;
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
                ImGui::Text("Start: %.3f ms", entryStartTime);
                ImGui::Text("Depth: %d", counter.depth);
                ImGui::EndTooltip();
            }

            currentTime += counter.totalTimeMs;
        }
    }

    // Draw time marker (only if visible)
    if (timeMarkerPosition >= visibleStartTime && timeMarkerPosition <= visibleEndTime)
    {
        float markerX = timelineStart.x + ((timeMarkerPosition - visibleStartTime) / visibleDuration) * timelineWidth;

        // Draw marker line
        ImVec2 markerTop = ImVec2(markerX, timelineStart.y - 10);
        ImVec2 markerBottom = ImVec2(markerX, timelineStart.y + actualTimelineHeight);
        drawList->AddLine(markerTop, markerBottom, IM_COL32(255, 100, 100, 255), 3.0f);
        
        // Draw marker handle (draggable triangle at top)
        const float handleSize = 8.0f;
        ImVec2 handleTop = ImVec2(markerX, timelineStart.y - 20);
        ImVec2 handleLeft = ImVec2(markerX - handleSize, timelineStart.y - 10);
        ImVec2 handleRight = ImVec2(markerX + handleSize, timelineStart.y - 10);
        
        ImU32 handleColor = isDraggingMarker ? IM_COL32(255, 150, 150, 255) : IM_COL32(255, 100, 100, 255);
        drawList->AddTriangleFilled(handleTop, handleLeft, handleRight, handleColor);
        drawList->AddTriangle(handleTop, handleLeft, handleRight, IM_COL32(255, 255, 255, 200));

        // Handle marker dragging
        ImVec2 handleMin = ImVec2(markerX - handleSize, timelineStart.y - 20);
        ImVec2 handleMax = ImVec2(markerX + handleSize, timelineStart.y - 10);

        bool isHoveringHandle = ImGui::IsMouseHoveringRect(handleMin, handleMax);
        
        if (isHoveringHandle) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        }
        
        if (isHoveringHandle && ImGui::IsMouseClicked(0)) {
            isDraggingMarker = true;
        }
        
        if (isDraggingMarker) {
            if (ImGui::IsMouseDown(0)) {
                // Update marker position based on mouse X
                float mouseTimelineX = mousePos.x - timelineStart.x;
                mouseTimelineX = std::max(0.0f, std::min(mouseTimelineX, timelineWidth));
                timeMarkerPosition = visibleStartTime + (mouseTimelineX / timelineWidth) * visibleDuration;
            } else {
                isDraggingMarker = false;
            }
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        }

        // Draw marker time label
        char markerTimeStr[32];
        snprintf(markerTimeStr, sizeof(markerTimeStr), "%.2fms", timeMarkerPosition);
        ImVec2 labelSize = ImGui::CalcTextSize(markerTimeStr);
        ImVec2 labelPos = ImVec2(markerX - labelSize.x * 0.5f, timelineStart.y - 35);
        
        // Ensure label stays within bounds
        labelPos.x = std::max(canvasPos.x, std::min(labelPos.x, canvasPos.x + canvasSize.x - labelSize.x));
        
        drawList->AddRectFilled(ImVec2(labelPos.x - 2, labelPos.y - 1), 
                               ImVec2(labelPos.x + labelSize.x + 2, labelPos.y + labelSize.y + 1),
                               IM_COL32(50, 50, 50, 200));
        drawList->AddText(labelPos, IM_COL32(255, 255, 255, 255), markerTimeStr);
        
        // Show which functions are active at marker time in a tooltip or info panel
        if (isHoveringHandle || isDraggingMarker) {
            ImGui::BeginTooltip();
            ImGui::Text("Time Marker: %.3f ms", timeMarkerPosition);
            ImGui::Separator();
            ImGui::Text("Active functions:");
            
            bool foundActive = false;
            for (const ecs::profiling::frame_data_t& frameData : frameHistory) 
            {
                const auto& entries = frameData.countersData;
                
                float currentTime = frameData.frameBeginTime;
                for (const auto& entry : entries) 
                {
                    if (timeMarkerPosition >= currentTime && 
                        timeMarkerPosition <= (currentTime + entry.second.totalTimeMs)) 
                    {
                        ImGui::Text("  %s (%.3f ms remaining)", 
                                  entry.first.c_str(), 
                                  (currentTime + entry.second.totalTimeMs) - timeMarkerPosition);
                        foundActive = true;
                    }

                    currentTime += entry.second.totalTimeMs;
                }
            }
            
            if (!foundActive) {
                ImGui::Text("  (none)");
            }
            
            ImGui::EndTooltip();
        }
    }

    // Draw horizontal scrollbar
    ImVec2 scrollbarStart = ImVec2(timelineStart.x, timelineStart.y + actualTimelineHeight + 5.0f);
    ImVec2 scrollbarEnd = ImVec2(timelineStart.x + timelineWidth, scrollbarStart.y + scrollbarHeight);
    
    // Scrollbar background
    drawList->AddRectFilled(scrollbarStart, scrollbarEnd, IM_COL32(60, 60, 60, 255));
    drawList->AddRect(scrollbarStart, scrollbarEnd, IM_COL32(100, 100, 100, 255));
    
    // Scrollbar thumb
    float thumbWidth = std::max(20.0f, timelineWidth / zoomLevel); // Minimum thumb width for usability
    float scrollRange = timelineWidth - thumbWidth;
    
    float thumbStart, maxScrollOffset;
    if (zoomLevel >= 1.0f)
    {
        // Normal zoom in behavior
        maxScrollOffset = 1.0f - 1.0f / zoomLevel;
        thumbStart = scrollbarStart.x + scrollOffset * scrollRange;
    }
    else
    {
        // Zoom out behavior - show more than full timeline
        maxScrollOffset = std::max(0.0f, 1.0f - zoomLevel);
        thumbStart = scrollbarStart.x + (scrollOffset / maxScrollOffset) * scrollRange;
    }
    
    float thumbEnd = thumbStart + thumbWidth;
    
    ImVec2 thumbMin = ImVec2(thumbStart, scrollbarStart.y + 1);
    ImVec2 thumbMax = ImVec2(thumbEnd, scrollbarEnd.y - 1);
    
    bool isHoveringScrollbar = ImGui::IsMouseHoveringRect(scrollbarStart, scrollbarEnd);
    bool isHoveringThumb = ImGui::IsMouseHoveringRect(thumbMin, thumbMax);
    
    ImU32 thumbColor = isHoveringThumb ? IM_COL32(150, 150, 150, 255) : IM_COL32(120, 120, 120, 255);
    drawList->AddRectFilled(thumbMin, thumbMax, thumbColor);
    drawList->AddRect(thumbMin, thumbMax, IM_COL32(180, 180, 180, 255));
    
    // Handle scrollbar interaction
    static bool isDraggingScrollbar = false;
    static float dragStartOffset = 0.0f;
    
    if (isHoveringScrollbar && ImGui::IsMouseClicked(0))
    {
        if (isHoveringThumb)
        {
            // Start dragging thumb
            isDraggingScrollbar = true;
            dragStartOffset = mousePos.x - thumbStart;
        }
        else
        {
            // Click on scrollbar track - jump to position
            float clickRatio = (mousePos.x - scrollbarStart.x) / timelineWidth;
            
            if (zoomLevel >= 1.0f)
            {
                scrollOffset = clickRatio - 0.5f / zoomLevel;
                scrollOffset = std::max(0.0f, std::min(scrollOffset, 1.0f - 1.0f / zoomLevel));
            }
            else
            {
                float maxOffset = std::max(0.0f, 1.0f - zoomLevel);
                scrollOffset = clickRatio * maxOffset;
                scrollOffset = std::max(0.0f, std::min(scrollOffset, maxOffset));
            }
        }
    }
    
    if (isDraggingScrollbar)
    {
        if (ImGui::IsMouseDown(0))
        {
            float newThumbStart = mousePos.x - dragStartOffset;
            float thumbRatio = (newThumbStart - scrollbarStart.x) / scrollRange;
            
            if (zoomLevel >= 1.0f)
            {
                scrollOffset = thumbRatio * (1.0f - 1.0f / zoomLevel);
                scrollOffset = std::max(0.0f, std::min(scrollOffset, 1.0f - 1.0f / zoomLevel));
            }
            else
            {
                float maxOffset = std::max(0.0f, 1.0f - zoomLevel);
                scrollOffset = thumbRatio * maxOffset;
                scrollOffset = std::max(0.0f, std::min(scrollOffset, maxOffset));
            }
        }
        else
        {
            isDraggingScrollbar = false;
        }
    }
    
    // Display zoom and scroll info
    ImGui::SetCursorScreenPos(ImVec2(canvasPos.x + 5, scrollbarEnd.y + 5));
    ImGui::Text("Zoom: %.1fx | Range: %.1f - %.1f ms", zoomLevel, visibleStartTime, visibleEndTime);
    if (zoomLevel != 1.0f)
    {
        ImGui::SameLine();
        if (zoomLevel > 1.0f)
        {
            ImGui::Text("| Mouse wheel to zoom, drag scrollbar to pan");
        }
        else
        {
            ImGui::Text("| Zoomed out - showing %.1fx timeline", 1.0f / zoomLevel);
        }
    }
    else
    {
        ImGui::SameLine();
        ImGui::Text("| Mouse wheel to zoom");
    }
    
    // Reserve space for the timeline and controls
    ImGui::Dummy(ImVec2(canvasSize.x, timelineHeight + 25.0f));
    */
}