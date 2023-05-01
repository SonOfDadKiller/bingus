#include "bingus.h"

#include <iomanip>
#include <sstream>

void Start();
void Update(float dt);
void Draw();

int main()
{
	SetupWindow(1280, 720, "GUI");
	BingusInit();

	SetGameStartFunction(Start);
	SetGameUpdateFunction(Update);
	SetGameDrawFunction(Draw);
	
	RunGame();

	return 0;
}

void Start()
{
	globalInputListener.BindAction(KEY_ESCAPE, HOLD, []()
	{
		ExitGame();
	});
}

using namespace gui;

std::vector<GUIWindow> windows;
bool tickboxState;

std::string text;

void Update(float dt)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	for (auto window = windows.begin(); window != windows.end(); window++)
	{
		Window(&(*window));
			Mask();
				vars.size = vec2(0);
				vars.margin = Edges(10, 10, 80, 10);
				TextField(&text);
					vars.size = vec2(0);
					vars.margin = Edges::All(0.01f);
					vars.textHeightInPixels = 25.f;
				EndNode();
			EndNode();
		EndNode();
	}
	
	Image(BOX);
		vars.pos = vec2(50, 80);
		vars.size = vec2(200, 100);
		Button();
			vars.pivot = vars.anchor = CENTER_RIGHT;
			vars.size = vec2(80);
			vars.margin = Edges::All(10);
			vars.onPress = []() {
				GUIWindow window;
				window.pos = vec2(200);
				window.size = vec2(300, 400);
				window.minSize = vec2(100, 100);
				window.maxSize = vec2(7000, 6000);
				windows.push_back(window);
			};
			Image(PLUS);
				vars.pivot = vars.anchor = CENTER;
				vars.size = vec2(60);
			EndNode();
		EndNode();
		Button();
			vars.pivot = vars.anchor = CENTER_LEFT;
			vars.size = vec2(80);
			vars.margin = Edges::All(10);
			vars.onPress = []() {
				if (windows.size() != 0) windows.pop_back();
			};
			Image(MINUS);
				vars.pivot = vars.anchor = CENTER;
				vars.size = vec2(60);
			EndNode();
		EndNode();
	EndNode();
	
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << GetAvgFrameTime() * 1000.f;
	gui::Text("fps: " + std::to_string(GetFPS()) + "(" + stream.str() + "ms)");
		gui::vars.margin = Edges::All(25);
		gui::vars.size = vec2(0);
		gui::vars.textHeightInPixels = 35.f;
	gui::EndNode();
}

void Draw()
{
	
}
