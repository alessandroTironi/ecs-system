cmake_minimum_required(VERSION 3.14)

# Enable hot-reload for MSVC
set(CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(ecs-profiler)

# MSVC-specific settings
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

# Include sub-projects.
add_subdirectory(profiler-core)
add_subdirectory(profiler-app)