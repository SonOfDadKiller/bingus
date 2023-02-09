#include "bingus.h"

#include <stack>

static GUIWidget canvas;

static std::vector<GUIWidget> widgets;
static std::vector<u32> widgetStack;
static std::vector<Rect> masks;
//static u32 highestInputWidget;
static u32 activeInputWidget;

static u32 widgetCount;
static u32 prevWidgetCount;

static SpriteSequence* spriteSequence;
static RenderQueue renderQueue;

static float guiDepth;
static InputState guiMouseState;
static vec2 guiMousePressPosition;


SpriteSheet defaultGuiSpritesheet;
SpriteSequence* defaultGuiSpriteSequence;
Font* defaultGuiFont;



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

	renderQueue.buffer = VertBuffer({ VERTEX_POS, VERTEX_UV, VERTEX_COLOR });
	renderQueue.spriteShader = Shader("ui_vertcolor.vert", "sprite_vertcolor.frag", SHADER_MAIN_TEX);
	renderQueue.textShader = Shader("ui_vertcolor.vert", "text_vertcolor.frag", SHADER_MAIN_TEX);
	renderQueue.spriteSheet = &defaultGuiSpritesheet;
	renderQueue.font = Fonts::arial;

	spriteSequence = defaultGuiSpriteSequence;

	canvas.vars.pos = vec2(0);
	canvas.vars.size = GetWindowSize();
	canvas.vars.pivot = TOP_LEFT;
	canvas.vars.anchor = TOP_LEFT;

	//TODO: Move input bindings out of here?
	BindInputAction(MOUSE_LEFT, PRESS, [](float dt) {
		guiMouseState = PRESS;
		guiMousePressPosition = mousePosition;
	});

	BindInputAction(MOUSE_LEFT, HOLD, [](float dt) {
		guiMouseState = HOLD;
	});

	BindInputAction(MOUSE_LEFT, RELEASE, [](float dt) {
		guiMouseState = RELEASE;
	});

	guiMouseState = RELEASE;
}

void SetGUICanvasSize(vec2 size)
{
	canvas.vars.size = size;
}

void BeginGUI()
{
	canvas.children.clear();
	widgets.clear();
	widgets.push_back(canvas);

	//Delete widgets from stack
	widgetStack.clear();

	//Add canvas to stack
	widgetStack.push_back(0);

	prevWidgetCount = widgetCount;
	widgetCount = 0;
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

bool PointIsHiddenByMask(vec2 position)
{
	bool hidden = false;
	for (int i = 0; i < masks.size(); i++)
	{
		Rect mask = masks[i];
		hidden = hidden || !(position.x > mask.min.x && position.x < mask.max.x
			&& position.y > mask.min.y && position.y < mask.max.y);
	}

	return hidden;
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

	depth = guiDepth;
	guiDepth -= 0.0001f;

	for (auto it = children.begin(); it != children.end(); it++)
	{
		widgets[*it].Build();
	}

	widgetCount++;
}

void GUIWidget::Draw()
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	vec2 _position = vars.pos;
	vec2 _size = vars.size;

	switch (type)
	{
	case IMAGE:
		NormalizeRect(_position, _size);
		NormalizeEdges(vars.nineSliceMargin);

		renderQueue.PushSprite(Sprite(vec3(_position, depth), _size, BOTTOM_LEFT, 0.f, vars.nineSliceMargin, vars.color, spriteSequence, (u32)vars.source));
		break;
	case TEXT:
		NormalizeRect(_position, _size);

		renderQueue.PushText(Text(vars.text, vec3(_position, depth), _size, vec2(1440) / GetWindowSize(), vars.textAlignment, vars.fontSize, vars.color, vars.font));
		break;
	case BUTTON:
	{
		NormalizeRect(_position, _size);
		NormalizeEdges(vars.nineSliceMargin);
		vec4 _color = vars.color - (activeInputWidget == id ? (guiMouseState == HOLD ? vec4(0.2, 0.2, 0.2, 0.0) : vec4(0.1, 0.1, 0.1, 0.0)) : vec4(0));
		renderQueue.PushSprite(Sprite(vec3(_position, depth), _size, BOTTOM_LEFT, 0.f, vars.nineSliceMargin, _color, spriteSequence, 1));
	}
	break;
	case TICKBOX:
	{
		NormalizeRect(_position, _size);
		NormalizeEdges(vars.nineSliceMargin);

		vec4 _color = vars.color;

		if (activeInputWidget == id)
		{
			_color -= vec4(0.1, 0.1, 0.1, 0.1);

			if (guiMouseState == HOLD)
			{
				_color -= vec4(0.1, 0.1, 0.1, 0.1);
			}
		}

		renderQueue.PushSprite(Sprite(vec3(_position, depth), _size, BOTTOM_LEFT, 0.f, vars.nineSliceMargin, _color, spriteSequence, 1));
		if (*vars.state) renderQueue.PushSprite(Sprite(vec3(_position, depth), _size, BOTTOM_LEFT, 0.f, vars.nineSliceMargin, _color, spriteSequence, 7));
	}
	break;
	case MASK:
		NormalizeRect(_position, _size);

		//Create masking render step, and push mask rectangle
		renderQueue.PushStep([]() {
			glEnable(GL_STENCIL_TEST);
			glStencilOp(GL_ZERO, GL_REPLACE, GL_REPLACE);
			glStencilMask(0xFF);
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
		}, nullptr);

		renderQueue.PushSprite(Sprite(vec3(_position, depth), _size, BOTTOM_LEFT, 0.f, Edges::None(), vec4(1, 1, 1, 0), spriteSequence, 0));

		//Create next step, which will be masked
		renderQueue.PushStep([]() {
			glEnable(GL_STENCIL_TEST);
			glStencilOp(GL_ZERO, GL_REPLACE, GL_REPLACE);
			glStencilMask(0x00);
			glStencilFunc(GL_EQUAL, 1, 0xFF);
		}, []() {
			glStencilMask(0xFF);
			glClear(GL_STENCIL_BUFFER_BIT);
			glDisable(GL_STENCIL_TEST);
		});

		break;
	}

	for (auto it = children.begin(); it != children.end(); it++)
	{
		widgets[*it].Draw();
	}

	if (type == MASK)
	{
		//Mask has ended, push new non-masked step
		renderQueue.PushStep(nullptr, nullptr);
	}
}

