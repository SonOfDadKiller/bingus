#include "bingus.h"

static SpriteSheet spriteSheet;
static SpriteBatch spriteBatch;
static TextBatch textBatch;

UINode* canvas;

static UIMouseEvent mouseEvent;

void InitializeUI()
{
	spriteSheet = SpriteSheet("ui.png");
	spriteSheet.sequences["run"] = SpriteSequence(vec2(0), vec2(128, 128), 4, 0.f);

	spriteBatch = SpriteBatch(VertBuffer({ VERTEX_POS, VERTEX_UV, VERTEX_COLOR }),
		Shader("ui_vertcolor.vert", "sprite_vertcolor.frag", SHADER_MAIN_TEX),
		&spriteSheet);

	textBatch = TextBatch(VertBuffer({ VERTEX_POS, VERTEX_UV, VERTEX_COLOR }),
		Shader("ui_vertcolor.vert", "text_vertcolor.frag", SHADER_MAIN_TEX),
		Fonts::arial);

	canvas = new UINode(nullptr, vec2(0), GetWindowSize());

	//TODO: Move input bindings out of here?
	BindInputAction(MOUSE_LEFT, PRESS, [](float dt) {
		mouseEvent.state = PRESS;
		mouseEvent.position = mousePosition;
	});

	BindInputAction(MOUSE_LEFT, RELEASE, [](float dt) {
		mouseEvent.state = RELEASE;
	});

	mouseEvent.state = RELEASE;
}

void SetUICanvasSize(vec2 size)
{
	canvas->size = size;
	canvas->dirty = true;
	//std::cout << canvas->size.x << "\n";
}

void UpdateUI()
{
	if (mouseEvent.state == RELEASE)
	{
		mouseEvent.position = mousePosition;
	}

	canvas->Step(vec2(0), canvas->size, mouseEvent);
}

void DrawUI()
{
	spriteBatch.Clear();
	textBatch.Clear();

	canvas->Draw();

	//TODO: Figure out depth and ordering
	spriteBatch.Draw();
	textBatch.Draw();
}

UINode::UINode(UINode* parent, vec2 position, vec2 size, vec2 pivot, vec2 anchor)
{
	this->position = position;
	this->screenPosition = vec2(0);
	this->size = size;
	this->pivot = pivot;
	this->anchor = anchor;
	this->parent = parent;
	this->dirty = true;

	if (parent)
	{
		parent->children.push_back(this);
	}
}

void UINode::NormalizeRect(vec2& nPos, vec2& nSize)
{
	nPos = (screenPosition / (GetWindowSize() / 2.f)) - vec2(1);
	nSize = size / (GetWindowSize() / 2.f);
}

void UINode::Step(vec2 parentPosition, vec2 parentSize, UIMouseEvent _mouseEvent)
{
	//TODO: only calculate if node is dirty
	//if (dirty)
	{
		//Calculate the global position for this node
		//(Local position does not change!)
		vec2 worldAnchor = parentPosition + anchor * parentSize;
		screenPosition = worldAnchor + position - (size * pivot);

		for (auto it = children.begin(); it != children.end(); it++)
		{
			(*it)->Step(screenPosition, size, _mouseEvent);
		}

		dirty = false;
	}
}

void UINode::Draw()
{
	for (auto it = children.begin(); it != children.end(); it++)
	{
		(*it)->Draw();
	}
}

UIImage::UIImage(UINode* parent, vec2 position, vec2 size, vec2 pivot, vec2 anchor) : UINode(parent, position, size, pivot, anchor)
{
	sequence = nullptr;
	frame = 0;
	color = vec4(1);
}

void UIImage::Step(vec2 parentPosition, vec2 parentSize, UIMouseEvent _mouseEvent)
{
	UINode::Step(parentPosition, parentSize, _mouseEvent);
}

void UIImage::Draw()
{
	UINode::Draw();
	vec2 nPos, nSize;
	NormalizeRect(nPos, nSize);
	spriteBatch.PushSprite(Sprite(vec3(nPos, 0.f), nSize, BOTTOM_LEFT, 0.f, color, sequence, frame));
}

UIText::UIText(UINode* parent, vec2 position, vec2 size, vec2 pivot, vec2 anchor) : UINode(parent, position, size, pivot, anchor)
{
	fontSize = 1.f;
	color = vec4(1);
	font = nullptr;
	data = "TextTextText";
	alignment = pivot;
}

void UIText::Step(vec2 parentPosition, vec2 parentSize, UIMouseEvent _mouseEvent)
{
	UINode::Step(parentPosition, parentSize, _mouseEvent);
}

void UIText::Draw()
{
	UINode::Draw();
	vec2 nPos, nSize;
	NormalizeRect(nPos, nSize);
	//Font* _font = font == nullptr ? Fonts::arial : font;
	//Text* text = new ;
	textBatch.PushText(Text(data, vec3(nPos, 0.f), nSize, vec2(1440) / GetWindowSize(), alignment, fontSize, color, Fonts::arial));

	//Outline sprite
	/*Sprite* sprite = new Sprite(vec3(nPos, 0.f), nSize, 0, 0);
	sprite->color = vec4(1, 1, 1, 0.2);
	sprites.push_back(sprite);*/
}

UIButton::UIButton(UINode* parent, vec2 position, vec2 size, vec2 pivot, vec2 anchor) : UINode(parent, position, size, pivot, anchor)
{
	state = RELEASED;
	onPress = nullptr;
	sequence = nullptr;
	frame = 0;
	color = vec4(1.f);
}

void UIButton::Step(vec2 parentPosition, vec2 parentSize, UIMouseEvent _mouseEvent)
{
	UINode::Step(parentPosition, parentSize, _mouseEvent);

	//Determine mouse collision
	if (_mouseEvent.position.x > screenPosition.x && _mouseEvent.position.x < screenPosition.x + size.x
		&& _mouseEvent.position.y > screenPosition.y && _mouseEvent.position.y < screenPosition.y + size.y)
	{
		if (_mouseEvent.state == PRESS)
		{
			if (state == PRESSED)
			{
				state = HELD;
			}
			else if (state != HELD)
			{
				state = PRESSED;
			}
		}
		else
		{
			state = HOVER;
		}
	}
	else
	{
		state = RELEASED;
	}

	switch (state)
	{
	case RELEASED:
		break;
	case HOVER:
		if (onHover != nullptr) onHover();
		break;
	case PRESSED:
		if (onPress != nullptr) onPress();
		break;
	case HELD:
		if (onHold != nullptr) onHold();
		break;
	}
}

void UIButton::Draw()
{
	UINode::Draw();
	vec4 buttonColor = color;

	switch (state)
	{
	case HOVER:
		buttonColor += vec4(0.1f, 0.1f, 0.1f, 0.f);
		break;
	case PRESSED:
		buttonColor -= vec4(0.1f, 0.1f, 0.1f, 0.f);
		break;
	case HELD:
		buttonColor -= vec4(0.1f, 0.1f, 0.1f, 0.f);
		break;
	case RELEASED: break;
	}

	//Push button sprite
	vec2 nPos, nSize;
	NormalizeRect(nPos, nSize);
	Sprite sprite = Sprite(vec3(nPos, 0.f), nSize, sequence, frame);
	sprite.color = buttonColor;
	spriteBatch.PushSprite(sprite);
}