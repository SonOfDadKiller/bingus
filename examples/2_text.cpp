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

	dynamicBatch.Clear();

	vec4 color = vec4(0.7f + sin(GetTime() * 0.5f) * 0.3f, 0.7f + cos(GetTime() * 1.f) * 0.3f, 0.7f + cos(GetTime() * 2.f) * 0.3f, 1.f);

	vec2 textPos = vec2(-1.f + sin(GetTime() * 0.5f) * 0.8, sin(GetTime()) * 0.6f);
	vec2 textExtents = vec2(1, 0.5f);

	dynamicBatch.PushText(Text("this text is dynamic",
		vec3(textPos, 0),					//position
		textExtents,						//extents
		vec2(1.f),							//scale
		CENTER,								//alignment
		0.5f,								//text size
		color,								//color
		Fonts::linuxLibertine));			//font

	DrawDebugAABB(DEBUG_WORLD, AABB(textPos, textPos + textExtents), vec4(1.f), false);

	staticBatch.Draw();
	dynamicBatch.Draw();
}