void GUIWidget::ProcessInput()
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	if (type == MASK)
	{
		masks.push_back(Rect(vars.pos, vars.pos + vars.size));
	}

	for (auto it = children.rbegin(); it != children.rend(); it++)
	{
		widgets[*it].ProcessInput();
	}

	switch (type)
	{
	case BUTTON:
		if (activeInputWidget == 0)
		{
			vars.hoveredState = !PointIsHiddenByMask(mousePosition) && (mousePosition.x > vars.pos.x && mousePosition.x < vars.pos.x + vars.size.x
				&& mousePosition.y > vars.pos.y && mousePosition.y < vars.pos.y + vars.size.y);

			if (vars.hoveredState) 
			{
				activeInputWidget = id;

				//Trigger events if necessary
				if (guiMouseState == PRESS && vars.onPress != nullptr) vars.onPress();
			}
		}
		else
		{
			vars.hoveredState = false;
		}
		
		if (vars.state != nullptr)
		{
			*vars.state = vars.hoveredState && guiMouseState == vars.eventState;
		}
		
		break;
	case TICKBOX:
		if (activeInputWidget == 0)
		{
			vars.hoveredState = !PointIsHiddenByMask(mousePosition) && (mousePosition.x > vars.pos.x && mousePosition.x < vars.pos.x + vars.size.x
				&& mousePosition.y > vars.pos.y && mousePosition.y < vars.pos.y + vars.size.y);

			if (vars.hoveredState) activeInputWidget = id;
		}
		else
		{
			vars.hoveredState = false;
		}

		*vars.state = guiMouseState == PRESS && vars.hoveredState ? !(*vars.state) : *vars.state;
		break;
	case MASK:
		masks.pop_back();
		break;
	}
}

void BuildGUI()
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	guiDepth = 1.f;

	for (auto it = widgets[0].children.begin(); it != widgets[0].children.end(); it++)
	{
		widgets[*it].Build();
	}
}

void ProcessGUIInput()
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	
	if (guiMouseState == HOLD)
	{
		if (widgetCount != prevWidgetCount)
		{
			activeInputWidget += widgetCount - prevWidgetCount;
		}

		if (activeInputWidget > widgets.size() + 1)
		{
			activeInputWidget = 0;
		}

		GUIWidget* inputWidget = &widgets[activeInputWidget];
		switch (inputWidget->type)
		{
		case BUTTON:
			if (inputWidget->vars.state != nullptr)
			{
				*inputWidget->vars.state = guiMouseState == inputWidget->vars.eventState;
			}

			if (inputWidget->vars.onHold != nullptr) inputWidget->vars.onHold();
			
			break;
		}

		return;
	}

	activeInputWidget = 0;

	for (auto it = widgets[0].children.rbegin(); it != widgets[0].children.rend(); it++)
	{
		widgets[*it].ProcessInput();
	}
}

