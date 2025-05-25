cmake_minimum_required(VERSION 3.14)
project(ecs-core)

include(FetchContent)

# SDL2 dependency
find_package(SDL2 2.30 QUIET)
if (NOT SDL2_FOUND)
    FetchContent_Declare(
        SDL2
        DOWNLOAD_EXTRACT_TIMESTAMP OFF
        URL https://github.com/libsdl-org/SDL/releases/download/release-2.32.0/SDL2-devel-2.32.0-VC.zip
    )
    FetchContent_GetProperties(SDL2)
    if (NOT SDL2_POPULATED)
        set(FETCHCONTENT_QUIET NO)
        FetchContent_MakeAvailable(SDL2)
        message(STATUS "sdl2_SOURCE_DIR: ${sdl2_SOURCE_DIR}")
        # Set SDL2 include and library directories
        set(SDL2_INCLUDE_DIR "${sdl2_SOURCE_DIR}/include")
        message("Including files from ${SDL2_INCLUDE_DIR}")
        set(SDL2_LIBRARY "${sdl2_SOURCE_DIR}/lib/x64/SDL2.lib")
        set(SDL2MAIN_LIBRARY "${sdl2_SOURCE_DIR}/lib/x64/SDL2main.lib")
        set(SDL2_FOUND TRUE)
    endif()
endif()

# Fetch ImGui
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git  
  GIT_TAG v1.91.9b
)
FetchContent_MakeAvailable(imgui)

# Get source directory of ImGui after fetching
FetchContent_GetProperties(imgui SOURCE_DIR imgui_SOURCE_DIR)

# Create ImGui library target
add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  # Add SDL2 backend files to ImGui library
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Setup include directories for ImGui
target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${SDL2_INCLUDE_DIR}  # SDL2 headers for ImGui
)

# Link SDL2 to ImGui library
target_link_libraries(imgui PUBLIC
  ${SDL2_LIBRARY}
  ${SDL2MAIN_LIBRARY}
)

# Find required packages for the backend
find_package(OpenGL REQUIRED)

# Fetch GLFW
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.4
)
# Disable GLFW tests and examples
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# Set up project directories
set(ECS_CORE_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include")
set(ECS_CORE_SRC "${CMAKE_CURRENT_LIST_DIR}/src")

# recursively gather all source files
file(GLOB_RECURSE SOURCES "${ECS_CORE_SRC}/*.cpp" "${ECS_CORE_SRC}/*.h" "${ECS_CORE_INCLUDE_DIR}/*.h" "${ECS_CORE_INCLUDE_DIR}/*.cpp")

# Create main library target
add_library(ecs-core STATIC ${SOURCES}
  # GLFW backend files are kept for reference, but you might want to
  # remove these if you're fully switching to SDL2
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Set debug macro (assuming this is defined elsewhere)
if(COMMAND set_debug_macro)
  set_debug_macro(ecs-core)
endif()

set_target_properties(ecs-core PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(ecs-core PROPERTIES OUTPUT_NAME "ECSCore")

# Setup include directories properly for your library
target_include_directories(ecs-core PUBLIC
  ${ECS_CORE_INCLUDE_DIR}
  ${imgui_SOURCE_DIR}          # Include ImGui main directory
  ${imgui_SOURCE_DIR}/backends  # Include ImGui backends directory
  ${SDL2_INCLUDE_DIR}          # SDL2 headers for your project
)

# Link against dependencies
target_link_libraries(ecs-core PUBLIC
  imgui
  glfw
  ${OPENGL_LIBRARIES}
  ${SDL2_LIBRARY}
  ${SDL2MAIN_LIBRARY}
  profiler-core
)

# Define platform-specific compile definitions
if(WIN32)
  target_compile_definitions(ecs-core PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
elseif(UNIX AND NOT APPLE)
  target_compile_definitions(ecs-core PRIVATE IMGUI_IMPL_OPENGL_LOADER_GL3W)
elseif(APPLE)
  target_compile_definitions(ecs-core PRIVATE IMGUI_IMPL_OPENGL_LOADER_GL3W)
endif()