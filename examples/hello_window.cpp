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
	BindInputAction(KEY_ESCAPE, HOLD, [](float dt) {
		ExitGame();
	});
}

void Update(float dt)
{
	gui::Text("fps: " + std::to_string(GetFPS()) + "(" + std::to_string(GetAvgFrameTime()) + "ms)");
		gui::vars.margin = Edges::All(25);
		gui::vars.size = vec2(0);
	gui::EndNode();
}

void Draw()
{
	SetClearColor(vec4(0.5f + cos(GetTime()) * 0.2f, 0.5f, 0.5f + sin(GetTime()) * 0.2f, 1.f));
	DrawDebugText(DEBUG_WORLD, vec3(0), 0.3f, vec4(1, 1, 1, 1), "Hello World!");
}
