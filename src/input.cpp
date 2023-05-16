#include "bingus.h"
#include <vector>
#include <algorithm>
#include "glm/ext/matrix_projection.hpp"

InputListener globalInputListener;
vec2 mousePosition;
vec3 mouseWorldPosition;
vec2 mouseDelta;
vec3 mouseWorldDelta;
vec2 mousePrevPosition;
vec3 mousePrevWorldPosition;

std::string* inputString;
float inputStringTimer;

static std::vector<InputListener*> listeners;
static std::vector<InputEvent> eventBuffer;
static std::vector<InputState> keyStates;

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	InputEvent event;

	switch (action)
	{
		case GLFW_PRESS: event.state = PRESS; break;
		case GLFW_RELEASE: event.state = RELEASE; break;
		default: return;
	}

	switch (button)
	{
		case GLFW_MOUSE_BUTTON_LEFT: event.key = MOUSE_LEFT; break;
		case GLFW_MOUSE_BUTTON_RIGHT: event.key = MOUSE_RIGHT; break;
		case GLFW_MOUSE_BUTTON_MIDDLE: event.key = MOUSE_MIDDLE; break;
		default: return;
	}

	eventBuffer.push_back(event);
}

void MouseScrollCallback(GLFWwindow* window, double x, double y)
{
	InputEvent event;

	event.key = y > 0 ? MOUSE_SCROLL_UP : MOUSE_SCROLL_DOWN;
	event.state = PRESS;
	eventBuffer.push_back(event);
}

void CharacterCallback(GLFWwindow* window, u32 codepoint)
{
	if (inputString != nullptr)
	{
		inputString->push_back(codepoint);
		inputStringTimer = 0.f;
	}
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	InputEvent event;
	
	switch (action)
	{
		case GLFW_PRESS:
			event.state = PRESS;
			if (key == GLFW_KEY_BACKSPACE && inputString != nullptr && inputString->size() != 0)
			{
				inputString->erase(inputString->end() - 1);
				inputStringTimer = 0.f;
			}
			break;
		case GLFW_RELEASE: event.state = RELEASE; break;
		case GLFW_REPEAT:
			if (key == GLFW_KEY_BACKSPACE && inputString != nullptr && inputString->size() != 0)
			{
				inputString->erase(inputString->end() - 1);
				inputStringTimer = 0.f;
			}
			else
			{
				return;
			}
			break;
		default: return;
	}

	switch (key)
	{
		//TODO: Fill out rest of keyboard
		case GLFW_KEY_A: event.key = KEY_A; break;
		case GLFW_KEY_B: event.key = KEY_B; break;
		case GLFW_KEY_C: event.key = KEY_C; break;
		case GLFW_KEY_D: event.key = KEY_D; break;
		case GLFW_KEY_E: event.key = KEY_E; break;
		case GLFW_KEY_F: event.key = KEY_F; break;
		case GLFW_KEY_G: event.key = KEY_G; break;
		case GLFW_KEY_H: event.key = KEY_H; break;
		case GLFW_KEY_I: event.key = KEY_I; break;
		case GLFW_KEY_J: event.key = KEY_J; break;
		case GLFW_KEY_K: event.key = KEY_K; break;
		case GLFW_KEY_L: event.key = KEY_L; break;
		case GLFW_KEY_M: event.key = KEY_M; break;
		case GLFW_KEY_N: event.key = KEY_N; break;
		case GLFW_KEY_O: event.key = KEY_O; break;
		case GLFW_KEY_P: event.key = KEY_P; break;
		case GLFW_KEY_Q: event.key = KEY_Q; break;
		case GLFW_KEY_R: event.key = KEY_R; break;
		case GLFW_KEY_S: event.key = KEY_S; break;
		case GLFW_KEY_T: event.key = KEY_T; break;
		case GLFW_KEY_U: event.key = KEY_U; break;
		case GLFW_KEY_V: event.key = KEY_V; break;
		case GLFW_KEY_W: event.key = KEY_W; break;
		case GLFW_KEY_X: event.key = KEY_X; break;
		case GLFW_KEY_Y: event.key = KEY_Y; break;
		case GLFW_KEY_Z: event.key = KEY_Z; break;
		case GLFW_KEY_SPACE: event.key = KEY_SPACE; break;
		case GLFW_KEY_ESCAPE: event.key = KEY_ESCAPE; break;
		case GLFW_KEY_BACKSPACE: event.key = KEY_BACKSPACE; break;
			//
			//break;
		default: return;
	}

	eventBuffer.push_back(event);
}

