cmake_minimum_required(VERSION 3.14)
project(profiler-core)

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
    PRIVATE
        ${PROFILER_SRC_DIR}
)