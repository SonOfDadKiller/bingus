#include "bingus.h"
#include <vector>
#include "glm/ext/matrix_projection.hpp"

vec2 mousePosition;
vec3 mouseWorldPosition;

struct Action
{
	InputEvent fun;
	u32 id;
};

struct Binding
{
	std::vector<Action> pressEvents;
	std::vector<Action> downEvents;
	std::vector<Action> releaseEvents;
	InputState state;

	void AddAction(Action action, InputState _state)
	{
		GetActions(_state)->push_back(action);
	}

	void FireEvents(InputState _state, float dt)
	{
		std::vector<Action>* actions = GetActions(_state);
		if (actions == nullptr) return;

		for (auto it = actions->begin(); it != actions->end(); it++)
		{
			it->fun(dt);
		}
	}

	void FireEvents(float dt)
	{
		FireEvents(state, dt);
	}

	std::vector<Action>* GetActions(InputState _state)
	{
		switch (_state)
		{
			case PRESS: return &pressEvents;
			case HOLD: return &downEvents;
			case RELEASE: return &releaseEvents;
		}

		return nullptr;
	}
};

static u32 currentActionID;
static std::vector<Binding> bindings;

void ScrollCallback(GLFWwindow* window, double x, double y)
{
	bindings[MOUSE_SCROLL_DOWN].state = y < 0 ? PRESS : UP;
	bindings[MOUSE_SCROLL_UP].state = y > 0 ? PRESS : UP;
}

void InitializeInput(GLFWwindow* window)
{
	bindings.reserve(KEY_END);
	for (int i = 0; i < KEY_END; i++)
	{
		Binding binding;
		binding.state = UP;
		bindings.push_back(binding);
	}

	//Set up mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetScrollCallback(window, ScrollCallback);
}

u32 BindInputAction(u32 key, InputState state, InputEvent fun)
{
	Action action;
	action.fun = fun;
	action.id = currentActionID++;
	assert(bindings.size() != 0); //Forgot to initialize
	bindings[key].AddAction(action, state);
	return action.id;
}

void UnbindInputAction(u32 key, InputState state, u32 id)
{
	std::vector<Action>* actions = bindings[key].GetActions(state);
	assert(actions != nullptr);

	auto it = actions->begin();
	while (it != actions->end())
	{
		if (it->id == id)
		{
			break;
		}

		it++;
	}

	assert(it != actions->end());
	actions->erase(it);
}

void UpdateBinding(u32 key, u32 glfwState, float dt)
{
	Binding* binding = &bindings[key];

	if (glfwState == GLFW_PRESS)
	{
		switch (binding->state)
		{
			case PRESS: binding->state = HOLD; break;
			case HOLD: binding->state = HOLD; break;
			case RELEASE: binding->state = PRESS; break;
			case UP: binding->state = PRESS; break;
		}
	}
	else if (glfwState == GLFW_RELEASE)
	{
		switch (binding->state)
		{
			case PRESS: binding->state = RELEASE; break;
			case HOLD: binding->state = RELEASE; break;
			case RELEASE: binding->state = UP; break;
			case UP: binding->state = UP; break;
		}
	}

	binding->FireEvents(dt);
}

void UpdateInput(GLFWwindow* window, float dt)
{
	//TODO: Add rest of keyboard
	UpdateBinding(MOUSE_LEFT, glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT), dt);
	UpdateBinding(MOUSE_RIGHT, glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT), dt);

	UpdateBinding(KEY_A, glfwGetKey(window, GLFW_KEY_A), dt);
	UpdateBinding(KEY_B, glfwGetKey(window, GLFW_KEY_B), dt);
	UpdateBinding(KEY_C, glfwGetKey(window, GLFW_KEY_C), dt);
	UpdateBinding(KEY_D, glfwGetKey(window, GLFW_KEY_D), dt);
	UpdateBinding(KEY_E, glfwGetKey(window, GLFW_KEY_E), dt);
	UpdateBinding(KEY_F, glfwGetKey(window, GLFW_KEY_F), dt);
	UpdateBinding(KEY_G, glfwGetKey(window, GLFW_KEY_G), dt);
	UpdateBinding(KEY_H, glfwGetKey(window, GLFW_KEY_H), dt);
	UpdateBinding(KEY_I, glfwGetKey(window, GLFW_KEY_I), dt);
	UpdateBinding(KEY_J, glfwGetKey(window, GLFW_KEY_J), dt);
	UpdateBinding(KEY_K, glfwGetKey(window, GLFW_KEY_K), dt);
	UpdateBinding(KEY_L, glfwGetKey(window, GLFW_KEY_L), dt);
	UpdateBinding(KEY_M, glfwGetKey(window, GLFW_KEY_M), dt);
	UpdateBinding(KEY_N, glfwGetKey(window, GLFW_KEY_N), dt);
	UpdateBinding(KEY_O, glfwGetKey(window, GLFW_KEY_O), dt);
	UpdateBinding(KEY_P, glfwGetKey(window, GLFW_KEY_P), dt);
	UpdateBinding(KEY_Q, glfwGetKey(window, GLFW_KEY_Q), dt);
	UpdateBinding(KEY_R, glfwGetKey(window, GLFW_KEY_R), dt);
	UpdateBinding(KEY_S, glfwGetKey(window, GLFW_KEY_S), dt);
	UpdateBinding(KEY_T, glfwGetKey(window, GLFW_KEY_T), dt);
	UpdateBinding(KEY_U, glfwGetKey(window, GLFW_KEY_U), dt);
	UpdateBinding(KEY_V, glfwGetKey(window, GLFW_KEY_V), dt);
	UpdateBinding(KEY_W, glfwGetKey(window, GLFW_KEY_W), dt);
	UpdateBinding(KEY_X, glfwGetKey(window, GLFW_KEY_X), dt);
	UpdateBinding(KEY_Y, glfwGetKey(window, GLFW_KEY_Y), dt);
	UpdateBinding(KEY_Z, glfwGetKey(window, GLFW_KEY_Z), dt);

	UpdateBinding(KEY_SPACE, glfwGetKey(window, GLFW_KEY_SPACE), dt);
	UpdateBinding(KEY_ESCAPE, glfwGetKey(window, GLFW_KEY_ESCAPE), dt);

	//Step mouse position
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	mousePosition = vec2(x, GetWindowSize().y - y);

	// make cursor coordinates from -1 to +1
	float pt_x = (mousePosition.x / GetWindowSize().x) * 2.f - 1.f;
	float pt_y = (mousePosition.y / GetWindowSize().y) * 2.f - 1.f;

	//z value from 0.f to 1.f for d3d
	vec4 origin = cameraViewProjInverse * vec4(pt_x, pt_y, 0.f, 1.f);
	origin.w = 1.0f / origin.w;
	origin.x *= origin.w;
	origin.y *= origin.w;
	origin.z *= origin.w;

	mouseWorldPosition = vec3(origin.x, origin.y, origin.z);

	//mouseWorldPosition = cameraProjection * cameraView * vec4(mousePosition, 0.f, 0.f);

	//Step scroll, hacky but whatever
	bindings[MOUSE_SCROLL_UP].FireEvents(dt);
	bindings[MOUSE_SCROLL_DOWN].FireEvents(dt);
	bindings[MOUSE_SCROLL_DOWN].state = UP;
	bindings[MOUSE_SCROLL_UP].state = UP;
}

InputState GetInputState(u32 key)
{
	return bindings[key].state;
}
