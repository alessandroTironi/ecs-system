# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.20)

# Enable Hot Reload for MSVC
set(CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ECS_CORE_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/ecs-core/include")

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
project(ecs-system)

# MSVC-specific settings
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

enable_testing()

# Include sub-projects.
add_subdirectory(ecs-core)
add_subdirectory(ecs-test)
add_subdirectory(ecs-runner)