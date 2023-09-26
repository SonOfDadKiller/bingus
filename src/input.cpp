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

static std::vector<InputListener*> listeners;
static std::vector<InputEvent> eventBuffer;
static std::vector<InputState> keyStates;
static std::vector<u32> keyModifierStates;

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	InputEvent event = { KEY_LAST, UP, KEY_MOD_NONE };

	if ((mods & GLFW_MOD_CONTROL) == GLFW_MOD_CONTROL) event.modifier |= KEY_MOD_CONTROL;
	if ((mods & GLFW_MOD_ALT) == GLFW_MOD_ALT) event.modifier |= KEY_MOD_ALT;
	if ((mods & GLFW_MOD_SHIFT) == GLFW_MOD_SHIFT) event.modifier |= KEY_MOD_SHIFT;

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
	InputEvent event = { KEY_LAST, UP, KEY_MOD_NONE };

	event.key = y > 0 ? MOUSE_SCROLL_UP : MOUSE_SCROLL_DOWN;
	event.state = PRESS;
	eventBuffer.push_back(event);
}

void CharacterCallback(GLFWwindow* window, u32 codepoint)
{
	for (auto itListener = listeners.begin(); itListener != listeners.end(); itListener++)
	{
		if ((*itListener)->onCharacterTyped != nullptr)
		{
			(*itListener)->onCharacterTyped(codepoint);
			if ((*itListener)->blocking) return;
		}
	}
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	InputEvent event = { KEY_LAST, UP, KEY_MOD_NONE };

	if ((mods & GLFW_MOD_CONTROL) == GLFW_MOD_CONTROL && key != GLFW_KEY_LEFT_CONTROL && key != GLFW_KEY_RIGHT_CONTROL)
	{
		event.modifier |= KEY_MOD_CONTROL;
	}

	if ((mods & GLFW_MOD_ALT) == GLFW_MOD_ALT && key != GLFW_KEY_LEFT_ALT && key != GLFW_KEY_RIGHT_ALT)
	{
		event.modifier |= KEY_MOD_ALT;
	}

	if ((mods & GLFW_MOD_SHIFT) == GLFW_MOD_SHIFT && key != GLFW_KEY_LEFT_SHIFT && key != GLFW_KEY_RIGHT_SHIFT)
	{
		event.modifier |= KEY_MOD_SHIFT;
	}
	
	switch (action)
	{
		case GLFW_PRESS:
			event.state = PRESS;
			if (key == GLFW_KEY_BACKSPACE)
			{
				CharacterCallback(window, 8); //Backspace in unicode
			}
			else if (key == GLFW_KEY_ENTER)
			{
				CharacterCallback(window, 133);
			}
			else if (key == GLFW_KEY_LEFT)
			{
				CharacterCallback(window, 5); //We use ENQ and ACK for left and right arrow, because idk what they are
			}
			else if (key == GLFW_KEY_RIGHT)
			{
				CharacterCallback(window, 6);
			}
			break;
		case GLFW_RELEASE: event.state = RELEASE; break;
		case GLFW_REPEAT:
			if (key == GLFW_KEY_BACKSPACE)
			{
				CharacterCallback(window, 8);
			}
			else if (key == GLFW_KEY_ENTER)
			{
				CharacterCallback(window, 133);
			}
			else if (key == GLFW_KEY_LEFT)
			{
				CharacterCallback(window, 5);
			}
			else if (key == GLFW_KEY_RIGHT)
			{
				CharacterCallback(window, 6);
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

		case GLFW_KEY_0: event.key = KEY_0; break;
		case GLFW_KEY_1: event.key = KEY_1; break;
		case GLFW_KEY_2: event.key = KEY_2; break;
		case GLFW_KEY_3: event.key = KEY_3; break;
		case GLFW_KEY_4: event.key = KEY_4; break;
		case GLFW_KEY_5: event.key = KEY_5; break;
		case GLFW_KEY_6: event.key = KEY_6; break;
		case GLFW_KEY_7: event.key = KEY_7; break;
		case GLFW_KEY_8: event.key = KEY_8; break;
		case GLFW_KEY_9: event.key = KEY_9; break;

		case GLFW_KEY_APOSTROPHE: event.key = KEY_APOSTROPHE; break;
		case GLFW_KEY_COMMA: event.key = KEY_COMMA; break;
		case GLFW_KEY_MINUS: event.key = KEY_MINUS; break;
		case GLFW_KEY_PERIOD: event.key = KEY_PERIOD; break;
		case GLFW_KEY_SLASH: event.key = KEY_SLASH; break;
		case GLFW_KEY_BACKSLASH: event.key = KEY_BACKSLASH; break;
		case GLFW_KEY_EQUAL: event.key = KEY_EQUAL; break;
		case GLFW_KEY_LEFT_BRACKET: event.key = KEY_LEFT_BRACKET; break;
		case GLFW_KEY_RIGHT_BRACKET: event.key = KEY_RIGHT_BRACKET; break;
		case GLFW_KEY_GRAVE_ACCENT: event.key = KEY_GRAVE_ACCENT; break;
		case GLFW_KEY_SEMICOLON: event.key = KEY_SEMICOLON; break;

		case GLFW_KEY_SPACE: event.key = KEY_SPACE; break;
		case GLFW_KEY_ESCAPE: event.key = KEY_ESCAPE; break;
		case GLFW_KEY_ENTER: event.key = KEY_ENTER; break;
		case GLFW_KEY_TAB: event.key = KEY_TAB; break;
		case GLFW_KEY_BACKSPACE: event.key = KEY_BACKSPACE; break;

		case GLFW_KEY_INSERT: event.key = KEY_INSERT; break;
		case GLFW_KEY_DELETE: event.key = KEY_DELETE; break;
		case GLFW_KEY_UP: event.key = KEY_UP; break;
		case GLFW_KEY_RIGHT: event.key = KEY_RIGHT; break;
		case GLFW_KEY_DOWN: event.key = KEY_DOWN; break;
		case GLFW_KEY_LEFT: event.key = KEY_LEFT; break;
		case GLFW_KEY_PAGE_UP: event.key = KEY_PAGE_UP; break;
		case GLFW_KEY_PAGE_DOWN: event.key = KEY_PAGE_DOWN; break;
		case GLFW_KEY_HOME: event.key = KEY_HOME; break;
		case GLFW_KEY_END: event.key = KEY_END; break;
		case GLFW_KEY_CAPS_LOCK: event.key = KEY_CAPS_LOCK; break;
		case GLFW_KEY_SCROLL_LOCK: event.key = KEY_SCROLL_LOCK; break;
		case GLFW_KEY_NUM_LOCK: event.key = KEY_NUM_LOCK; break;
		case GLFW_KEY_PRINT_SCREEN: event.key = KEY_PRINT_SCREEN; break;
		case GLFW_KEY_PAUSE: event.key = KEY_PAUSE; break;

		case GLFW_KEY_LEFT_CONTROL: event.key = KEY_CONTROL_LEFT; break;
		case GLFW_KEY_LEFT_ALT: event.key = KEY_ALT_LEFT; break;
		case GLFW_KEY_LEFT_SHIFT: event.key = KEY_SHIFT_LEFT; break;
		case GLFW_KEY_RIGHT_CONTROL: event.key = KEY_CONTROL_RIGHT; break;
		case GLFW_KEY_RIGHT_ALT: event.key = KEY_ALT_RIGHT; break;
		case GLFW_KEY_RIGHT_SHIFT: event.key = KEY_SHIFT_RIGHT; break;
		
		case GLFW_KEY_F1: event.key = KEY_F1; break;
		case GLFW_KEY_F2: event.key = KEY_F2; break;
		case GLFW_KEY_F3: event.key = KEY_F3; break;
		case GLFW_KEY_F4: event.key = KEY_F4; break;
		case GLFW_KEY_F5: event.key = KEY_F5; break;
		case GLFW_KEY_F6: event.key = KEY_F6; break;
		case GLFW_KEY_F7: event.key = KEY_F7; break;
		case GLFW_KEY_F8: event.key = KEY_F8; break;
		case GLFW_KEY_F9: event.key = KEY_F9; break;
		case GLFW_KEY_F10: event.key = KEY_F10; break;
		case GLFW_KEY_F11: event.key = KEY_F11; break;
		case GLFW_KEY_F12: event.key = KEY_F12; break;

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
	keyStates = std::vector<InputState>(KEY_LAST, UP);
	keyModifierStates = std::vector<u32>(KEY_LAST, KEY_MOD_NONE);

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

std::string GetInputBindingName(u32 binding)
{
	static std::vector<std::string> inputBindingNames = {
		"Mouse Left", "Mouse Right", "Mouse Middle", "Mouse Scroll Up", "Mouse Scroll Down",
		"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
		"'", ",", "-", ".", "/", "\\", "=", "[", "]", "`", ";",
		"Space", "Escape", "Enter", "Tab", "Backspace", "Insert", "Delete", "Up", "Right", "Down", "Left", "Page Up", "Page Down", "Home",
		"End", "Caps Lock", "Scroll Lock", "Num Lock", "Print Screen", "Pause",
		"Left Control", "Left Alt", "Left Shift", "Right Control", "Right Alt", "Right Shift",
		"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
		"KEY_LAST"
	};

	return inputBindingNames[binding];
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
	this->onCharacterTyped = nullptr;
}

void InputListener::BindAction(u32 key, InputState state, InputCallback callback)
{
	BindAction(key, state, KEY_MOD_ANY, false, callback);
}

void InputListener::BindAction(u32 key, InputState state, std::string namedEvent)
{
	BindAction(key, state, KEY_MOD_ANY, false, namedEvent);
}

void InputListener::BindAction(u32 key, InputState state, u32 modifier, InputCallback callback)
{
	BindAction(key, state, modifier, false, callback);
}

void InputListener::BindAction(u32 key, InputState state, u32 modifier, std::string namedEvent)
{
	BindAction(key, state, modifier, false, namedEvent);
}

void InputListener::BindAction(u32 key, InputState state, u32 modifier, bool blocking, InputCallback callback)
{
	InputEvent event = { key, state, modifier };
	InputBinding binding = { callback, "", blocking};
	bindings[event] = binding;
}

void InputListener::BindAction(u32 key, InputState state, u32 modifier, bool blocking, std::string namedEvent)
{
	InputEvent event = { key, state, modifier };
	InputBinding binding = { nullptr, namedEvent, blocking };
	bindings[event] = binding;
}

void InputListener::BindNamedEvent(std::string name, InputCallback callback)
{
	namedEvents[name] = callback;
}

void InputListener::UnbindNamedEvent(std::string name)
{
	namedEvents.erase(name);
}

void InputListener::UnbindAction(u32 key, InputState state)
{
	UnbindAction(key, state, KEY_MOD_ANY);
}

void InputListener::UnbindAction(u32 key, InputState state, u32 modifier)
{
	bindings.erase({ key, state, KEY_MOD_ANY });
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
		keyModifierStates[itEvent->key] = itEvent->modifier;
	}

	//Clear that buffer - it is a good idea to remember to do this
	eventBuffer.clear();

	for (u32 key = 0; key != KEY_LAST; key++)
	{
		//Send event to listeners
		for (auto itListener = listeners.begin(); itListener != listeners.end(); itListener++)
		{
			//Check for event with matching modifier
			InputEvent event = { key, keyStates[key], keyModifierStates[key] };
			auto entry = (*itListener)->bindings.find(event);

			if (entry != (*itListener)->bindings.end())
			{
				//Binding for this event exists, trigger callback, or named event if null
				if (entry->second.callback == nullptr)
				{
					(*itListener)->namedEvents[entry->second.eventName]();
				}
				else
				{
					entry->second.callback();
				}

				//Block if either listener or binding is set to blocking
				if (entry->second.blocking || (*itListener)->blocking) break;
			}

			//Check for event with 'any' modifier
			InputEvent anyModEvent = { key, keyStates[key], KEY_MOD_ANY };
			auto anyModEntry = (*itListener)->bindings.find(anyModEvent);

			if (anyModEntry != (*itListener)->bindings.end())
			{
				//Binding for this event exists, trigger callback, or named event if null
				if (anyModEntry->second.callback == nullptr)
				{
					(*itListener)->namedEvents[anyModEntry->second.eventName]();
				}
				else
				{
					anyModEntry->second.callback();
				}

				//Block if either listener or binding is set to blocking
				if (anyModEntry->second.blocking || (*itListener)->blocking) break;
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
}
