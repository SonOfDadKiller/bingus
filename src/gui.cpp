#include "bingus.h"

#include <stack>

static AutoBatcher batcher;
static GUIWidget canvas;
static GUIMouseEvent mouseEvent;

static std::vector<GUIWidget*> widgetStack;

static SpriteSequence* spriteSequence;

SpriteSheet defaultGuiSpritesheet;
SpriteSequence* defaultGuiSpriteSequence;
Font* defaultGuiFont;

#define NULL_FLOAT -std::numeric_limits<float>::max()
#define NULL_VEC2 vec2(NULL_FLOAT)

void InitializeGUI()
{
	vec2 uiFrameSize = vec2(128);
	defaultGuiSpritesheet = SpriteSheet("ui.png", {
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

	defaultGuiSpriteSequence = &defaultGuiSpritesheet.sequences["ui"];
	defaultGuiFont = Fonts::arial;

	batcher.vertexAttributes = { VERTEX_POS, VERTEX_UV, VERTEX_COLOR };
	batcher.spriteShader = Shader("ui_vertcolor.vert", "sprite_vertcolor.frag", SHADER_MAIN_TEX);
	batcher.spriteSheet = &defaultGuiSpritesheet;
	batcher.textShader = Shader("ui_vertcolor.vert", "text_vertcolor.frag", SHADER_MAIN_TEX);
	batcher.font = defaultGuiFont;

	spriteSequence = defaultGuiSpriteSequence;

	canvas.vars.pos = vec2(0);
	canvas.vars.size = GetWindowSize();
	canvas.vars.pivot = TOP_LEFT;
	canvas.vars.anchor = TOP_LEFT;

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
	canvas.vars.size = size;
}

void FreeWidget(GUIWidget* widget, bool childrenOnly = false)
{
	for (int i = 1; i < widget->children.size(); i++)
	{
		FreeWidget(widget->children[i]);
	}

	if (!childrenOnly)
	{
		delete widget;
	}
}

void BeginGUI()
{
	if (mouseEvent.state == RELEASE)
	{
		mouseEvent.position = mousePosition;
	}

	//Delete widgets from stack
	FreeWidget(&canvas, true);
	widgetStack.clear();
	canvas.children.clear();

	//Add canvas to stack
	widgetStack.push_back(&canvas);

	batcher.Clear();
}

void NormalizeRect(vec2& position, vec2& size)
{
	position = (position / (GetWindowSize() / 2.f)) - vec2(1);
	size = size / (GetWindowSize() / 2.f);
}

void NormalizeEdges(Edges& edges)
{
	edges.top /= GetWindowSize().y;
	edges.right /= GetWindowSize().x;
	edges.bottom /= GetWindowSize().y;
	edges.left /= GetWindowSize().x;
}

void NormalizeLayout(LayoutType type, float& spacing)
{
	if (type == HORIZONTAL) spacing /= GetWindowSize().x;
	if (type == VERTICAL) spacing /= GetWindowSize().y;
}

void GUIWidget::Build()
{
	vars.pos = vec2(vars.pos.x, -vars.pos.y); //Invert y, so that origin is in the top-left

	//Calculate locals
	//TODO: Need to tell if a property has been set (replace all comparisons with 0.f below)
	if (vars.margin.left != 0.f || vars.margin.right != 0.f)
	{
		if (vars.size.x == 0.f) vars.size.x = parent->vars.size.x - vars.margin.right - vars.margin.left;
		if (vars.margin.left != 0.f) vars.pos.x = vars.margin.left - vars.anchor.x * (vars.size.x + vars.margin.left * 2.f);
		vars.pos.x += vars.pivot.x * vars.size.x;
	}

	if (vars.margin.bottom != 0.f || vars.margin.top != 0.f)
	{
		if (vars.size.y == 0.f) vars.size.y = parent->vars.size.y - vars.margin.top - vars.margin.bottom;
		if (vars.margin.bottom != 0.f) vars.pos.y = vars.margin.bottom - vars.anchor.y * (vars.size.y + vars.margin.bottom * 2.f);
		vars.pos.y += vars.pivot.y * vars.size.y;
	}

	if (parent->vars.layoutType == HORIZONTAL)
	{
		vars.pos.x += parent->layoutOffset;
		parent->layoutOffset += vars.size.x + parent->vars.spacing;
	}
	else if (parent->vars.layoutType == VERTICAL)
	{
		vars.pos.y -= parent->layoutOffset;
		parent->layoutOffset += vars.size.y + parent->vars.spacing;
	}

	//Position the widget relative to its parent
	vec2 worldSpaceAnchor = parent->vars.pos + vars.anchor * parent->vars.size;
	vars.pos = worldSpaceAnchor + vars.pos - (vars.size * vars.pivot);

	vec2 _position = vars.pos;
	vec2 _size = vars.size;

	//Side effects
	switch (type)
	{
	case IMAGE:
		
		NormalizeRect(_position, _size);
		NormalizeEdges(vars.nineSliceMargin);
		batcher.PushSprite(Sprite(vec3(_position, 0.f), _size, BOTTOM_LEFT, 0.f, vars.nineSliceMargin, vars.color, spriteSequence, (u32)vars.source));
		break;
	case TEXT:
		NormalizeRect(_position, _size);
		batcher.PushText(Text(vars.text, vec3(_position, 0.f), _size, vec2(1440) / GetWindowSize(), vars.textAlignment, vars.fontSize, vars.color, vars.font));

		break;
	}

	for (auto it = children.begin(); it != children.end(); it++)
	{
		(*it)->Build();
	}
}

void DrawGUI()
{
	//Build GUI
	for (GUIWidget* widget : canvas.children)
	{
		widget->Build();
	}

	batcher.Draw();
}

GUIWidgetVars::GUIWidgetVars()
{
	pos = vec2(0);
	size = vec2(100);
	pivot = TOP_LEFT;
	anchor = TOP_LEFT;
	margin = Edges::None();

	source = BLOCK;
	color = vec4(1);
	nineSliceMargin = Edges::All(15);

	layoutType = NONE;
	spacing = 0.f;
	stretch = false;

	text = "";
	textAlignment = TOP_LEFT;
	fontSize = 0.5f;
	font = Fonts::arial;
}

namespace gui
{
	GUIWidgetVars vars;

	GUIWidget* Widget()
	{
		//Apply vars to current widget if it is not the canvas
		if (widgetStack.size() > 1)
		{
			widgetStack.back()->vars = vars;
		}

		//Push new widget onto stack, setting parent and child accordingly
		GUIWidget* widget = new GUIWidget();
		widget->type = WIDGET;
		widgetStack.back()->children.push_back(widget);
		widget->parent = widgetStack.back();
		widgetStack.push_back(widget);

		vars = GUIWidgetVars();

		return widget;
	}

	GUIWidget* Image(GUIImageSource _source)
	{
		GUIWidget* widget = Widget();
		widget->type = IMAGE;

		vars.source = _source;

		return widget;
	}

	GUIWidget* Layout(LayoutType type)
	{
		GUIWidget* widget = Widget();
		widget->type = LAYOUT;

		vars.layoutType = type;

		return widget;
	}

	GUIWidget* Text(std::string _text)
	{
		GUIWidget* widget = Widget();
		widget->type = TEXT;

		vars.text = _text;

		return widget;
	}

	void EndNode()
	{
		//Ensure we do not pop the canvas from the stack
		assert(widgetStack.size() > 1);

		//Copy vars into top widget, then pop it from the stack
		widgetStack.back()->vars = vars;
		widgetStack.pop_back();
		vars = widgetStack.back()->vars;
	}
}

//void GUIWidgetBegin(vec2 position, vec2 size, vec2 pivot, vec2 anchor, Edges padding, LayoutType layoutType, float layoutSpacing, bool layoutStretch)
//{
//	//There should always be a widget in the stack. If the user has not defined one, the first widget will be the canvas.
//	assert(!widgetStack.empty()); 
//	CalculateLocalRect(position, size, pivot, anchor);
//	//NormalizeLayout(layoutType, layoutSpacing);
//	widgetStack.push_back({ position, size, pivot, anchor, padding, layoutType, layoutSpacing, layoutStretch });
//}
//
//void GUIWidgetEnd()
//{
//	widgetStack.pop_back();
//	assert(!widgetStack.empty());
//}
//
//void GUIImage(vec2 position, vec2 size, vec2 pivot, vec2 anchor, vec4 color, u32 frame, Edges nineSliceMargin)
//{
//	CalculateLocalRect(position, size, pivot, anchor);
//	NormalizeRect(position, size);
//	NormalizeEdges(nineSliceMargin);
//	batcher.PushSprite(Sprite(vec3(position, 0.f), size, BOTTOM_LEFT, 0.f, nineSliceMargin, color, spriteSequence, frame));
//}
//
//void GUIText(vec2 position, vec2 size, vec2 pivot, vec2 anchor, std::string text, float fontSize, Font* font, vec4 color, vec2 alignment)
//{
//	CalculateLocalRect(position, size, pivot, anchor);
//	NormalizeRect(position, size);
//	batcher.PushText(Text(text, vec3(position, 0.f), size, vec2(1440) / GetWindowSize(), alignment, fontSize, color, font));
//}
//
//bool GUIButton(vec2 position, vec2 size, vec2 pivot, vec2 anchor, InputState eventState, vec4 color)
//{
//	CalculateLocalRect(position, size, pivot, anchor);
//
//	//Determine input state before the rect is normalized for sprite rendering
//	bool pressed = mouseEvent.state == eventState
//		&& mouseEvent.position.x > position.x && mouseEvent.position.x < position.x + size.x
//		&& mouseEvent.position.y > position.y && mouseEvent.position.y < position.y + size.y;
//
//	NormalizeRect(position, size);
//
//	Sprite sprite = Sprite(vec3(position, 0.f), size, BOTTOM_LEFT, 0.f, color, spriteSequence, 1);
//	vec2 margin = vec2(20) / GetWindowSize();
//	sprite.nineSliceMargin = Edges(margin.y, margin.x, margin.y, margin.x);
//
//	batcher.PushSprite(sprite);
//	return pressed;
//}
//
//bool GUITickbox(vec2 position, vec2 size, vec2 pivot, vec2 anchor, vec4 color, bool state)
//{
//	CalculateLocalRect(position, size, pivot, anchor);
//
//	//Determine input state before the rect is normalized for sprite rendering
//	bool pressed = mouseEvent.state == PRESS
//		&& mouseEvent.position.x > position.x && mouseEvent.position.x < position.x + size.x
//		&& mouseEvent.position.y > position.y && mouseEvent.position.y < position.y + size.y;
//
//	NormalizeRect(position, size);
//	batcher.PushSprite(Sprite(vec3(position, 0.f), size, BOTTOM_LEFT, 0.f, color, spriteSequence, state ? 7 : 1));
//
//	return pressed ? !state : state;
//}
//
//void GUISetSpritesheet(SpriteSheet* sheet)
//{
//	batcher.spriteSheet = sheet;
//}
//
//void GUISetFont(Font* font)
//{
//	batcher.font = font;
//}
//
//void GUISetSpriteSequence(SpriteSequence* sequence)
//{
//	spriteSequence = sequence;
//}
