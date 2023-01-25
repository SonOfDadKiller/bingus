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

SpriteBatch spriteBatch;
SpriteSheet spriteSheet;
SpriteAnimator spriteAnim;

SpriteBatch testBatch;
SpriteSheet testSheet;

void Start()
{
	BindInputAction(KEY_ESCAPE, HOLD, [](float dt)
	{
		ExitGame();
	});

	//Set up sprite batch
	spriteSheet = SpriteSheet("spritesheet.png", { { "run", SpriteSequence(vec2(0), vec2(128, 128), 4, 0.f) } });
	spriteAnim = SpriteAnimator(&spriteSheet, "run", 10.f);

	spriteBatch = SpriteBatch(VertBuffer({ VERTEX_POS, VERTEX_UV, VERTEX_COLOR }),
		Shader("world_vertcolor.vert", "sprite_vertcolor.frag", SHADER_MAIN_TEX),
		&spriteSheet);

	vec2 uiFrameSize = vec2(128);
	testSheet = SpriteSheet("ui.png", { 
		{ "ui", SpriteSequence({ 
			SpriteSequenceFrame(Edges::None(), Rect(vec2(0, 0), vec2(uiFrameSize.x, uiFrameSize.y))),
			SpriteSequenceFrame(Edges::All(7), Rect(vec2(uiFrameSize.x, 0), vec2(uiFrameSize.x * 2.f, uiFrameSize.y))),
		})}
	});

	testBatch = SpriteBatch(VertBuffer({ VERTEX_POS, VERTEX_UV, VERTEX_COLOR }),
		Shader("world_vertcolor.vert", "sprite_vertcolor.frag", SHADER_MAIN_TEX),
		&testSheet);
}

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
	spriteBatch.Clear();

	for (int i = 0; i < 10; i++)
	{
		vec2 position = vec2(wrapMinMax((i * 0.4f) + GetTime() * 0.5f, -2.f, 2.f), 0.f);
		spriteBatch.PushSprite(Sprite(position, vec2(0.3), CENTER, 0.f, Edges::None(), vec4(1), &spriteAnim));
	}

	spriteBatch.Draw();
}
