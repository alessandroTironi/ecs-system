# sprite-eater-tests

include(FetchContent)

# Configure GoogleTest
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.16.0
)

# Make GoogleTest available
FetchContent_MakeAvailable(googletest)

# Ensure GoogleTest uses the same runtime settings as the rest of the project
set_target_properties(gtest PROPERTIES
    MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}"
)
set_target_properties(gtest_main PROPERTIES
    MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}"
)

include_directories("${ECS_CORE_INCLUDE_DIR}" "include")

# Create the unit test executable
file(GLOB TEST_SOURCES "src/*.cpp")
add_executable(ecs-test ${TEST_SOURCES})

# Link to the sprite-eater-core library and gtest
target_link_libraries(ecs-test
 PRIVATE
  gtest_main
  ecs-core
)

# Add tests to CTest with verbose output
add_test(NAME ecs_gtests COMMAND ecs-test --gtest_color=yes)
set_tests_properties(ecs_gtests PROPERTIES
    ENVIRONMENT "GTEST_COLOR=1"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)