project(ecs-runner)

include_directories("include")

file(GLOB_RECURSE SOURCES "include/*.h" "include/*.cpp" "src/*.h" "src/*.cpp")

# Add source to this project's executable.
add_executable (ecs-runner ${SOURCES})
target_link_libraries(ecs-runner PRIVATE ecs-core)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ecs-runner PROPERTY CXX_STANDARD 20)
endif()