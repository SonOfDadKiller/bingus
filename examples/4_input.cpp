#include "bingus.h"

#include <iomanip>
#include <sstream>

void Start();
void Update(float dt);
void Draw();

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
	globalInputListener.BindAction(KEY_ESCAPE, HOLD, []()
	{
		ExitGame();
	});
}

using namespace gui;

std::vector<GUIWindow> windows;
bool tickboxState;

void Update(float dt)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	for (auto window = windows.begin(); window != windows.end(); window++)
	{
		Window(&(*window));
			Tickbox(&tickboxState);
				vars.size = vec2(32);
				vars.pos = vec2(25);
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
				window.pos = vec2(300);
				window.size = vec2(400, 300);
				window.minSize = vec2(400, 300);
				window.maxSize = vec2(700, 500);
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
	gui::EndNode();
}

void Draw()
{
	
}
