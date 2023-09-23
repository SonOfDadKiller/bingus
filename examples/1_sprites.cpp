#include "bingus.h"

void Start();
void Update(float dt);
void Draw();

int main()
{
	SetupWindow(1280, 720, "Sprites");
	BingusInit();

	SetGameStartFunction(Start);
	SetGameUpdateFunction(Update);
	SetGameDrawFunction(Draw);
	
	RunGame();

	return 0;
}

SpriteBatch spriteBatch;
SpriteSheet spriteSheet;
SpriteAnimator spriteAnim;

SpriteBatch testBatch;
SpriteSheet testSheet;

static vec2 camPos;

void Start()
{
	globalInputListener.BindAction(KEY_ESCAPE, HOLD, []()
	{
		ExitGame();
	});

	globalInputListener.BindAction(MOUSE_MIDDLE, HOLD, []()
	{
		std::cout << "Mouse World Delta: " << mouseWorldDelta.x << ", " << mouseWorldDelta.y << "\n";
		SetCameraPosition(GetCameraPosition() - vec2(mouseWorldDelta.x, mouseWorldDelta.y));
	});

	//Set up sprite batch
	spriteSheet = SpriteSheet(LoadTexture("spritesheet.png"), { { "run", SpriteSequence(vec2(0), vec2(128, 128), 4, 0.f) } });
	spriteAnim = SpriteAnimator(&spriteSheet, "run", 10.f);

	spriteBatch.buffer = new VertBuffer(POS_UV_COLOR);
	spriteBatch.shader = LoadShader("world_vertcolor.vert", "sprite_vertcolor.frag");
	spriteBatch.shader->EnableUniforms(SHADER_MAIN_TEX);
	spriteBatch.sheet = &spriteSheet;
	spriteBatch.texture = spriteSheet.texture;

	vec2 uiFrameSize = vec2(128);
	testSheet = SpriteSheet(LoadTexture("ui.png"), {
		{ "ui", SpriteSequence({
			SpriteSequenceFrame(Edges::Zero(), Rect(vec2(0, 0), vec2(uiFrameSize.x, uiFrameSize.y))),
			SpriteSequenceFrame(Edges::All(7), Rect(vec2(uiFrameSize.x, 0), vec2(uiFrameSize.x * 2.f, uiFrameSize.y))),
		})}
	});

	testBatch.buffer = new VertBuffer(POS_UV_COLOR);
	testBatch.shader = LoadShader("world_vertcolor.vert", "sprite_vertcolor.frag");
	testBatch.shader->EnableUniforms(SHADER_MAIN_TEX);
	testBatch.sheet = &testSheet;
	testBatch.texture = testSheet.texture;
}

using namespace gui;

void Update(float dt)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	gui::Text("fps: " + std::to_string(GetFPS()) + "(" + std::to_string(GetAvgFrameTime()) + "ms)");
		gui::vars.margin = Edges::All(25);
		gui::vars.size = vec2(0);
	gui::EndNode();
}

void Draw()
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	//Draw sprites
	spriteBatch.buffer->Clear();

	SetClearColor(vec4(0.5f + cos(GetTime()) * 0.2f, 0.5f, 0.5f + sin(GetTime()) * 0.2f, 1.f));

	for (int i = 0; i < 10; i++)
	{
		Sprite sprite;
		sprite.position = vec3(wrapMinMax((i * 0.4f) + GetTime() * 0.5f, -2.f, 2.f), 0.f, 0.f);
		sprite.size = vec2(0.3);
		sprite.pivot = CENTER;
		sprite.animator = &spriteAnim;
		spriteBatch.PushSprite(sprite);
	}

	spriteBatch.Draw();
}
