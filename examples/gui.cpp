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

bool tickboxState[6];

void Start()
{
	BindInputAction(KEY_ESCAPE, HOLD, [](float dt)
	{
		ExitGame();
	});
}

using namespace gui;

void Update(float dt)
{
	for (int i = 0; i < 30; i++)
	{
		Image(BOX);
			vars.pos = vec2(50.f + i * (50.f + sin(GetTime() * 0.6f) * 25.f), cos(GetTime() + i) * 400.f);
			vars.size = vec2(100.f + cos(GetTime()) * 25.f, 100.f + sin(GetTime()) * 25.f);
			vars.pivot = CENTER_LEFT;
			vars.anchor = CENTER_LEFT;
			vars.color = vec4(i / 20.f, 1.f, 1.f, 1.f);
		EndNode();
	}

	gui::Text("fps: " + std::to_string(GetFPS()) + "(" + std::to_string(GetAvgFrameTime()) + "ms)");
		vars.margin = Edges::All(25);
		vars.size = vec2(0);
	EndNode();
}

void Draw()
{
	
}
