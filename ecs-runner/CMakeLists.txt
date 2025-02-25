project(ecs-runner)

find_package(SDL2 REQUIRED)
if(SDL2_FOUND)
	get_property(SDL2_DLL TARGET SDL2::SDL2 PROPERTY IMPORTED_LOCATION)
endif() 

find_package(SDL2_image REQUIRED)
if(SDL2_image_FOUND)
	get_property(SDL2_image_INCLUDE_DIRS TARGET SDL2_image::SDL2_image PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
	get_property(SDL2_image_DLL TARGET SDL2_image::SDL2_image PROPERTY IMPORTED_LOCATION)
endif() 

include_directories("${ECS_CORE_INCLUDE_DIR}" "include" "${SDL2_INCLUDE_DIRS}" "${SDL2_image_INCLUDE_DIRS}")

file(GLOB_RECURSE SOURCES "include/*.h" "include/*.cpp" "src/*.h" "src/*.cpp")

# Add source to this project's executable.
add_executable (ecs-runner ${SOURCES})
target_link_libraries(ecs-runner PRIVATE SDL2::SDL2main PRIVATE SDL2::SDL2 PRIVATE SDL2_image::SDL2_image PRIVATE ecs-core)

# copy SDl2.dll to target folder
#add_custom_command(TARGET ecs-runner POST_BUILD
#	COMMAND ${CMAKE_COMMAND} -E copy_if_different 
#	"${SDL2_DLL}"
#	$<TARGET_FILE_DIR: ecs-runner>)

# Copy SDL2_image.dll to target folder
#add_custom_command(TARGET  ecs-runner POST_BUILD 
#	COMMAND ${CMAKE_COMMAND} -E copy_if_different
#	"${SDL2_image_DLL}"
#	$<TARGET_FILE_DIR: ecs-runner>)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ecs-runner PROPERTY CXX_STANDARD 20)
endif()