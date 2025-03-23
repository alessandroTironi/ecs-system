project(ecs-runner)
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

# SDL2_image dependency
find_package(SDL2_image 2.8 QUIET)
if (NOT SDL2_image_FOUND)
    FetchContent_Declare(
        SDL2_image
        DOWNLOAD_EXTRACT_TIMESTAMP OFF
        URL https://github.com/libsdl-org/SDL_image/releases/download/release-2.8.5/SDL2_image-devel-2.8.5-VC.zip
    )
    FetchContent_GetProperties(SDL2_image)
    if (NOT SDL2_image_POPULATED)
        FetchContent_MakeAvailable(SDL2_image)

        message(STATUS "SDL2_image_SOURCE_DIR: ${sdl2_image_SOURCE_DIR}")

        set(SDL2_image_INCLUDE_DIR "${sdl2_image_SOURCE_DIR}/include")
        set(SDL2_image_LIBRARY "${sdl2_image_SOURCE_DIR}/lib/x64/SDL2_image.lib")
    endif()
endif()

# SDL2_ttf dependency
find_package(SDL2_ttf 2.20 QUIET)
if (NOT SDL2_ttf_FOUND)
    FetchContent_Declare(
        SDL2_ttf
        DOWNLOAD_EXTRACT_TIMESTAMP OFF
        URL https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.20.2/SDL2_ttf-devel-2.20.2-VC.zip
    )
    FetchContent_GetProperties(SDL2_ttf)
    if (NOT SDL2_ttf_POPULATED)
        FetchContent_MakeAvailable(SDL2_ttf)

        message(STATUS "SDL2_ttf_SOURCE_DIR: ${sdl2_ttf_SOURCE_DIR}")

        set(SDL2_ttf_INCLUDE_DIR "${sdl2_ttf_SOURCE_DIR}/include")
        set(SDL2_ttf_LIBRARY "${sdl2_ttf_SOURCE_DIR}/lib/x64/SDL2_ttf.lib")
    endif()
endif()

include_directories("${ECS_CORE_INCLUDE_DIR}" "include" "${SDL2_INCLUDE_DIR}" "${SDL2_image_INCLUDE_DIR}" "${SDL2_ttf_INCLUDE_DIR}")

file(GLOB_RECURSE SOURCES "include/*.h" "include/*.cpp" "src/*.h" "src/*.cpp")

# Add source to this project's executable.
add_executable(ecs-runner ${SOURCES})

# Link against SDL2, SDL2_image and SDL2_ttf
target_link_libraries(ecs-runner 
    PRIVATE ${SDL2_LIBRARY} ${SDL2MAIN_LIBRARY} ${SDL2_image_LIBRARY} ${SDL2_ttf_LIBRARY}
    PRIVATE ecs-core
)

# Copy SDL2.dll to target folder
add_custom_command(TARGET ecs-runner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    "${sdl2_SOURCE_DIR}/lib/x64/SDL2.dll"
    $<TARGET_FILE_DIR:ecs-runner>)

# Copy SDL2_image.dll to target folder
add_custom_command(TARGET ecs-runner POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${sdl2_image_SOURCE_DIR}/lib/x64/SDL2_image.dll"
    $<TARGET_FILE_DIR:ecs-runner>)

# Copy SDL2_ttf.dll to target folder
add_custom_command(TARGET ecs-runner POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${sdl2_ttf_SOURCE_DIR}/lib/x64/SDL2_ttf.dll"
    $<TARGET_FILE_DIR:ecs-runner>)

# Copy font file to target folder
add_custom_command(TARGET ecs-runner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    "$<TARGET_FILE_DIR:ecs-runner>/fonts")
add_custom_command(TARGET ecs-runner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/fonts/arial.ttf"
    "$<TARGET_FILE_DIR:ecs-runner>/fonts/")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ecs-runner PROPERTY CXX_STANDARD 20)
endif()
