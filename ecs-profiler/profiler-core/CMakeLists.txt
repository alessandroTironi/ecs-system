cmake_minimum_required(VERSION 3.14)
project(profiler-core)

include(FetchContent)

# Add dependency to cereal serialization library
FetchContent_Declare(
    cereal
    GIT_REPOSITORY https://github.com/USCiLab/cereal.git
    GIT_TAG v1.3.2
)

# Populate cereal but don't call add_subdirectory
FetchContent_GetProperties(cereal)
if(NOT cereal_POPULATED)
    FetchContent_Populate(cereal)
    # Don't call add_subdirectory to avoid cereal's CMakeLists.txt
    # We only need the headers anyway
endif()

# project source files
set(PROFILER_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include")
set(PROFILER_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")

# automatically gather all source files
file(GLOB_RECURSE SOURCES "${PROFILER_SRC_DIR}/*.cpp" "${PROFILER_SRC_DIR}/*.h" 
    "${PROFILER_INCLUDE_DIR}/*.h" "${PROFILER_INCLUDE_DIR}/*.cpp")

# Create main library target
add_library(profiler-core STATIC ${SOURCES})

set_target_properties(profiler-core PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(profiler-core PROPERTIES OUTPUT_NAME "fang-prof") # temp name

# expose the include directory
target_include_directories(profiler-core 
    PUBLIC
        ${PROFILER_INCLUDE_DIR}
        ${cereal_SOURCE_DIR}/include
    PRIVATE
        ${PROFILER_SRC_DIR}   
)

target_link_libraries(profiler-core 
    PRIVATE 
        dbghelp)