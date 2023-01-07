#include "bingus.h"

#include <stack>

static SpriteSheet spriteSheet;
static SpriteBatch spriteBatch;
static TextBatch textBatch;

static GUIWidget canvas;

static GUIMouseEvent mouseEvent;

static std::vector<GUIWidget> widgetStack;

void InitializeGUI()
{
	spriteSheet = SpriteSheet("ui.png");
	spriteSheet.sequences["ui"] = SpriteSequence(vec2(0), vec2(128, 128), 4, 0.f);

	spriteBatch = SpriteBatch(VertBuffer({ VERTEX_POS, VERTEX_UV, VERTEX_COLOR }),
		Shader("ui_vertcolor.vert", "sprite_vertcolor.frag", SHADER_MAIN_TEX),
		&spriteSheet);

	textBatch = TextBatch(VertBuffer({ VERTEX_POS, VERTEX_UV, VERTEX_COLOR }),
		Shader("ui_vertcolor.vert", "text_vertcolor.frag", SHADER_MAIN_TEX),
		Fonts::arial);

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

	spriteBatch.Clear();
	textBatch.Clear();
}

void DrawGUI()
{
	//TODO: This is wrong! I should figure out depth and ordering better - perhaps the solution to this
	//actually belongs in the rendering API
	spriteBatch.Draw();
	textBatch.Draw();
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
	spriteBatch.PushSprite(Sprite(vec3(position, 0.f), size, BOTTOM_LEFT, 0.f, color, &spriteSheet.sequences["ui"], frame));
}

void GUIText(vec2 position, vec2 size, vec2 pivot, vec2 anchor, std::string text, float fontSize, Font* font, vec4 color, vec2 alignment)
{
	CalculateLocalRect(position, size, pivot, anchor);
	NormalizeRect(position, size);
	textBatch.PushText(Text(text, vec3(position, 0.f), size, vec2(1440) / GetWindowSize(), alignment, fontSize, color, font));
}

bool GUIButton(vec2 position, vec2 size, vec2 pivot, vec2 anchor, InputState eventState, vec4 color)
{
	CalculateLocalRect(position, size, pivot, anchor);

	//Determine input state before the rect is normalized for sprite rendering
	bool pressed = mouseEvent.state == eventState
		&& mouseEvent.position.x > position.x && mouseEvent.position.x < position.x + size.x
		&& mouseEvent.position.y > position.y && mouseEvent.position.y < position.y + size.y;

	NormalizeRect(position, size);

	Sprite sprite = Sprite(vec3(position, 0.f), size, BOTTOM_LEFT, 0.f, color, &spriteSheet.sequences["ui"], 1);
	sprite.nineSliceMargin = 0.05f;
	sprite.nineSliceSample = 7;

	spriteBatch.PushSprite(sprite);
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
	spriteBatch.PushSprite(Sprite(vec3(position, 0.f), size, BOTTOM_LEFT, 0.f, color, &spriteSheet.sequences["ui"], state ? 7 : 1));

	return pressed ? !state : state;
}

void GUISetSpritesheet(SpriteSheet* sheet)
{
	//TODO: Need to implement this
}



