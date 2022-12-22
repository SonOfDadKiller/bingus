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

SpriteBatch spriteBatch;
SpriteSheet spriteSheet;

void Start()
{
	fpsCounter = new UIText(vec2(22, -22), vec2(500.f, 50.f), TOP_LEFT, TOP_LEFT);
	fpsCounter->fontSize = 0.5f;

	BindInputAction(KEY_ESCAPE, HOLD, [](float dt)
	{
		ExitGame();
	});

	//Set up sprite batch
	spriteSheet = SpriteSheet("spritesheet.png");
	spriteSheet.sequences["run"] = SpriteSequence(vec2(0), vec2(128, 128), 4, 0.f);

	spriteBatch = SpriteBatch(VertBuffer({ VERTEX_POS, VERTEX_UV, VERTEX_COLOR }),
		Shader("world_vertcolor.vert", "sprite_vertcolor.frag", SHADER_MAIN_TEX),
		&spriteSheet);
}

void Update(float dt)
{
	fpsCounter->data = "fps: " + std::to_string(GetFPS()) + "(" + std::to_string(GetAvgFrameTime() * 1000.f) + "ms)";
}

void Draw()
{
	spriteBatch.Clear();

	for (int i = 0; i < 10; i++)
	{
		vec3 position = vec3(wrapMinMax((i * 0.4f) + GetTime() * 0.5f, -2.f, 2.f), -0.15f, 0.f);
		spriteBatch.PushSprite(Sprite(position, vec2(0.3), &spriteSheet.sequences["run"], (u32)(GetTime() * 4.f) % 4));
	}

	spriteBatch.Draw();
}

