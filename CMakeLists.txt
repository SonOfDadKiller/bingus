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
	
	#Example 0 - hello_window
	add_executable(example0_hello_window "examples/hello_window.cpp")
	target_link_libraries(example0_hello_window ${LIB_NAME})
	target_link_libraries(${LIB_NAME} Tracy::TracyClient)

	# Set EXAMPLE1 to be the default startup project in visual studio
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT example0_hello_window )

	#Example 1 - sprites
	add_executable(example1_sprites "examples/sprites.cpp")
	target_link_libraries(example1_sprites ${LIB_NAME})
	target_link_libraries(${LIB_NAME} Tracy::TracyClient)

	#Example 2 - text
	add_executable(example2_text "examples/text.cpp")
	target_link_libraries(example2_text ${LIB_NAME})
	target_link_libraries(${LIB_NAME} Tracy::TracyClient)

	#Example 3 - gui
	add_executable(example3_gui "examples/gui.cpp")
	target_link_libraries(example3_gui ${LIB_NAME})
	target_link_libraries(${LIB_NAME} Tracy::TracyClient)

	#Example 4 - boids
	add_executable(example4_boids "examples/boids.cpp")
	target_link_libraries(example4_boids ${LIB_NAME})
	target_link_libraries(${LIB_NAME} Tracy::TracyClient)

endif()

#Copy resources into proj directory where projects can read them
file(COPY ${PROJECT_SOURCE_DIR}/res/ DESTINATION ${PROJECT_BINARY_DIR}/res/)

