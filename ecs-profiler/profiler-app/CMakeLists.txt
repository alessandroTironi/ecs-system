project(profiler-app)

file(GLOB_RECURSE SOURCES "include/*.h" "include/*.cpp" "src/*.h" "src/*.cpp")

add_executable(profiler-app ${SOURCES})

target_link_libraries(profiler-app PRIVATE profiler-core)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET profiler-app PROPERTY CXX_STANDARD 20)
endif()