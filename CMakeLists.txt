cmake_minimum_required(VERSION 3.23.0)

set(LIB_NAME "bingus")
project(${LIB_NAME})

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
	"src/bingus.h"
)

set (CMAKE_CXX_STANDARD 17)

find_library(lib_glfw glfw NAMES glfw3 libglfw3 REQUIRED PATHS ${PROJECT_SOURCE_DIR}/lib/)

add_library(${LIB_NAME} ${BINGUS_SOURCE} ${BINGUS_HEADERS})

target_include_directories(${LIB_NAME}
	PUBLIC "include"
)

target_link_directories(${LIB_NAME}
	PUBLIC "lib"
)

target_link_libraries(${LIB_NAME}
	${lib_glfw}
)

#Examples

set(EXAMPLE1 hello_window)
add_executable(${EXAMPLE1} "examples/hello_window.cpp")
target_link_libraries(${EXAMPLE1} ${LIB_NAME})
target_include_directories(${EXAMPLE1} PUBLIC "src")

# Set EXAMPLE1 to be the default startup project in visual studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXAMPLE1})

set(EXAMPLE2 sprites)
add_executable(${EXAMPLE2} "examples/sprites.cpp")
target_link_libraries(${EXAMPLE2} ${LIB_NAME})
target_include_directories(${EXAMPLE2} PUBLIC "src")

set(EXAMPLE3 text)
add_executable(${EXAMPLE3} "examples/text.cpp")
target_link_libraries(${EXAMPLE3} ${LIB_NAME})
target_include_directories(${EXAMPLE3} PUBLIC "src")

#Copy resources into proj directory where projets can read them
file(COPY ${PROJECT_SOURCE_DIR}/res/ DESTINATION ${PROJECT_BINARY_DIR}/res/)

