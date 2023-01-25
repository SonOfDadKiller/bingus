#include "bingus.h"

#include <stack>

static AutoBatcher batcher;
static GUIWidget canvas;
static GUIMouseEvent mouseEvent;

static std::vector<GUIWidget> widgets;
static std::vector<u32> widgetStack;

static SpriteSequence* spriteSequence;

static i32 guiDepth;

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

void BeginGUI()
{
	if (mouseEvent.state == RELEASE)
	{
		mouseEvent.position = mousePosition;
	}

	canvas.children.clear();
	widgets.clear();
	widgets.push_back(canvas);

	//Delete widgets from stack
	widgetStack.clear();

	//Add canvas to stack
	widgetStack.push_back(0);

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
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	vars.pos = vec2(vars.pos.x, -vars.pos.y); //Invert y, so that origin is in the top-left

	GUIWidget* parent = &widgets[parentID];

	//Copy parent rect, so we can modify locally
	vec2 parentPos = parent->vars.pos;
	vec2 parentSize = parent->vars.size;

	//React to parent layout if set
	if (parent->vars.layoutType == HORIZONTAL)
	{
		parentPos.x += parentSize.x - vars.size.x - parent->layoutOffset;
		parentSize.x = vars.size.x;
		vars.pos.x = 0.f;
		vars.anchor.x = 1.f;
		vars.pivot.x = 1.f;
		parent->layoutOffset += vars.size.x + parent->vars.spacing;
	}
	else if (parent->vars.layoutType == VERTICAL)
	{
		parentPos.y += parentSize.y - vars.size.y - parent->layoutOffset;
		parentSize.y = vars.size.y;
		vars.pos.y = 0.f;
		vars.anchor.y = 1.f;
		vars.pivot.y = 1.f;
		parent->layoutOffset += vars.size.y + parent->vars.spacing;
	}

	//React to margins
	//TODO: Need to tell if a property has been set (replace all comparisons with 0.f below)
	if (vars.margin.left != 0.f || vars.margin.right != 0.f)
	{
		if (vars.size.x == 0.f) vars.size.x = parentSize.x - vars.margin.right - vars.margin.left;
		if (vars.margin.left != 0.f) vars.pos.x = vars.margin.left - vars.anchor.x * (vars.size.x + vars.margin.left * 2.f);
		vars.pos.x += vars.pivot.x * vars.size.x;
	}

	if (vars.margin.bottom != 0.f || vars.margin.top != 0.f)
	{
		if (vars.size.y == 0.f) vars.size.y = parentSize.y - vars.margin.top - vars.margin.bottom;
		if (vars.margin.bottom != 0.f) vars.pos.y = vars.margin.bottom - vars.anchor.y * (vars.size.y + vars.margin.bottom * 2.f);
		vars.pos.y += vars.pivot.y * vars.size.y;
	}

	//Move into parent-space based on pivot and anchor
	vec2 worldSpaceAnchor = parentPos + vars.anchor * parentSize;
	vars.pos = worldSpaceAnchor + vars.pos - (vars.size * vars.pivot);
	
	vec2 _position = vars.pos;
	vec2 _size = vars.size;

	//Side effects
	switch (type)
	{
	case IMAGE:
		
		NormalizeRect(_position, _size);
		NormalizeEdges(vars.nineSliceMargin);
		batcher.PushSprite(Sprite(vec3(_position, 0.f), _size, BOTTOM_LEFT, 0.f, vars.nineSliceMargin, vars.color, spriteSequence, (u32)vars.source, guiDepth));
		break;
	case TEXT:
		NormalizeRect(_position, _size);
		batcher.PushText(Text(vars.text, vec3(_position, 0.f), _size, vec2(1440) / GetWindowSize(), vars.textAlignment, vars.fontSize, vars.color, vars.font, 10));
		break;
	case BUTTON:
		{
			//Determine input state
			bool mouseInRect = mouseEvent.position.x > _position.x && mouseEvent.position.x < _position.x + vars.size.x
				&& mouseEvent.position.y > _position.y && mouseEvent.position.y < _position.y + vars.size.y;
			bool eventFired = mouseEvent.state == vars.eventState;

			*vars.state = mouseInRect && eventFired;

			NormalizeRect(_position, _size);
			NormalizeEdges(vars.nineSliceMargin);
			vec4 _color = vars.color - (mouseInRect ? (eventFired ? vec4(0.2, 0.2, 0.2, 0.0) : vec4(0.1, 0.1, 0.1, 0.0)) : vec4(0));
			batcher.PushSprite(Sprite(vec3(_position, 0.f), _size, BOTTOM_LEFT, 0.f, vars.nineSliceMargin, _color, spriteSequence, 1, guiDepth));
		}
		break;
	case TICKBOX:
		{
			bool mouseInRect = mouseEvent.position.x > _position.x && mouseEvent.position.x < _position.x + vars.size.x
				&& mouseEvent.position.y > _position.y && mouseEvent.position.y < _position.y + vars.size.y;
			bool eventFired = mouseEvent.state == PRESS;

			*vars.state = eventFired && mouseInRect ? !(*vars.state) : *vars.state;

			NormalizeRect(_position, _size);
			NormalizeEdges(vars.nineSliceMargin);
			vec4 _color = vars.color - (mouseInRect ? (eventFired ? vec4(0.2, 0.2, 0.2, 0.0) : vec4(0.1, 0.1, 0.1, 0.0)) : vec4(0));
			batcher.PushSprite(Sprite(vec3(_position, 0.f), _size, BOTTOM_LEFT, 0.f, vars.nineSliceMargin, _color, spriteSequence, 1, guiDepth));
			if (*vars.state) batcher.PushSprite(Sprite(vec3(_position, 0.f), _size, BOTTOM_LEFT, 0.f, vars.nineSliceMargin, vars.color, spriteSequence, 7, guiDepth));
		}
		break;
	case MASK:

		break;
	}

	guiDepth++;

	for (auto it = children.begin(); it != children.end(); it++)
	{
		widgets[*it].Build();
	}

	guiDepth--;

	if (type == MASK)
	{
		//Reverse stencil settings
	}
}

