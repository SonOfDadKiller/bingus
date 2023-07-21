#include "bingus.h"

#include <iomanip>
#include <sstream>

void Start();
void Update(float dt);
void Draw();

std::vector<bool> bindingStates;
std::string textField;

struct FloatyText
{
	float time;
	std::string text;
};

std::vector<FloatyText> floatyTexts;

int main()
{
	SetupWindow(1280, 720, "Input");
	BingusInit();

	SetGameStartFunction(Start);
	SetGameUpdateFunction(Update);
	SetGameDrawFunction(Draw);

	RunGame();

	return 0;
}

void Start()
{
	inputString = &textField;

	bindingStates = std::vector<bool>(KEY_LAST, false);

	for (int i = 0; i < KEY_LAST; i++)
	{
		globalInputListener.BindAction(i, PRESS, KEY_MOD_NONE, [i]()
		{
			bindingStates[i] = true;
			floatyTexts.push_back({ 0, GetInputBindingName(i) });
		});

		globalInputListener.BindAction(i, PRESS, KEY_MOD_CONTROL, [i]()
		{
			bindingStates[i] = true;
			floatyTexts.push_back({ 0, "Ctrl+" + GetInputBindingName(i) });
		});

		globalInputListener.BindAction(i, PRESS, KEY_MOD_ALT, [i]()
		{
			bindingStates[i] = true;
			floatyTexts.push_back({ 0, "Alt+" + GetInputBindingName(i) });
		});

		globalInputListener.BindAction(i, PRESS, KEY_MOD_SHIFT, [i]()
		{
			bindingStates[i] = true;
			floatyTexts.push_back({ 0, "Shift+" + GetInputBindingName(i) });
		});

		globalInputListener.BindAction(i, RELEASE, [i]()
		{
			bindingStates[i] = false;
		});
	}

	globalInputListener.BindNamedEvent("Jump", []() {
		std::cout << "Jump!\n";
	});

	globalInputListener.BindAction(KEY_SPACE, PRESS, "Jump");
}

void Update(float dt)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	for (int i = 0; i < floatyTexts.size(); i++)
	{
		DrawDebugText(DEBUG_WORLD, vec2(0.5, 0.5) + vec2(0, 1) * floatyTexts[i].time * 0.4f, 0.1f, vec4(0, 1, 0, 1), floatyTexts[i].text);
		floatyTexts[i].time += dt;

		if (floatyTexts[i].time > 2.f)
		{
			floatyTexts.erase(floatyTexts.begin() + i);
			i--;
		}
	}

	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << GetAvgFrameTime() * 1000.f;
	gui::Text("fps: " + std::to_string(GetFPS()) + "(" + stream.str() + "ms)");
		gui::vars.margin = Edges::All(25);
		gui::vars.size = vec2(0);
	gui::EndNode();
}

std::string PreciseString(float value, int precision)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(precision) << value;
	return stream.str();
}

void DrawText(vec2 position, std::string data, vec4 color)
{
	Text text;
	text.data = data;
	text.position = vec3(position.x, position.y, 0);
	text.extents = vec2(2, 0.f);
	text.alignment = TOP_LEFT;
	text.textSize = 0.06f;
	text.color = color;
	text.font = LoadFont("arial.ttf", 80);
	globalRenderQueue.PushText(text);
}

void Draw()
{
	vec2 offset = vec2(-1.5f, 0.3f);

	for (int i = 0; i < KEY_LAST; i++)
	{
		std::string bindingName = GetInputBindingName(i);
		vec4 color = bindingStates[i] ? vec4(1, 0, 0, 1) : vec4(1);

		if (i != 0)
		{
			if (i % 20 == 0)
			{
				offset.x += 0.6f;
				offset.y = 0.3f;
			}
			else
			{
				offset.y -= 0.06f;
			}
		}

		DrawText(offset, bindingName, color);
	}

	DrawText(vec2(-1.5f, 0.78f), "Mouse Position (pixel): (" + PreciseString(mousePosition.x, 0) + ", " + PreciseString(mousePosition.y, 0) + ")", vec4(1));
	DrawText(vec2(-1.5f, 0.7f), "Mouse Position (world): (" + PreciseString(mouseWorldPosition.x, 2) + ", " + PreciseString(mouseWorldPosition.y, 2) + ")", vec4(1));

	DrawText(vec2(-1.5f, 0.5f), textField, vec4(1));

	bindingStates[MOUSE_SCROLL_UP] = false;
	bindingStates[MOUSE_SCROLL_DOWN] = false;
}
