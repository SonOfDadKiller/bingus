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
	"src/ui.cpp"
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

# Git submodule dependencies
add_subdirectory(lib/glm)
add_subdirectory(lib/glfw)

# Link library dependencies
target_link_libraries(${LIB_NAME} glfw)



#Examples

#Example 1 - hello_window
add_executable(hello_window "examples/hello_window.cpp")
target_link_libraries(hello_window ${LIB_NAME})

# Set EXAMPLE1 to be the default startup project in visual studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT hello_window)

#Example 2 - sprites
add_executable(sprites "examples/sprites.cpp")
target_link_libraries(sprites ${LIB_NAME})

#Example 3 - text
add_executable(text "examples/text.cpp")
target_link_libraries(text ${LIB_NAME})

#Copy resources into proj directory where projects can read them
file(COPY ${PROJECT_SOURCE_DIR}/res/ DESTINATION ${PROJECT_BINARY_DIR}/res/)

