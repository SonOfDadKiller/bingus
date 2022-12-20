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

TextBatch dynamicBatch;
TextBatch staticBatch;

void Start()
{
	fpsCounter = new UIText(vec2(22, -22), vec2(500.f, 50.f), TOP_LEFT, TOP_LEFT);
	fpsCounter->fontSize = 0.5f;

	BindInputAction(KEY_ESCAPE, HOLD, [](float dt)
	{
		ExitGame();
	});

	//Set up batches
	dynamicBatch = TextBatch(VertBuffer({ VERTEX_POS, VERTEX_UV, VERTEX_COLOR }),
		Shader("world_vertcolor.vert", "text_vertcolor.frag", SHADER_MAIN_TEX),
		Fonts::linuxLibertine);

	staticBatch = TextBatch(VertBuffer({ VERTEX_POS, VERTEX_UV, VERTEX_COLOR }),
		Shader("world_vertcolor.vert", "text_vertcolor.frag", SHADER_MAIN_TEX),
		Fonts::arial);

	for (i32 i = 0; i < 10; i++)
	{
		float y = -0.9f + (float)i * 0.195f;
		float x = i % 2 == 0 ? -3.f : -2.8f;
		staticBatch.PushText(Text("this text is static this text is static this text is static this text is static",
			vec3(x, -0.03f + y, 0.f), vec2(6, 0.2f), vec2(0.5), BOTTOM_CENTER, 4.f, vec4(0.2, 0.2, 0.2, 1.0), Fonts::arial));
	}
}

void Update(float dt)
{
	fpsCounter->data = "fps: " + std::to_string(GetFPS()) + "(" + std::to_string(GetAvgFrameTime() * 1000.f) + "ms)";
}

void Draw()
{
	dynamicBatch.Clear();

	vec4 color = vec4(0.7f + sin(GetTime() * 0.5f) * 0.3f, 0.7f + cos(GetTime() * 1.f) * 0.3f, 0.7f + cos(GetTime() * 2.f) * 0.3f, 1.f);
	dynamicBatch.PushText(Text("this text is dynamic", vec3(-1.f + sin(GetTime() * 0.5f) * 0.8, sin(GetTime()) * 0.6f, 0), vec2(2, 0), vec2(0.6), CENTER, 4.f, color, Fonts::linuxLibertine));

	staticBatch.Draw();
	dynamicBatch.Draw();
}
