# sprite-eater-tests

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.16.0
)

# Disable pthread in GoogleTest
set(gtest_disable_pthreads ON)

FetchContent_MakeAvailable(googletest)

# Optionally suppress warnings for the googletest targets (gtest, gtest_main)
if (MSVC)
  # For MSVC (Visual Studio), suppress all warnings with /W0
  target_compile_options(gtest PRIVATE /W0)
  target_compile_options(gtest_main PRIVATE /W0)
else()
  # For GCC/Clang, suppress warnings with -w
  target_compile_options(gtest PRIVATE -w)
  target_compile_options(gtest_main PRIVATE -w)
endif()

# Ensure GoogleTest uses the same runtime settings as the rest of the project
set_target_properties(gtest PROPERTIES
    MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}"
)
set_target_properties(gtest_main PROPERTIES
    MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}"
)

include_directories("${ECS_CORE_INCLUDE_DIR}" "include")

# Create the unit test executable
add_executable(ecs_test 
  "src/test_ecs.cpp" 
  "src/test_unique_ids.cpp"
  "src/test_archetypes.cpp"
  "src/test_queries.cpp"
)

# Link to the sprite-eater-core library and gtest
target_link_libraries(ecs_test
 PRIVATE
  gtest_main  # Use the original gtest_main target provided by GoogleTest
  ecs-core
)

# Add tests to CTest
add_test(ecs_gtests ecs_test)