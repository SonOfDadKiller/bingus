#include "bingus.h"

#include <stack>

static AutoBatcher batcher;
static SpriteSheet spriteSheet;
static SpriteSequence* spriteSequence;

static GUIWidget canvas;

static GUIMouseEvent mouseEvent;

static std::vector<GUIWidget> widgetStack;

void InitializeGUI()
{
	vec2 uiFrameSize = vec2(128);
	spriteSheet = SpriteSheet("ui.png", {
		{ "ui", SpriteSequence({
			SpriteSequenceFrame(Edges::None(), Rect(vec2(0, 0), vec2(uiFrameSize.x, uiFrameSize.y))),
			SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x, 0), vec2(uiFrameSize.x * 2.f, uiFrameSize.y))),
			SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 2.f, 0), vec2(uiFrameSize.x * 3.f, uiFrameSize.y))),
			SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 3.f, 0), vec2(uiFrameSize.x * 4.f, uiFrameSize.y))),
			SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 4.f, 0), vec2(uiFrameSize.x * 5.f, uiFrameSize.y))),
			SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 5.f, 0), vec2(uiFrameSize.x * 6.f, uiFrameSize.y))),
			SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 6.f, 0), vec2(uiFrameSize.x * 7.f, uiFrameSize.y))),
			SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 7.f, 0), vec2(uiFrameSize.x * 8.f, uiFrameSize.y))),
			SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 8.f, 0), vec2(uiFrameSize.x * 9.f, uiFrameSize.y))),
			SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 9.f, 0), vec2(uiFrameSize.x * 10.f, uiFrameSize.y))),
		})}
	});

	spriteSequence = &spriteSheet.sequences["ui"];

	batcher.vertexAttributes = { VERTEX_POS, VERTEX_UV, VERTEX_COLOR };
	batcher.spriteShader = Shader("ui_vertcolor.vert", "sprite_vertcolor.frag", SHADER_MAIN_TEX);
	batcher.spriteSheet = &spriteSheet;
	batcher.textShader = Shader("ui_vertcolor.vert", "text_vertcolor.frag", SHADER_MAIN_TEX);
	batcher.font = Fonts::arial;

	canvas.position = vec2(0);
	canvas.size = GetWindowSize();

	//TODO: Move input bindings out of here?
	BindInputAction(MOUSE_LEFT, PRESS, [](float dt) {
		mouseEvent.state = PRESS;
		mouseEvent.position = mousePosition;
	});

	BindInputAction(MOUSE_LEFT, HOLD, [](float dt) {
		mouseEvent.state = HOLD;
	});

	BindInputAction(MOUSE_LEFT, RELEASE, [](float dt) {
		mouseEvent.state = RELEASE;
	});

	mouseEvent.state = RELEASE;
}

void SetGUICanvasSize(vec2 size)
{
	canvas.size = size;
}

void BeginGUI()
{
	if (mouseEvent.state == RELEASE)
	{
		mouseEvent.position = mousePosition;
	}

	widgetStack.clear();
	widgetStack.push_back(canvas);

	batcher.Clear();
}

void DrawGUI()
{
	//TODO: This is wrong! I should figure out depth and ordering better - perhaps the solution to this
	//actually belongs in the rendering API
	batcher.Draw();
}

void NormalizeRect(vec2& position, vec2& size)
{
	position = (position / (GetWindowSize() / 2.f)) - vec2(1);
	size = size / (GetWindowSize() / 2.f);
}

void CalculateLocalRect(vec2& position, vec2 size, vec2 pivot, vec2 anchor)
{
	//Position the widget relative to its parent
	vec2 worldSpaceAnchor = widgetStack.back().position + anchor * widgetStack.back().size;
	position = worldSpaceAnchor + position - (size * pivot);
}

void GUIWidgetBegin(vec2 position, vec2 size, vec2 pivot, vec2 anchor)
{
	//There should always be a widget in the stack. If the user has not defined one, the first widget will be the canvas.
	assert(!widgetStack.empty()); 
	CalculateLocalRect(position, size, pivot, anchor);
	widgetStack.push_back({ position, size, pivot, anchor });
}

void GUIWidgetEnd()
{
	widgetStack.pop_back();
	assert(!widgetStack.empty());
}

void GUIImage(vec2 position, vec2 size, vec2 pivot, vec2 anchor, vec4 color, u32 frame)
{
	CalculateLocalRect(position, size, pivot, anchor);
	NormalizeRect(position, size);
	batcher.PushSprite(Sprite(vec3(position, 0.f), size, BOTTOM_LEFT, 0.f, color, spriteSequence, frame));
}

void GUIText(vec2 position, vec2 size, vec2 pivot, vec2 anchor, std::string text, float fontSize, Font* font, vec4 color, vec2 alignment)
{
	CalculateLocalRect(position, size, pivot, anchor);
	NormalizeRect(position, size);
	batcher.PushText(Text(text, vec3(position, 0.f), size, vec2(1440) / GetWindowSize(), alignment, fontSize, color, font));
}

bool GUIButton(vec2 position, vec2 size, vec2 pivot, vec2 anchor, InputState eventState, vec4 color)
{
	CalculateLocalRect(position, size, pivot, anchor);

	//Determine input state before the rect is normalized for sprite rendering
	bool pressed = mouseEvent.state == eventState
		&& mouseEvent.position.x > position.x && mouseEvent.position.x < position.x + size.x
		&& mouseEvent.position.y > position.y && mouseEvent.position.y < position.y + size.y;

	NormalizeRect(position, size);

	Sprite sprite = Sprite(vec3(position, 0.f), size, BOTTOM_LEFT, 0.f, color, spriteSequence, 1);
	vec2 margin = vec2(20) / GetWindowSize();
	sprite.nineSliceMargin = Edges(margin.y, margin.x, margin.y, margin.x);

	batcher.PushSprite(sprite);
	return pressed;
}

bool GUITickbox(vec2 position, vec2 size, vec2 pivot, vec2 anchor, vec4 color, bool state)
{
	CalculateLocalRect(position, size, pivot, anchor);

	//Determine input state before the rect is normalized for sprite rendering
	bool pressed = mouseEvent.state == PRESS
		&& mouseEvent.position.x > position.x && mouseEvent.position.x < position.x + size.x
		&& mouseEvent.position.y > position.y && mouseEvent.position.y < position.y + size.y;

	NormalizeRect(position, size);
	batcher.PushSprite(Sprite(vec3(position, 0.f), size, BOTTOM_LEFT, 0.f, color, spriteSequence, state ? 7 : 1));

	return pressed ? !state : state;
}

void GUISetSpritesheet(SpriteSheet* sheet)
{
	//TODO: Need to implement this
}