void InitializeInput(GLFWwindow* window)
{
	//Set up cursor mode
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	//Set up input callbacks
	glfwSetScrollCallback(window, MouseScrollCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCharCallback(window, CharacterCallback);

	//Initialize key states
	keyStates.reserve(KEY_LAST - 1);

	for (u32 key = 0; key != KEY_LAST; key++)
	{
		keyStates.push_back(UP);
	}

	//Initialize global listener
	globalInputListener.blocking = false;
	globalInputListener.priority = 0;
	listeners.push_back(&globalInputListener);
}

bool CompareInputListeners(const InputListener* a, const InputListener* b)
{
	return a->priority < b->priority;
}

void SortListeners()
{
	//Sort input list
	sort(listeners.begin(), listeners.end(), CompareInputListeners);
}

void RegisterInputListener(InputListener* listener)
{
	listeners.push_back(listener);
	SortListeners();
}

void UnregisterInputListener(InputListener* listener)
{
	auto it = std::find(listeners.begin(), listeners.end(), listener);
	if (it != listeners.end())
	{
		listeners.erase(it);
		SortListeners();
	}
}

vec2 PixelToWorld(vec2 pixelCoord)
{
	pixelCoord = (pixelCoord / GetWindowSize()) * 2.f - 1.f;
	vec4 origin = cameraViewProjInverse * vec4(pixelCoord, 0.f, 1.f);
	origin.w = 1.0f / origin.w;
	return vec2(origin.x, origin.y) * origin.w;
}

vec3 PixelToWorld(vec3 pixelCoord)
{
	pixelCoord = (pixelCoord / vec3(GetWindowSize(), 0)) * 2.f - 1.f;
	vec4 origin = cameraViewProjInverse * vec4(pixelCoord.x, pixelCoord.y, 0.f, 1.f);
	origin.w = 1.0f / origin.w;
	return vec3(origin.x, origin.y, origin.z) * origin.w;
}

vec2 WorldToPixel(vec2 worldCoord)
{
	vec4 origin = cameraViewProj * vec4(worldCoord, 0.f, 1.f);
	return vec2(origin.x, -origin.y) * GetWindowSize() / 2.f;
}

void CalculateWorldMousePos(GLFWwindow* window)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	//Step mouse position
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	mousePrevPosition = mousePosition;
	mousePosition = vec2(x, GetWindowSize().y - y);
	mouseDelta = mousePosition - mousePrevPosition;

	mousePrevWorldPosition = PixelToWorld(vec3(mousePrevPosition.x, mousePrevPosition.y, 0));
	mouseWorldPosition = PixelToWorld(vec3(mousePosition.x, mousePosition.y, 0));
	mouseWorldDelta = mouseWorldPosition - mousePrevWorldPosition;
}

InputListener::InputListener(i32 priority, bool blocking)
{
	this->priority = priority;
	this->blocking = blocking;
}

void InputListener::BindAction(u32 key, InputState state, InputCallback callback)
{
	BindAction(key, state, false, callback);
}

void InputListener::BindAction(u32 key, InputState state, bool blocking, InputCallback callback)
{
	InputEvent event = { key, state };
	InputBinding binding = { callback, blocking };

	bindings[event] = binding;
}

void InputListener::UnbindAction(u32 key, InputState state)
{
	bindings.erase({ key, state });
}

void InputListener::SetPriority(i32 priority)
{
	this->priority = priority;
	SortListeners();
}

void UpdateInput(GLFWwindow* window, float dt)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	CalculateWorldMousePos(window);

	//Process events in buffer
	for (auto itEvent = eventBuffer.begin(); itEvent != eventBuffer.end(); itEvent++)
	{
		keyStates[itEvent->key] = itEvent->state;
	}

	//Clear that mother fucking buffer god damn
	eventBuffer.clear();

	for (u32 key = 0; key != KEY_LAST; key++)
	{
		//Send event to listeners
		for (auto itListener = listeners.begin(); itListener != listeners.end(); itListener++)
		{
			InputEvent event = { key, keyStates[key] };
			auto entry = (*itListener)->bindings.find(event);

			if (entry != (*itListener)->bindings.end())
			{
				//Binding for this event exists, trigger callback
				entry->second.callback();

				//Block if either listener or binding is set to blocking
				if (entry->second.blocking || (*itListener)->blocking) break;
			}
		}

		//Update event
		if (keyStates[key] == PRESS)
		{
			keyStates[key] = HOLD;
		}
		else if (keyStates[key] == RELEASE)
		{
			keyStates[key] = UP;
		}
	}

	//Reset scroll state, as it should only be in either PRESS or UP state
	keyStates[MOUSE_SCROLL_UP] = UP;
	keyStates[MOUSE_SCROLL_DOWN] = UP;

	//Update input string timer
	inputStringTimer += dt;
}