void DrawGUI()
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	guiDepth = 0;

	//Build GUI
	for (auto it = widgets[0].children.begin(); it != widgets[0].children.end(); it++)
	{
		widgets[*it].Build();
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

	state = nullptr;
	eventState = PRESS;
}

namespace gui
{
	GUIWidgetVars vars;

	u32 Widget()
	{
#ifdef TRACY_ENABLE
		ZoneScoped;
#endif
		//Apply vars to top-stack widget if it is not the canvas
		if (widgetStack.size() > 1)
		{
			
			widgets[widgetStack.back()].vars = vars;
		}

		//Push new widget onto stack, setting parent and child accordingly
		u32 widgetID = widgets.size();
		
		widgets[widgetStack.back()].children.push_back(widgetID);
		widgets.push_back(GUIWidget());
		widgets[widgetID].type = WIDGET;
		widgets[widgetID].parentID = widgetStack.back();
		widgetStack.push_back(widgetID);

		vars = GUIWidgetVars();

		return widgetID;
	}

	u32 Image(GUIImageSource _source)
	{
#ifdef TRACY_ENABLE
		ZoneScoped;
#endif
		u32 widgetID = Widget();
		widgets[widgetID].type = IMAGE;

		vars.source = _source;

		return widgetID;
	}

	u32 Layout(LayoutType type)
	{
#ifdef TRACY_ENABLE
		ZoneScoped;
#endif
		u32 widgetID = Widget();
		widgets[widgetID].type = LAYOUT;

		vars.layoutType = type;

		return widgetID;
	}

	u32 Text(std::string _text)
	{
#ifdef TRACY_ENABLE
		ZoneScoped;
#endif
		u32 widgetID = Widget();
		widgets[widgetID].type = TEXT;

		vars.text = _text;

		return widgetID;
	}

	u32 Button(bool* state, InputState eventState)
	{
#ifdef TRACY_ENABLE
		ZoneScoped;
#endif
		u32 widgetID = Widget();
		widgets[widgetID].type = BUTTON;

		assert(state != nullptr);
		vars.state = state;
		vars.eventState = eventState;

		return widgetID;
	}

	u32 Tickbox(bool* state)
	{
#ifdef TRACY_ENABLE
		ZoneScoped;
#endif
		u32 widgetID = Widget();
		widgets[widgetID].type = TICKBOX;

		assert(state != nullptr);
		vars.state = state;

		return widgetID;
	}

	u32 Mask()
	{
#ifdef TRACY_ENABLE
		ZoneScoped;
#endif
		u32 widgetID = Widget();
		widgets[widgetID].type = MASK;

		return widgetID;
	}

	void EndNode()
	{
		//Ensure we do not pop the canvas from the stack
		assert(widgetStack.size() > 1);

		//Copy vars into top widget, then pop it from the stack
		widgets[widgetStack.back()].vars = vars;
		widgetStack.pop_back();
		vars = widgets[widgetStack.back()].vars;
	}

	GUIWidget& GetWidget(u32 id)
	{
		return widgets[id];
	}
}
