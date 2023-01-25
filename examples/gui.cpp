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

using namespace gui;
bool tickboxState[6];

void Update(float dt)
{
	Image(BOX);
		vars.anchor = CENTER;
		vars.pivot = CENTER;
		vars.size = vec2(400, 600);
		Layout(VERTICAL);
			vars.margin = Edges::All(15.f);
			vars.size = vec2(0);
			vars.spacing = 10.f;
			for (int i = 0; i < 6; i++)
			{ 
				Widget();
					vars.size = vec2(0.f, 60.f);
					vars.margin = Edges::All(1.f);
					gui::Text("Option " + std::to_string(i) + ":");
						vars.textAlignment = CENTER_LEFT;
						vars.size = vec2(0);
						vars.margin = Edges::All(10.f);
						vars.color = vec4(0, 0, 0, 1);
					EndNode();
					Image(BOX);
						vars.anchor = vars.pivot = CENTER_RIGHT;
						vars.size = vec2(60.f);
						Tickbox(&tickboxState[i]);
							vars.margin = Edges::All(1.f);
							vars.size = vec2(0);
						EndNode();
					EndNode();
				EndNode();
			}
		EndNode();
	EndNode();

	Image(BOX);
		vars.pos = vec2(50, 0);
		vars.size = vec2(200, 300);
		vars.anchor = CENTER_LEFT;
		vars.pivot = CENTER_LEFT;
		Image(BOX);
			vars.size = vec2(0, 50);
			vars.margin = Edges::All(25);
			vars.anchor = CENTER;
		EndNode();
	EndNode();

	gui::Text("fps: " + std::to_string(GetFPS()) + "(" + std::to_string(GetAvgFrameTime()) + "ms)");
		gui::vars.margin = Edges::All(25);
		gui::vars.size = vec2(0);
	gui::EndNode();
}

void Draw()
{
	
}
