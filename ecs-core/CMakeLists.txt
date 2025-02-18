project(ecs-core)

include_directories("include")

set(ECS_CORE_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include")
set(ECS_CORE_SRC "${CMAKE_CURRENT_LIST_DIR}/src")

# recursively gather all source files
file(GLOB_RECURSE SOURCES "${ECS_CORE_SRC}/*.cpp" "${ECS_CORE_SRC}/*.h" "${ECS_CORE_INCLUDE_DIR}/*.h" "${ECS_CORE_INCLUDE_DIR}/*.cpp")

add_library(ecs-core STATIC ${SOURCES})

set_target_properties(ecs-core PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(ecs-core PROPERTIES OUTPUT_NAME "ECSCore")