void DrawGUI()
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	renderQueue.Clear();
	for (auto it = widgets[0].children.begin(); it != widgets[0].children.end(); it++)
	{
		widgets[*it].Draw();
	}
	renderQueue.Draw();
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
	hoveredState = false;
	eventState = PRESS;
	onPress = nullptr;
	onHold = nullptr;
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
		widgets[widgetID].id = widgetID;
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

	u32 Button()
	{
#ifdef TRACY_ENABLE
		ZoneScoped;
#endif
		u32 widgetID = Widget();
		widgets[widgetID].type = BUTTON;

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

	u32 Window(GUIWindow* window)
	{
#ifdef TRACY_ENABLE
		ZoneScoped;
#endif
		u32 widgetID = Widget();
			vars.pos = window->pos;
			vars.size = window->size;

			Button(&window->grabbed, HOLD);
				vars.margin = Edges::All(0.01f);
				vars.size = vec2(0);
			EndNode();

			Button(&window->grabbedLeft, HOLD);
				vars.margin = Edges(20, window->size.x - 20, 20, 0);
				vars.size = vec2(0);
			EndNode();

			Button(&window->grabbedTop, HOLD);
				vars.pivot = vars.anchor = TOP_CENTER;
				vars.size = vec2(window->size.x - 40, 20);
			EndNode();

			Button(&window->grabbedRight, HOLD);
				vars.margin = Edges(20, 0, 20, window->size.x - 20);
				vars.size = vec2(0);
			EndNode();

			Button(&window->grabbedBottom, HOLD);
				vars.pivot = vars.anchor = BOTTOM_CENTER;
				vars.size = vec2(window->size.x - 40, 20);
			EndNode();

			Button(&window->grabbedTopLeft, HOLD);
				vars.size = vec2(20);
				vars.pivot = vars.anchor = TOP_LEFT;
			EndNode();

			Button(&window->grabbedTopRight, HOLD);
				vars.size = vec2(20);
				vars.pivot = vars.anchor = TOP_RIGHT;
			EndNode();

			Button(&window->grabbedBottomLeft, HOLD);
				vars.size = vec2(20);
				vars.pivot = vars.anchor = BOTTOM_LEFT;
			EndNode();

			Button(&window->grabbedBottomRight, HOLD);
				vars.size = vec2(20);
				vars.pivot = vars.anchor = BOTTOM_RIGHT;
			EndNode();

			//Window dragging
			if (window->grabbedLeft || window->grabbedTop || window->grabbedRight || window->grabbedBottom || 
				window->grabbedBottomLeft || window->grabbedTopLeft || window->grabbedBottomRight || window->grabbedTopRight)
			{
				if (window->grabSize == NULL_VEC2)
				{
					window->grabSize = window->size;
					window->grabPos = window->pos;
				}
			}
			else
			{
				window->grabSize = NULL_VEC2;
			}

			vec2 mouseMove = guiMousePressPosition - mousePosition;

			if (window->grabbed)
			{
				window->pos += vec2(-mouseDelta.x, mouseDelta.y);
			}
			
			if (window->grabbedLeft || window->grabbedBottomLeft || window->grabbedTopLeft)
			{
				float minPos = window->grabPos.x + window->grabSize.x - window->maxSize.x;
				float maxPos = window->grabPos.x + window->grabSize.x - window->minSize.x;
				window->pos.x = glm::clamp(window->grabPos.x - mouseMove.x, minPos, maxPos);
				window->size.x = window->grabSize.x + mouseMove.x;
			}

			if (window->grabbedTop || window->grabbedTopLeft || window->grabbedTopRight)
			{
				float minPos = window->grabPos.y + window->grabSize.y - window->maxSize.y;
				float maxPos = window->grabPos.y + window->grabSize.y - window->minSize.y;
				window->pos.y = glm::clamp(window->grabPos.y + mouseMove.y, minPos, maxPos);
				window->size.y = window->grabSize.y - mouseMove.y;
			}

			if (window->grabbedRight || window->grabbedBottomRight || window->grabbedTopRight)
			{
				window->size.x = window->grabSize.x - mouseMove.x;
			}

			if (window->grabbedBottom || window->grabbedBottomLeft || window->grabbedBottomRight)
			{
				window->size.y = window->grabSize.y + mouseMove.y;
			}

			window->size = glm::clamp(window->size, window->minSize, window->maxSize);



			Image(BOX);
				vars.margin = Edges::All(0.001);
				vars.size = vec2(0);
			EndNode();

		//We leave out the node terminator, so that the caller may include the window contents before terminating
		//EndNode();

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
