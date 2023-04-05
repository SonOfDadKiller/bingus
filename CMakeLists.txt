cmake_minimum_required(VERSION 3.23.0)

set(LIB_NAME "bingus")
project(${LIB_NAME})
set (CMAKE_CXX_STANDARD 17)

# Define source and header files
SET(BINGUS_SOURCE
	"src/glad.c"
	"src/bingus.cpp"
	"src/math.cpp"
	"src/shader.cpp"
	"src/texture.cpp"
	"src/renderer.cpp"
	"src/window.cpp"
	"src/input.cpp"
	"src/gui.cpp"
	"src/entity.cpp"
	"src/debug.cpp"
	"src/collision.cpp"
)

SET(BINGUS_HEADERS
	"include/bingus.h"
)

# Compile bingus as a library
add_library(${LIB_NAME} ${BINGUS_SOURCE} ${BINGUS_HEADERS})

# Our include directories, these will apply to examples below also
include_directories(include)
include_directories(lib/glfw/include)
include_directories(lib/glm)

option(TRACY_ENABLE "" ON)
option( TRACY_ON_DEMAND "" ON)

# Git submodule dependencies
add_subdirectory(lib/glm)
add_subdirectory(lib/glfw)
add_subdirectory(lib/tracy)

# Link library dependencies
target_link_libraries(${LIB_NAME} glfw)
target_link_libraries(${LIB_NAME} Tracy::TracyClient)



#Examples
if (${BINGUS_EXAMPLES})
	
	#Example 0 - hello
	add_executable(example_0_hello "examples/0_hello.cpp")
	target_link_libraries(example_0_hello ${LIB_NAME})
	target_link_libraries(${LIB_NAME} Tracy::TracyClient)

	# Set 'example_0_hello' to be the default startup project in visual studio
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT example_0_hello)

	#Example 1 - sprites
	add_executable(example_1_sprites "examples/1_sprites.cpp")
	target_link_libraries(example_1_sprites ${LIB_NAME})
	target_link_libraries(${LIB_NAME} Tracy::TracyClient)

	#Example 2 - text
	add_executable(example_2_text "examples/2_text.cpp")
	target_link_libraries(example_2_text ${LIB_NAME})
	target_link_libraries(${LIB_NAME} Tracy::TracyClient)

	#Example 3 - gui
	add_executable(example_3_gui "examples/3_gui.cpp")
	target_link_libraries(example_3_gui ${LIB_NAME})
	target_link_libraries(${LIB_NAME} Tracy::TracyClient)

	#Example 4 - input
	add_executable(example_4_input "examples/4_input.cpp")
	target_link_libraries(example_4_input ${LIB_NAME})
	target_link_libraries(${LIB_NAME} Tracy::TracyClient)

	#Example 5 - collision
	add_executable(example_5_collision "examples/5_collision.cpp")
	target_link_libraries(example_5_collision ${LIB_NAME})
	target_link_libraries(${LIB_NAME} Tracy::TracyClient)

	#Example 6 - boids
	add_executable(example_6_boids "examples/6_boids.cpp")
	target_link_libraries(example_6_boids ${LIB_NAME})
	target_link_libraries(${LIB_NAME} Tracy::TracyClient)

endif()

#Copy resources into proj directory where projects can read them
file(COPY ${PROJECT_SOURCE_DIR}/res/ DESTINATION ${PROJECT_BINARY_DIR}/res/)

