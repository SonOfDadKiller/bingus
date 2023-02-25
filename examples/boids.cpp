#include "bingus.h"

#include <iomanip>
#include <sstream>

using namespace gui;

void Start();
void Update(float dt);
void Draw();
void Reset();

GUIWindow controlWindow;
bool tickboxState;

SpriteBatch spriteBatch;
SpriteSheet spriteSheet;

struct Boid
{
	vec2 position;
	vec2 oldPosition;
	vec2 size;
	vec2 velocity;
	vec2 oldVelocity;
};

std::vector<Boid> boids;

float timestep;
float alignmentWeight = 0.f;// 0.0005f;
float cohesionWeight = 0.f;// 0.005f;
float separationWeight = 0.f;// 0.001f;
float cursorWeight = 0.008f;
float acceleration = 2.75f;
float drag = 0.00009f;
float maxSpeed = 0.55f;

int main()
{
	SetupWindow(1280, 720, "Boids");
	BingusInit();

	SetGameStartFunction(Start);
	SetGameUpdateFunction(Update);
	SetGameDrawFunction(Draw);
	
	RunGame();

	return 0;
}

void Start()
{
	//Control Window
	controlWindow.pos = vec2(25, 25);
	controlWindow.minSize = vec2(450, 500);
	controlWindow.maxSize = controlWindow.minSize;

	timestep = GetFixedTimestep();

	//Set up sprite batch
	spriteSheet = SpriteSheet("triangle.png", { { "triangle", SpriteSequence(vec2(0), vec2(128, 128), 4, 0.f) } });

	spriteBatch = SpriteBatch(VertBuffer({ VERTEX_POS, VERTEX_UV, VERTEX_COLOR }),
		Shader("world_vertcolor.vert", "sprite_vertcolor.frag", SHADER_MAIN_TEX),
		&spriteSheet);

	SetCameraSize(7.f);

	//Input Bindings
	BindInputAction(KEY_ESCAPE, HOLD, [](float dt)
	{
		ExitGame();
	});

	BindInputAction(KEY_W, HOLD, [](float dt)
	{
		TranslateCamera(vec2(0, 3) * dt);
	});

	BindInputAction(KEY_A, HOLD, [](float dt)
	{
		TranslateCamera(vec2(-3, 0) * dt);
	});

	BindInputAction(KEY_S, HOLD, [](float dt)
	{
		TranslateCamera(vec2(0, -3) * dt);
	});

	BindInputAction(KEY_D, HOLD, [](float dt)
	{
		TranslateCamera(vec2(3, 0) * dt);
	});

	BindInputAction(MOUSE_SCROLL_UP, PRESS, [](float dt)
	{
		ZoomCamera(-0.2f);
	});

	BindInputAction(MOUSE_SCROLL_DOWN, PRESS, [](float dt)
	{
		ZoomCamera(0.2f);
	});

	BindInputAction(KEY_R, PRESS, [](float dt)
	{
		Reset();
	});
}

void Reset()
{

}

void GUIControl(std::string label, float* value)
{
	Widget();
		vars.size = vec2(400, 50);
		Layout(HORIZONTAL);
			vars.size = vec2(0);
			vars.margin = Edges::All(15.f);
			vars.spacing = 4.f;
			gui::Text(label);
				vars.size = vec2(150, 50);
				vars.textAlignment = CENTER_LEFT;
			EndNode();
			Button();
				vars.size = vec2(50);
				vars.onPress = [value]() {
					*value *= 0.95f;
				};
				Image(MINUS);
					vars.pivot = vars.anchor = CENTER;
					vars.size = vec2(35);
				EndNode();
			EndNode();
			gui::Text(std::to_string(*value));
				vars.size = vec2(150, 50);
				vars.textAlignment = CENTER;
			EndNode();
			Button();
				vars.size = vec2(50);
				vars.onPress = [value]() {
					*value *= 1.05f;
				};
				Image(PLUS);
					vars.pivot = vars.anchor = CENTER;
					vars.size = vec2(35);
				EndNode();
			EndNode();
		EndNode();
	EndNode();
}

