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

# Find required packages
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
add_library(ecs-core STATIC ${SOURCES})

# Set debug macro (assuming this is defined elsewhere)
if(COMMAND set_debug_macro)
  set_debug_macro(ecs-core)
endif()

set_target_properties(ecs-core PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(ecs-core PROPERTIES OUTPUT_NAME "ECSCore")

# Setup include directories 
target_include_directories(ecs-core PUBLIC
  ${ECS_CORE_INCLUDE_DIR}
  ${SDL2_INCLUDE_DIR}         
)

# Link against dependencies
target_link_libraries(ecs-core PUBLIC
  glfw
  ${OPENGL_LIBRARIES}
  ${SDL2_LIBRARY}
  ${SDL2MAIN_LIBRARY}
  profiler-core
)