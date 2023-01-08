#include "bingus.h"

void Start();
void Update(float dt);
void Draw();

int main()
{
	SetupWindow(1280, 720);
	BingusInit();

	SetGameStartFunction(Start);
	SetGameUpdateFunction(Update);
	SetGameDrawFunction(Draw);
	
	RunGame();

	return 0;
}

void Start()
{
	BindInputAction(KEY_ESCAPE, HOLD, [](float dt)
	{
		ExitGame();
	});
}

bool tickboxState;

void Update(float dt)
{
	std::string text = "fps: " + std::to_string(GetFPS()) + "(" + std::to_string(GetAvgFrameTime()) + "ms)";
	GUIText(vec2(22, -22), vec2(500.f, 50.f), TOP_LEFT, TOP_LEFT, text, 0.5f, Fonts::arial, vec4(1.f), TOP_LEFT);

	
	/*GUIWidgetBegin(vec2(200, -100), vec2(400, 500), TOP_LEFT, TOP_LEFT);
		GUIImage(vec2(0), vec2(400, 500), TOP_LEFT, TOP_LEFT, vec4(0.7f, 0.1, 0.1, 0.8f), 0);
	GUIWidgetEnd();*/
	
	if (GUIButton(vec2(22, -122), vec2(300.f, 300.f) + vec2(sin(GetTime()), cos(GetTime())) * 200.f, TOP_LEFT, TOP_LEFT, PRESS, vec4(1, 0.4f, 0.6f, 1)))
	{
		std::cout << "Button Pressed!\n";
	}

	//tickboxState = GUITickbox(vec2(22, -222), vec2(60.f, 60.f), TOP_LEFT, TOP_LEFT, vec4(1), tickboxState);
}

void Draw()
{
	
}
