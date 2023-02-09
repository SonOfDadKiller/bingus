#include "bingus.h"

#include <iomanip>
#include <sstream>

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

std::vector<GUIWindow> windows;


vec2 prevMousePos = vec2(0);
bool moreWidgets = false;
bool lessWidgets = false;

bool pressButton;

void Update(float dt)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	for (auto window = windows.begin(); window != windows.end(); window++)
	{
		Window(&(*window));
			Button();

			vars.pos = vec2(50);

			vars.onPress = []() {
				std::cout << "WTF\n";
			};
			
			EndNode();
		EndNode();
	}
	
	Image(BOX);
		vars.pos = vec2(50);
		vars.size = vec2(200, 100);

		Button(&moreWidgets, PRESS);
			vars.pivot = vars.anchor = CENTER_RIGHT;
			vars.size = vec2(80);
			vars.margin = Edges::All(10);
			gui::Text("+");
				vars.margin = Edges::All(0.01f);

				vars.size = vec2(0);
				vars.textAlignment = CENTER;
				vars.fontSize = 1.1f;
				vars.color = vec4(0, 0, 0, 1);
			EndNode();
		EndNode();
		Button(&lessWidgets, PRESS);
			vars.pivot = vars.anchor = CENTER_LEFT;
			vars.size = vec2(80);
			vars.margin = Edges::All(10);
			gui::Text("-");
				vars.margin = Edges::All(0.01f);
				vars.size = vec2(0);
				vars.textAlignment = CENTER;
				vars.fontSize = 1.1f;
				vars.color = vec4(0, 0, 0, 1);
			EndNode();
		EndNode();
	EndNode();

	if (moreWidgets)
	{
		GUIWindow window;
		window.pos = vec2(300);
		window.size = vec2(400, 300);
		window.minSize = vec2(400, 300);
		window.maxSize = vec2(700, 500);
		windows.push_back(window);
	}
	else if (lessWidgets && !windows.empty())
	{
		windows.pop_back();
	}
	

	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << GetAvgFrameTime() * 1000.f;
	gui::Text("fps: " + std::to_string(GetFPS()) + "(" + stream.str() + "ms)");
		gui::vars.margin = Edges::All(25);
		gui::vars.size = vec2(0);
	gui::EndNode();
}

void Draw()
{
	
}
