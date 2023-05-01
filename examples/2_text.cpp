#include "bingus.h"

void Start();
void Update(float dt);
void Draw();

int main()
{
	SetupWindow(1280, 720, "Text");
	BingusInit();

	SetGameStartFunction(Start);
	SetGameUpdateFunction(Update);
	SetGameDrawFunction(Draw);
	
	RunGame();

	return 0;
}

TextBatch dynamicBatch;
TextBatch staticBatch;

void Start()
{
	globalInputListener.BindAction(KEY_ESCAPE, HOLD, []()
	{
		ExitGame();
	});

	//Set up batches

	dynamicBatch.buffer = new VertBuffer(POS_UV_COLOR);
	dynamicBatch.shader = LoadShader("world_vertcolor.vert", "text_vertcolor.frag");
	dynamicBatch.font = LoadFont("linux_libertine.ttf", 80);
	dynamicBatch.texture = &dynamicBatch.font->texture;

	staticBatch.buffer = new VertBuffer(POS_UV_COLOR);
	staticBatch.shader = LoadShader("world_vertcolor.vert", "text_vertcolor.frag");
	staticBatch.font = LoadFont("arial.ttf", 80);
	staticBatch.texture = &staticBatch.font->texture;

	for (i32 i = 0; i < 10; i++)
	{
		float y = -0.9f + (float)i * 0.195f;
		float x = i % 2 == 0 ? -3.f : -2.8f;

		Text text;
		text.data = "this text is static this text is static this text is static this text is static";
		text.position = vec3(x, -0.03f + y, 0.f);
		text.extents = vec2(6, 0.2f);
		text.alignment = BOTTOM_CENTER;
		text.textSize = 0.2f;
		text.color = vec4(0.2, 0.2, 0.2, 1.0);
		text.font = LoadFont("arial.ttf", 80);
		staticBatch.PushText(text);
	}
}

void Update(float dt)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	gui::Text("fps: " + std::to_string(GetFPS()) + "(" + std::to_string(GetAvgFrameTime()) + "ms)");
		gui::vars.margin = Edges::All(25);
		gui::vars.size = vec2(0);
		gui::vars.textHeightInPixels = 35.f;
	gui::EndNode();
}

void Draw()
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	dynamicBatch.buffer->Clear();

	vec4 color = vec4(0.7f + sin(GetTime() * 0.5f) * 0.3f,
					  0.7f + cos(GetTime() * 1.f) * 0.3f,
					  0.7f + cos(GetTime() * 2.f) * 0.3f,
					  1.f);

	Text text;
	text.data = "This text is dynamic!";
	text.position = vec3(-1.f + sin(GetTime() * 0.5f) * 0.8, sin(GetTime()) * 0.6f, 0.f);
	text.extents = vec2(2, 0.f);
	text.alignment = CENTER;
	text.textSize = 0.2f;
	text.color = color;
	text.font = LoadFont("linux_libertine.ttf", 80);
	dynamicBatch.PushText(text);

	staticBatch.Draw();
	dynamicBatch.Draw();
}