void Update(float dt)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	float menuHue = 0.6f;

	Window(&controlWindow);
		
		Layout(VERTICAL);
			vars.size = vec2(0);
			vars.margin = Edges::All(0.001f);

			GUIControl("Timestep: ", &timestep);
			
		EndNode();
	



	//controlsRoot = new UIImage(vec2(-25, -25), vec2(520, 500), TOP_RIGHT, TOP_RIGHT);
	//controlsRoot->spriteRow = 0;
	//controlsRoot->spriteColumn = 0;
	//controlsRoot->color = hsv(vec4(0.6, 0.4, 0.4, 0.5));

	////Controllable Fields
	//float* fieldVariables[8] = {
	//	&stepTime,
	//	& alignmentWeight,
	//	& cohesionWeight,
	//	& separationWeight,
	//	& cursorWeight,
	//	& acceleration,
	//	& drag,
	//	& maxSpeed
	//};

	//std::string fieldTitles[8] = {
	//	"Timestep:",
	//	"Alignment Weight:",
	//	"Cohesion Weight:",
	//	"Separation Weight:",
	//	"Cursor Weight:",
	//	"Acceleration:",
	//	"Drag:",
	//	"Max Speed:"
	//};

	//for (int i = 0; i < 8; i++)
	//{
	//	ControlsField* field = new ControlsField();
	//	controllableFields.push_back(field);
	//	field->data = fieldVariables[i];
	//	
	//	UINode* base = new UINode(controlsRoot, vec2(20, -20 - i * 45.f), vec2(600, 40), TOP_LEFT, TOP_LEFT);

	//	UIText* title = new UIText(base, vec2(0, 0), vec2(250, 40), CENTER_LEFT, CENTER_LEFT);
	//	title->fontSize = 1.f;
	//	title->color = vec4(1.f);
	//	title->data = fieldTitles[i];

	//	UIButton* minus = new UIButton(base, vec2(260, 0), vec2(40, 40), CENTER_LEFT, CENTER_LEFT);
	//	minus->spriteColumn = 8;
	//	minus->color = hsv(vec4(menuHue, 0.4, 0.8, 1));

	//	UIButton* plus = new UIButton(base, vec2(310, 0), vec2(40, 40), CENTER_LEFT, CENTER_LEFT);
	//	plus->spriteColumn = 9;
	//	plus->color = hsv(vec4(menuHue, 0.4, 0.8, 1));

	//	field->text = new UIText(base, vec2(360, 0), vec2(130, 40), CENTER_LEFT, CENTER_LEFT);
	//	field->text->fontSize = 1.f;
	//	field->text->color = vec4(1.f);
	//	field->text->data = std::to_string(*field->data);

	//	minus->onPress = [field]() {
	//		(*field->data) *= 0.95f;
	//		field->text->data = std::to_string(*field->data);
	//	};

	//	plus->onPress = [field]() {
	//		(*field->data) *= 1.05f;
	//		field->text->data = std::to_string(*field->data);
	//	};
	//}

	//resetButton = new UIButton(controlsRoot, vec2(0, 16), vec2(150, 40), BOTTOM_CENTER, BOTTOM_CENTER);
	//resetButton->spriteRow = 0;
	//resetButton->spriteColumn = 0;
	//resetButton->color = hsv(vec4(menuHue, 0.4, 0.8, 1));
	//resetButton->onPress = ECSTestReset;

	//resetButtonText = new UIText(resetButton, vec2(0), vec2(150, 40), CENTER, CENTER);
	//resetButtonText->color = vec4(1.f);
	//resetButtonText->fontSize = 1.f;
	//resetButtonText->data = "Reset";

	//displayText = new UIText(vec2(22, -22), vec2(1000.f, 100.f), TOP_LEFT, TOP_LEFT);
	//displayText->font = Fonts::arial;
	//displayText->fontSize = 1.f;


	EndNode();
	
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << GetAvgFrameTime() * 1000.f;
	gui::Text("fps: " + std::to_string(GetFPS()) + "(" + stream.str() + "ms)");
		gui::vars.margin = Edges::All(25);
		gui::vars.size = vec2(0);
	gui::EndNode();

	//Logic
	SetFixedTimestep(timestep);
}

void Draw()
{
	
}
