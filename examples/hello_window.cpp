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

static UIText* fpsCounter;

void Start()
{
	fpsCounter = new UIText(vec2(22, -22), vec2(500.f, 50.f), TOP_LEFT, TOP_LEFT);

	BindInputAction(KEY_ESCAPE, HOLD, [](float dt)
	{
		ExitGame();
	});
}

void Update(float dt)
{
	fpsCounter->data = "fps: " + std::to_string(GetFPS()) + "(" + std::to_string(GetAvgFrameTime()) + "ms)";
}

void Draw()
{
	SetClearColor(vec4(0.5f + cos(GetTime()) * 0.2f, 0.5f, 0.5f + sin(GetTime()) * 0.2f, 1.f));
	DrawDebugText(DEBUG_WORLD, vec3(0), 0.3f, vec4(1, 1, 1, 1), "Hello World!");
}
