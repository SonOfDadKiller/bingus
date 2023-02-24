#include "bingus.h"
#include <iostream>

GLFWwindow* window;
vec2 windowSize;

void SetupWindow(i32 width, i32 height, const char* title)
{
	//Initialize GLFW and create window
	//TODO: Load window dimensions from save/load system
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, title, nullptr, nullptr);

	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(window);
	glfwSetWindowPos(window, 200, 50);

	//Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD\n";
		return;
	}

	//Set up viewport
	glViewport(0, 0, width, height);
	windowSize = vec2(width, height);

	//Hook up window size change callback
	glfwSetFramebufferSizeCallback(window, HandleWindowSizeChange);

	//Input
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_TRUE);

	//Setup mouse input
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, HandeMouseMove);
	glfwSetScrollCallback(window, HandleMouseScroll);
}

GLFWwindow* GetWindow()
{
	return window;
}

vec2 GetWindowSize()
{
	return windowSize;
}

void HandleWindowSizeChange(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	windowSize = vec2(width, height);
	SetCameraSize(GetCameraSize(), true);
	SetGUICanvasSize(windowSize);
}

void HandeMouseMove(GLFWwindow* window, double mouseX, double mouseY)
{

}

void HandleMouseScroll(GLFWwindow* window, double scrollX, double scrollY)
{

}
