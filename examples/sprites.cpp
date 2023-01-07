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
	std::string text = "fps: " + std::to_string(GetFPS()) + "(" + std::to_string(GetAvgFrameTime()) + "ms)";
	GUIText(vec2(22, -22), vec2(500.f, 50.f), TOP_LEFT, TOP_LEFT, text, 0.5f, Fonts::arial, vec4(1.f), TOP_LEFT);
}

void Draw()
{
	spriteBatch.Clear();

	for (int i = 0; i < 10; i++)
	{
		vec3 position = vec3(wrapMinMax((i * 0.4f) + GetTime() * 0.5f, -2.f, 2.f), -0.15f, 0.f);
		spriteBatch.PushSprite(Sprite(position, vec2(0.3), &spriteAnim));
	}
	
	spriteBatch.Draw();

	//9 slice test
	testBatch.Clear();
	
	Sprite sprite = Sprite(vec3(-0.7f, -0.85f, 0), vec2(1.f) + vec2(cos(GetTime()), sin(GetTime())) * 0.8f, &testSheet.sequences["ui"], 1);
	//sprite.nineSliceMargin = 0.02f;
	//sprite.nineSliceSample = 7;
	testBatch.PushSprite(sprite);

	testBatch.Draw();
}

