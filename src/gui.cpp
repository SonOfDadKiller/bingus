#include "bingus.h"

#include <sstream>
#include <iomanip>

GUIContext globalGUIContext;

static RenderQueue renderQueue;

static std::unordered_map<u64, GUIWidget> widgetPool;
static std::unordered_map<u64, GUIImage> imagePool;
static std::unordered_map<u64, GUILabel> labelPool;
static std::unordered_map<u64, GUIButton> buttonPool;
static std::unordered_map<u64, GUITickbox> tickboxPool;
static std::unordered_map<u64, GUISlider> sliderPool;
static std::unordered_map<u64, GUITextField> textFieldPool;
static std::unordered_map<u64, GUIFloatField> floatFieldPool;
static std::unordered_map<u64, GUIRow> rowPool;
static std::unordered_map<u64, GUIColumn> columnPool;

static std::vector<u64> widgetStack;
static u64 hotWidget;
static u64 activeWidget;
static u64 canvasID = 1;
static u64 nullWidgetID = 0;

static std::vector<u64> buildWidgets;
static std::vector<u64> renderWidgets;

static InputState guiMouseState;
static bool shiftHeld;
static vec2 guiMousePressPosition;

static InputListener inputListener;

static u32 textSelectEnd; 
static u32 textSelectStart;
static bool selectingText;
static float textFieldInputTime;

static bool dragging;

static bool initialized = false;

void ProcessEnterKey();

void GUIContext::Start()
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	if (!initialized)
	{
		spriteTexture = LoadTexture("ui.png");
		spriteTexture->SetFilterMode(GL_LINEAR);
		spriteTexture->SetWrapMode(GL_CLAMP_TO_EDGE);
		vec2 uiFrameSize = vec2(128);
		spriteSheet = SpriteSheet(spriteTexture, {
			{ "ui", SpriteSequence({
				SpriteSequenceFrame(Edges::Zero(), Rect(vec2(0, 0), vec2(uiFrameSize.x, uiFrameSize.y))),
				SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x, 0), vec2(uiFrameSize.x * 2.f, uiFrameSize.y))),
				SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 2.f, 0), vec2(uiFrameSize.x * 3.f, uiFrameSize.y))),
				SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 3.f, 0), vec2(uiFrameSize.x * 4.f, uiFrameSize.y))),
				SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 4.f, 0), vec2(uiFrameSize.x * 5.f, uiFrameSize.y))),
				SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 5.f, 0), vec2(uiFrameSize.x * 6.f, uiFrameSize.y))),
				SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 6.f, 0), vec2(uiFrameSize.x * 7.f, uiFrameSize.y))),
				SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 7.f, 0), vec2(uiFrameSize.x * 8.f, uiFrameSize.y))),
				SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 8.f, 0), vec2(uiFrameSize.x * 9.f, uiFrameSize.y))),
				SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 9.f, 0), vec2(uiFrameSize.x * 10.f, uiFrameSize.y))),
				SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 10.f, 0), vec2(uiFrameSize.x * 11.f, uiFrameSize.y))),
				SpriteSequenceFrame(Edges::All(8), Rect(vec2(uiFrameSize.x * 11.f, 0), vec2(uiFrameSize.x * 12.f, uiFrameSize.y))),
			})}
		});
		spriteSequence = &spriteSheet.sequences["ui"];

		defaultFont = LoadFont("inter_semibald.ttf", 26);

		renderQueue.spriteShader = LoadShader("ui_vertcolor.vert", "sprite_vertcolor.frag");
		renderQueue.spriteShader->EnableUniforms(SHADER_MAIN_TEX);
		renderQueue.textShader = LoadShader("ui_vertcolor.vert", "text_vertcolor.frag");
		renderQueue.textShader->EnableUniforms(SHADER_MAIN_TEX);
		renderQueue.spriteSheet = &spriteSheet;
		renderQueue.font = defaultFont;

		//TODO: Move input bindings out of here?
		inputListener.BindAction(MOUSE_LEFT, PRESS, []() {
			guiMouseState = PRESS;
			guiMousePressPosition = mousePosition;

			// 		if (!mouseAboveGUI)
			// 		{
			// 			interactWidget = 0;
			// 		}
		});

		inputListener.BindAction(MOUSE_LEFT, HOLD, []() {
			guiMouseState = HOLD;
		});

		inputListener.BindAction(MOUSE_LEFT, RELEASE, []() {
			guiMouseState = RELEASE;
		});

		inputListener.BindAction(MOUSE_LEFT, UP, []() {
			guiMouseState = UP;
		});

		inputListener.BindAction(KEY_SHIFT_LEFT, PRESS, []() {
			shiftHeld = true;
		});

		inputListener.BindAction(KEY_SHIFT_LEFT, RELEASE, []() {
			shiftHeld = false;
		});

		inputListener.BindAction(KEY_SHIFT_RIGHT, PRESS, []() {
			shiftHeld = true;
		});

		inputListener.BindAction(KEY_SHIFT_RIGHT, RELEASE, []() {
			shiftHeld = false;
		});

		inputListener.BindAction(KEY_ENTER, PRESS, ProcessEnterKey);

		inputListener.priority = 1;

		RegisterInputListener(&inputListener);

		selectingText = false;
		dragging = false;

		initialized = true;
	}

	guiDepth = 1.f;

	widgetStack.clear();
	buildWidgets.clear();
	renderWidgets.clear();

	GUIWidget canvas = GUIWidget();
	canvas.size = GetWindowSize();
	canvas.pivot = TOP_LEFT;
	canvas.anchor = TOP_LEFT;
	canvas.id = canvasID;
	canvas.dirty = false;
	widgetPool[canvasID] = canvas;
	widgetStack.push_back(canvasID);

	defaultImage = GUIImage();
	defaultLabel = GUILabel();
	defaultButton = GUIButton();
	defaultTickbox = GUITickbox();
	defaultSlider = GUISlider();
	defaultTextField = GUITextField();
	defaultFloatField = GUIFloatField();
	defaultRow = GUIRow();
	defaultColumn = GUIColumn();
}

void ProcessTextInput(u32 codepoint)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	//Get pointer to string being edited
	std::string* inputString;
	GUIWidget* widget = &widgetPool[activeWidget];
	if (widget->componentType == GUI_TEXT_FIELD)
	{
		inputString = textFieldPool[activeWidget].value;
	}
	else if (widget->componentType == GUI_FLOAT_FIELD)
	{
		inputString = &floatFieldPool[activeWidget].text;
	}
	
	if (codepoint == 5) //Left arrow (ENQ)
	{
		if (textSelectEnd != 0) textSelectEnd--;
		if (!shiftHeld) textSelectStart = textSelectEnd;
	}
	else if (codepoint == 6) //Right arrow (ACK)
	{
		if (textSelectEnd != inputString->size()) textSelectEnd++;
		if (!shiftHeld) textSelectStart = textSelectEnd;
	}
	else if (codepoint == 8) //Backspace
	{
		if (textSelectStart != textSelectEnd)
		{
			//Deleting with multi-select
			u32 _selectStart = std::min(textSelectStart, textSelectEnd);
			u32 _selectEnd = std::max(textSelectStart, textSelectEnd);
			inputString->erase(_selectStart, _selectEnd - _selectStart);

			textSelectEnd = _selectStart;
			textSelectStart = _selectStart;
		}
		else if (textSelectEnd != 0)
		{
			inputString->erase(textSelectEnd - 1, 1);
			textSelectEnd--;
		}
		textSelectStart = textSelectEnd;
	}
	else
	{
		//Normal text character
		if (textSelectStart != textSelectEnd)
		{
			//Deleting with multi-select
			u32 _selectStart = std::min(textSelectStart, textSelectEnd);
			u32 _selectEnd = std::max(textSelectStart, textSelectEnd);
			inputString->erase(_selectStart, _selectEnd - _selectStart);

			textSelectEnd = _selectStart;
			textSelectStart = _selectStart;
		}

		inputString->insert(inputString->begin() + textSelectEnd, codepoint);
		textSelectEnd++;
		textSelectStart = textSelectEnd;
	}

	if (widget->componentType == GUI_FLOAT_FIELD)
	{
		GUIFloatField* floatField = &floatFieldPool[activeWidget];
		float oldVal = *floatField->value;
		*floatField->value = glm::clamp(ParseFloat(floatField->text), floatField->min, floatField->max);
		if (floatField->onValueChanged != nullptr && oldVal != *floatField->value) floatField->onValueChanged(*floatField->value);
	}

	textFieldInputTime = GetTime();
}

void ProcessEnterKey()
{
	if (activeWidget != 0)
	{
		if (widgetPool[activeWidget].componentType == GUI_FLOAT_FIELD)
		{
			GUIFloatField* floatField = &floatFieldPool[activeWidget];
			if (!dragging)
			{
				float oldVal = *floatField->value;
				*floatField->value = glm::clamp(ParseFloat(floatField->text), floatField->min, floatField->max);
				if (floatField->onValueChanged != nullptr && oldVal != *floatField->value) floatField->onValueChanged(*floatField->value);
				selectingText = false;
				textSelectStart = 0;
				textSelectEnd = 0;
				activeWidget = 0;
			}
		}
	}
}

u32 GetTextCharacterIndexAtPosition(GUIWidget* widget, const TextRenderInfo& textInfo, float textHeightInPixels, vec2 position)
{
	//Clamp position to text area
	position = vec2(std::clamp(position.x, widget->pos.x, widget->pos.x + widget->size.x), 
		std::clamp(position.y, widget->pos.y, widget->pos.y + widget->size.y));

// 	DrawDebugCircle(DEBUG_SCREEN, Circle(position, 5.f), 12, vec4(1), false);
// 	DrawDebugCircle(DEBUG_SCREEN, Circle(mousePosition, 5.f), 12, vec4(1, 0, 0, 1), false);
// 	DrawDebugText(DEBUG_SCREEN, mousePosition, 30.f, vec4(1, 0, 0, 1), std::to_string(mousePosition.x) + ", " + std::to_string(mousePosition.y));
// 	DrawDebugCircle(DEBUG_SCREEN, Circle(widget->pos, 5.f), 12, vec4(1, 1, 0, 1), false);
// 	DrawDebugCircle(DEBUG_SCREEN, Circle(widget->pos + widget->size, 5.f), 12, vec4(1, 0, 1, 1), false);
// 	DrawDebugAABB(DEBUG_SCREEN, AABB(widget->pos, widget->pos + widget->size), vec4(0, 0, 1, 1), false);

	u32 lineEndCharacterIndex = std::max((int)textInfo.caretPositionOffsets.size() - 1, 0);
	u32 line = 0;

	if (!textInfo.lineEndCharacters.empty())
	{
		float diff = (widget->pos.y + widget->size.y - position.y);
		// DrawDebugLine(DEBUG_SCREEN, widget->pos, vec2(widget->pos.x, widget->pos.y + diff), 2.f, vec4(0, 1, 0, 1));

		float lineFloat = diff / textHeightInPixels;
		float floor = std::floor(lineFloat);

		line = std::max((int)floor, 0);
		if (line < textInfo.lineEndCharacters.size())
		{
			lineEndCharacterIndex = textInfo.lineEndCharacters[line];
		}
	}

	for (int i = lineEndCharacterIndex; i != 0; i--)
	{
		vec2 offset = textInfo.caretPositionOffsets[i];
		offset = ((offset) / 2.f) * GetWindowSize();
		vec2 charPos = widget->pos + widget->size + vec2(4.f, -4.f) + offset;

		if (position.x > widget->pos.x + offset.x)
		{
			if (i == lineEndCharacterIndex)
			{
				if (line < textInfo.lineEndCharacters.size())
				{
					vec2 lineEndOffset = textInfo.lineEndOffsets[line];
					lineEndOffset = ((lineEndOffset) / 2.f) * GetWindowSize();
					if (position.x > widget->pos.x + 4.f + lineEndOffset.x)
					{
						i++;
					}
				}

				return std::clamp(i, 0, (int)textInfo.caretPositionOffsets.size() - 1);
			}
			return i;
		}
	}

	return 0;
}

//Convert position and size from pixel coordinates to normalized screen coordinates
void NormalizeRect(vec2& position, vec2& size)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	position.x = std::round(position.x);
	position.y = std::round(position.y);
	size.x = std::round(size.x);
	size.y = std::round(size.y);

	position = (position / (GetWindowSize() / 2.f)) - vec2(1);
	size = size / (GetWindowSize() / 2.f);
}

void NormalizeEdges(Edges& edges)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	edges.top /= GetWindowSize().y / 2.f;
	edges.right /= GetWindowSize().x / 2.f;
	edges.bottom /= GetWindowSize().y / 2.f;
	edges.left /= GetWindowSize().x / 2.f;
}

void GUIContext::EndAndDraw()
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	u64 oldHotWidget = hotWidget;
	bool foundHotWidget = false;
	inputListener.onCharacterTyped = nullptr;

	//Build
	for (u64 id : buildWidgets)
	{
		GUIWidget* widget = &widgetPool[id];
		if (widget->dirty) BuildWidget(widget->id);

		//Calculate hot and active widgets
		if (!foundHotWidget && widget->receiveInput)
		{
			//Calculate hit
			if (mousePosition.x > widget->pos.x && mousePosition.x < widget->pos.x + widget->size.x
				&& mousePosition.y > widget->pos.y && mousePosition.y < widget->pos.y + widget->size.y)
			{
				hotWidget = widget->id;
				foundHotWidget = true;

				if ((activeWidget == 0
					|| widgetPool[activeWidget].componentType == GUI_TEXT_FIELD
					|| widgetPool[activeWidget].componentType == GUI_FLOAT_FIELD)
					&& guiMouseState == PRESS)
				{
					activeWidget = widget->id;

					
				}
			}
		}

		//Do some extra processing
		if (id != activeWidget && widgetPool[id].componentType == GUI_FLOAT_FIELD)
		{
			//Set float field text to value
			GUIFloatField* floatField = &floatFieldPool[id];
			std::stringstream stream;
			stream << std::fixed << std::setprecision(2) << *floatField->value;
			floatField->text = stream.str();
		}
	}

	mouseOverGUI = foundHotWidget;

	if (!foundHotWidget) hotWidget = 0;

	//Process hot and active input events
	if (hotWidget != oldHotWidget)
	{
		if (oldHotWidget != 0)
		{
			//Trigger hover exit
			GUIWidget* oldWidget = &widgetPool[oldHotWidget];
			if (oldWidget->componentType == GUI_BUTTON)
			{
				GUIButton* button = &buttonPool[oldHotWidget];
				if (button->onHoverExit != nullptr) button->onHoverExit();
			}
		}

		//Trigger hover enter
		GUIWidget* widget = &widgetPool[hotWidget];
		if (widget->componentType == GUI_BUTTON)
		{
			GUIButton* button = &buttonPool[hotWidget];
			if (button->onHoverEnter != nullptr) button->onHoverEnter();
		}
	}
	else if (hotWidget != 0)
	{
		//Trigger hover
		GUIWidget* widget = &widgetPool[hotWidget];
		if (widget->componentType == GUI_BUTTON)
		{
			GUIButton* button = &buttonPool[hotWidget];
			if (button->onHover != nullptr) button->onHover();
		}
	}

	if (guiMouseState == RELEASE)
	{
		dragging = false;

		if (activeWidget != 0)
		{
			GUIWidget* widget = &widgetPool[activeWidget];

			//Trigger release
			if (widget->componentType == GUI_BUTTON)
			{
				GUIButton* button = &buttonPool[widget->id];
				if (button->onRelease != nullptr) button->onRelease();
			}

			if (widget->componentType == GUI_TEXT_FIELD)
			{
				//Keep text field active if we are selecting text
				GUITextField* textField = &textFieldPool[activeWidget];
				if (activeWidget != hotWidget)
				{
					if (!selectingText)
					{
						activeWidget = 0;
					}
				}
				selectingText = false;
			}
			else if (widget->componentType == GUI_FLOAT_FIELD)
			{
				if (hotWidget == activeWidget)
				{
					//Select whole text within float field
					GUIFloatField* floatField = &floatFieldPool[activeWidget];
					textFieldInputTime = GetTime();
					textSelectStart = glm::max((int)floatField->text.size(), 0);
					textSelectEnd = 0;
					selectingText = true;
				}
				else
				{
					textSelectStart = 0;
					textSelectEnd = 0;
					activeWidget = 0;
					selectingText = false;
				}
			}
			else
			{
				activeWidget = 0;
			}
		}
	}
	else if (activeWidget != 0)
	{
		GUIWidget* widget = &widgetPool[activeWidget];

		if (widget->componentType == GUI_BUTTON)
		{
			GUIButton* button = &buttonPool[activeWidget];
			if (guiMouseState == PRESS && button->onPress != nullptr) button->onPress();
			if (guiMouseState == HOLD && button->onHold != nullptr) button->onHold();
		}
		else if (widget->componentType == GUI_TICKBOX)
		{
			GUITickbox* tickbox = &tickboxPool[activeWidget];
			if (guiMouseState == PRESS) *tickbox->value = !(*tickbox->value);
		}
		else if (widget->componentType == GUI_SLIDER)
		{
			if (guiMouseState == PRESS || guiMouseState == HOLD)
			{
				GUISlider* slider = &sliderPool[activeWidget];
				float sliderVal = glm::clamp((mousePosition.x - widget->pos.x) / widget->size.x, 0.f, 1.f);
				*slider->value = glm::mix(slider->min, slider->max, sliderVal);
			}
		}
		else if (widget->componentType == GUI_TEXT_FIELD)
		{
			GUITextField* textField = &textFieldPool[activeWidget];
			inputListener.onCharacterTyped = ProcessTextInput;

			if (guiMouseState == PRESS)
			{
				if (hotWidget == activeWidget)
				{
					textFieldInputTime = GetTime();
					textSelectStart = GetTextCharacterIndexAtPosition(widget, textField->textInfo, textField->textHeightInPixels, mousePosition);
					textSelectEnd = textSelectStart;
				}
			}
			else if (guiMouseState == HOLD)
			{
				if (activeWidget == hotWidget && glm::distance(guiMousePressPosition, mousePosition) > 3.f)
				{
					selectingText = true;
				}

				if (selectingText)
				{
					u32 selectIndex = GetTextCharacterIndexAtPosition(widget, textField->textInfo, textField->textHeightInPixels, mousePosition);
					textSelectEnd = selectIndex;
				}
			}
		}
		else if (widget->componentType == GUI_FLOAT_FIELD)
		{
			GUIFloatField* floatField = &floatFieldPool[activeWidget];
			inputListener.onCharacterTyped = ProcessTextInput;

			if (!selectingText)
			{
				textSelectStart = 0;
				textSelectEnd = 0;
				textFieldInputTime = GetTime() + 0.5f;
			}

			if (guiMouseState == PRESS)
			{
				if (selectingText)
				{
					textSelectStart = glm::max((int)floatField->text.size(), 0);
					textSelectEnd = 0;
				}
			}
			else if (guiMouseState == HOLD)
			{
				if (activeWidget == hotWidget && glm::distance(guiMousePressPosition, mousePosition) > 3.f)
				{
					dragging = true;
				}

				if (dragging && !selectingText)
				{
					//Drag value
					float oldVal = *floatField->value;
					*floatField->value += mouseDelta.x / 100.f;
					*floatField->value = glm::clamp(*floatField->value, floatField->min, floatField->max);

					if (floatField->onValueChanged != nullptr && *floatField->value != oldVal)
					{
						floatField->onValueChanged(*floatField->value);
					}

					//Update text
					std::stringstream stream;
					stream << std::fixed << std::setprecision(2) << *floatField->value;
					floatField->text = stream.str();
				}
			}
		}
	}

	//Draw
	renderQueue.Clear();

	for (u64 id : renderWidgets)
	{
		GUIWidget* widget = &widgetPool[id];
		vec2 pos = widget->pos;
		vec2 size = widget->size;

		if (widget->componentType == GUI_IMAGE)
		{
			GUIImage* image = &imagePool[id];
			NormalizeRect(pos, size);
			NormalizeEdges(image->nineSliceMargin);

			Sprite sprite;
			sprite.position = vec3(pos, widget->renderDepth);
			sprite.size = size;
			sprite.pivot = BOTTOM_LEFT;
			sprite.nineSliceMargin = image->nineSliceMargin;
			sprite.color = image->color;
			sprite.sequence = spriteSequence;
			sprite.sequenceFrame = (u32)image->source;
			renderQueue.PushSprite(sprite);
		}
		else if (widget->componentType == GUI_LABEL)
		{
			GUILabel* label = &labelPool[id];
			NormalizeRect(pos, size);

			Text text;
			text.data = label->text;
			text.position = vec3(pos, widget->renderDepth);
			text.extents = size;
			text.scale = vec2(GetWindowSize().y / GetWindowSize().x, 1.f);
			text.alignment = label->textAlignment;
			text.textSize = label->textHeightInPixels / GetWindowSize().y * 2.f;
			text.color = label->color;
			text.font = label->font;
			renderQueue.PushText(text);
		}
		else if (widget->componentType == GUI_BUTTON)
		{
			GUIButton* button = &buttonPool[id];
			NormalizeRect(pos, size);
			NormalizeEdges(button->nineSliceMargin);

			Sprite sprite;
			sprite.position = vec3(pos, widget->renderDepth);
			sprite.size = size;
			sprite.pivot = BOTTOM_LEFT;
			sprite.nineSliceMargin = button->nineSliceMargin;
			sprite.color = id == activeWidget ? button->pressColor : (id == hotWidget ? button->hoverColor : button->color);
			sprite.sequence = spriteSequence;
			sprite.sequenceFrame = BOX;
			renderQueue.PushSprite(sprite);
		}
		else if (widget->componentType == GUI_TICKBOX)
		{
			GUITickbox* tickbox = &tickboxPool[id];
			NormalizeRect(pos, size);
			NormalizeEdges(tickbox->nineSliceMargin);

			Sprite boxSprite;
			boxSprite.position = vec3(pos, widget->renderDepth);
			boxSprite.size = size;
			boxSprite.pivot = BOTTOM_LEFT;
			boxSprite.nineSliceMargin = tickbox->nineSliceMargin;
			boxSprite.color = id == activeWidget ? tickbox->pressColor : (id == hotWidget ? tickbox->hoverColor : tickbox->color);
			boxSprite.sequence = spriteSequence;
			boxSprite.sequenceFrame = BOX;
			renderQueue.PushSprite(boxSprite);

			if (*tickbox->value)
			{
				Sprite crossSprite;
				crossSprite.position = vec3(pos, widget->renderDepth);
				crossSprite.size = size;
				crossSprite.pivot = BOTTOM_LEFT;
				crossSprite.nineSliceMargin = tickbox->nineSliceMargin;
				crossSprite.color = id == activeWidget ? tickbox->pressColor : (id == hotWidget ? tickbox->hoverColor : tickbox->color);
				crossSprite.sequence = spriteSequence;
				crossSprite.sequenceFrame = CROSS;
				renderQueue.PushSprite(crossSprite);
			}
		}
		else if (widget->componentType == GUI_SLIDER)
		{
			GUISlider* slider = &sliderPool[id];

			float sliderVal = (*slider->value - slider->min) / (slider->max - slider->min);
			vec2 linePos = pos + vec2((size.x - 2.f) * sliderVal, 0.f);
			vec2 lineSize = vec2(2.f, size.y);

			vec2 textPos = widget->pos + vec2(4.f, 0.f);
			vec2 textSize = widget->size;

			NormalizeRect(pos, size);
			NormalizeRect(linePos, lineSize);
			NormalizeRect(textPos, textSize);
			NormalizeEdges(slider->nineSliceMargin);
			vec4 _color = id == activeWidget ? slider->pressColor : (id == hotWidget ? slider->hoverColor : slider->color);
			vec4 _textColor = glm::mix(slider->color, vec4(1.f, 1.f, 1.f, slider->color.w), 0.7f);

			//Background
			Sprite sprite;
			sprite.position = vec3(pos, widget->renderDepth);
			sprite.size = size;
			sprite.pivot = BOTTOM_LEFT;
			sprite.nineSliceMargin = slider->nineSliceMargin;
			sprite.color = _color;
			sprite.sequence = spriteSequence;
			sprite.sequenceFrame = 11;
			renderQueue.PushSprite(sprite);

			//Line
			sprite.position = vec3(linePos, widget->renderDepth);
			sprite.size = lineSize;
			sprite.nineSliceMargin = Edges::Zero();
			sprite.color = _color;
			sprite.sequenceFrame = 0;
			renderQueue.PushSprite(sprite);

			//Text
			Text text;
			std::stringstream stream;
			stream << std::fixed << std::setprecision(2) << *slider->value;
			text.data = stream.str();
			text.position = vec3(textPos, widget->renderDepth);
			text.extents = textSize;
			text.scale = vec2(GetWindowSize().y / GetWindowSize().x, 1.f);
			text.alignment = slider->textAlignment;
			text.textSize = slider->textHeightInPixels / GetWindowSize().y * 2.f;
			text.color = _textColor;
			text.font = slider->font;
			renderQueue.PushText(text);
		}
		else if (widget->componentType == GUI_TEXT_FIELD)
		{
			GUITextField* textField = &textFieldPool[id];

			vec2 textPos = pos + vec2(4.f, -4.f);
			vec2 textExtents = size - vec2(8.f, 0.f);
			vec4 textColor = glm::mix(textField->color, vec4(1.f, 1.f, 1.f, textField->color.w), 0.7f);

			NormalizeRect(pos, size);
			NormalizeRect(textPos, textExtents);
			Edges nineSliceMargin = textField->nineSliceMargin;
			NormalizeEdges(nineSliceMargin);

			//Background
			Sprite sprite;
			sprite.position = vec3(pos, widget->renderDepth);
			sprite.size = size;
			sprite.pivot = BOTTOM_LEFT;
			sprite.nineSliceMargin = nineSliceMargin;
			sprite.color = textField->color;
			sprite.sequence = spriteSequence;
			sprite.sequenceFrame = (activeWidget == id || hotWidget == id) ? 10 : 11;
			renderQueue.PushSprite(sprite);

			//Blank text
			if (textField->value->empty() && !textField->text.empty())
			{
				Text text;
				text.data = textField->text;
				text.position = vec3(textPos, widget->renderDepth);
				text.extents = textExtents;
				text.scale = vec2(GetWindowSize().y / GetWindowSize().x, 1.f);
				text.alignment = textField->textAlignment;
				text.textSize = textField->textHeightInPixels / GetWindowSize().y * 2.f;
				text.color = vec4(textColor.x, textColor.y, textColor.z, 0.5);
				text.font = textField->font;
				renderQueue.PushText(text);
			}

			TextRenderInfo info;
			//info.caretPositionOffsets = std::vector<vec2>(1, vec2(0));
			info.lineHeightInPixels = 40.f;
			info.renderScale = vec2(1);

			//Text
			Text text;
			text.data = *textField->value;
			text.position = vec3(textPos, widget->renderDepth);
			text.extents = textExtents;
			text.scale = vec2(GetWindowSize().y / GetWindowSize().x, 1.f);
			text.alignment = textField->textAlignment;
			text.textSize = textField->textHeightInPixels / GetWindowSize().y * 2.f;
			text.color = textColor;
			text.font = textField->font;
			renderQueue.PushText(text, info);

			textField->textInfo = info;

			//Selection
			if (activeWidget == id)
			{
				if (textSelectStart != textSelectEnd)
				{
					//Selection box

					u32 _selectStart = std::min(textSelectStart, textSelectEnd);
					u32 _selectEnd = std::max(textSelectStart, textSelectEnd);

					vec2 lineStartPos = textField->textInfo.caretPositionOffsets[_selectStart];
					u32 line = 0;

					//Find line by iterating through characters
					for (int i = 0; i < _selectStart; i++)
					{
						auto it = std::find(textField->textInfo.lineEndCharacters.begin(), textField->textInfo.lineEndCharacters.end(), i);
						if (it != textField->textInfo.lineEndCharacters.end()) line++;
					}

					for (int i = _selectStart; i != _selectEnd; i++)
					{
						//Check if this character starts a new line
						auto it = std::find(textField->textInfo.lineEndCharacters.begin(), textField->textInfo.lineEndCharacters.end(), i);
						if (it != textField->textInfo.lineEndCharacters.end())
						{
							//Draw this line's selection rect
							float rectWidth = ((textField->textInfo.lineEndOffsets[line].x - lineStartPos.x) / 2.f) * GetWindowSize().x;
							vec2 caretSize = vec2(rectWidth, textField->textHeightInPixels);
							vec2 caretPixelPos = widget->pos + vec2(4.f, 0.f);
							caretPixelPos.y += (widget->size.y - caretSize.y) * textField->textAlignment.y;
							NormalizeRect(caretPixelPos, caretSize);

							Sprite sprite;
							sprite.position = vec3(caretPixelPos + lineStartPos, widget->renderDepth);
							sprite.size = caretSize;
							sprite.pivot = BOTTOM_LEFT;
							sprite.color = vec4(1, 1, 1, 0.3f);
							sprite.sequence = spriteSequence;
							sprite.sequenceFrame = 0;
							renderQueue.PushSprite(sprite);

							if (textField->textInfo.caretPositionOffsets.size() > i - 1)
							{
								lineStartPos = textField->textInfo.caretPositionOffsets[i + 1];
							}
							
							line++;	
						}
					}

					vec2 endPos = textField->textInfo.caretPositionOffsets[_selectEnd];
					float rectWidth = ((endPos.x - lineStartPos.x) / 2.f) * GetWindowSize().x;
					vec2 caretSize = vec2(rectWidth, textField->textHeightInPixels);
					vec2 caretPixelPos = widget->pos + vec2(4.f, 0.f);
					caretPixelPos.y += (widget->size.y - caretSize.y) * textField->textAlignment.y;
					NormalizeRect(caretPixelPos, caretSize);

					Sprite sprite;
					sprite.position = vec3(caretPixelPos + lineStartPos, widget->renderDepth);
					sprite.size = caretSize;
					sprite.pivot = BOTTOM_LEFT;
					sprite.color = vec4(1, 1, 1, 0.3f);
					sprite.sequence = spriteSequence;
					sprite.sequenceFrame = 0;
					renderQueue.PushSprite(sprite);
				}

				//Caret
				if ((int)((GetTime() - textFieldInputTime) * 2.f) % 2 == 0)
				{
					vec2 caretSize = vec2(2.f, textField->textHeightInPixels);
					vec2 caretPixelPos = widget->pos + vec2(4.f, 0.f);
					caretPixelPos.y += (widget->size.y - caretSize.y) * textField->textAlignment.y;
					NormalizeRect(caretPixelPos, caretSize);

					vec2 caretOffset = vec2(0);
					if (info.caretPositionOffsets.size() > 1) caretOffset = info.caretPositionOffsets[textSelectEnd];

					Sprite sprite;
					sprite.position = vec3(caretPixelPos + caretOffset, widget->renderDepth);
					sprite.size = caretSize;
					sprite.pivot = BOTTOM_LEFT;
					sprite.sequence = spriteSequence;
					sprite.sequenceFrame = 0;
					renderQueue.PushSprite(sprite);
				}
			}
		}
		else if (widget->componentType == GUI_FLOAT_FIELD)
		{
			GUIFloatField* floatField = &floatFieldPool[id];

			vec2 textPos = pos + vec2(4.f, 0);
			vec2 textExtents = size - vec2(8.f, 0.f);
			vec4 textColor = glm::mix(floatField->color, vec4(1.f, 1.f, 1.f, floatField->color.w), 0.7f);

			NormalizeRect(pos, size);
			NormalizeRect(textPos, textExtents);
			Edges nineSliceMargin = floatField->nineSliceMargin;
			NormalizeEdges(nineSliceMargin);

			//Background
			Sprite sprite;
			sprite.position = vec3(pos, widget->renderDepth);
			sprite.size = size;
			sprite.pivot = BOTTOM_LEFT;
			sprite.nineSliceMargin = nineSliceMargin;
			sprite.color = floatField->color;
			sprite.sequence = spriteSequence;
			sprite.sequenceFrame = (activeWidget == id || hotWidget == id) ? 10 : 11;
			renderQueue.PushSprite(sprite);

			TextRenderInfo info;
			//info.caretPositionOffsets = std::vector<vec2>(1, vec2(0));
			info.lineHeightInPixels = 40.f;
			info.renderScale = vec2(1);

			//Text
			Text text;
			text.data = floatField->text;
			text.position = vec3(textPos, widget->renderDepth);
			text.extents = textExtents;
			text.scale = vec2(GetWindowSize().y / GetWindowSize().x, 1.f);
			text.alignment = floatField->textAlignment;
			text.textSize = floatField->textHeightInPixels / GetWindowSize().y * 2.f;
			text.color = textColor;
			text.font = floatField->font;
			renderQueue.PushText(text, info);

			floatField->textInfo = info;

			//Selection
			if (activeWidget == id)
			{
				if (textSelectStart != textSelectEnd)
				{
					//Selection box
					u32 _selectStart = std::min(textSelectStart, textSelectEnd);
					u32 _selectEnd = std::max(textSelectStart, textSelectEnd);

					vec2 lineStartPos = floatField->textInfo.caretPositionOffsets[_selectStart];
					u32 line = 0;

					//Find line by iterating through characters
					for (int i = 0; i < _selectStart; i++)
					{
						auto it = std::find(floatField->textInfo.lineEndCharacters.begin(), floatField->textInfo.lineEndCharacters.end(), i);
						if (it != floatField->textInfo.lineEndCharacters.end()) line++;
					}

					for (int i = _selectStart; i != _selectEnd; i++)
					{
						//Check if this character starts a new line
						auto it = std::find(floatField->textInfo.lineEndCharacters.begin(), floatField->textInfo.lineEndCharacters.end(), i);
						if (it != floatField->textInfo.lineEndCharacters.end())
						{
							//Draw this line's selection rect
							float rectWidth = ((floatField->textInfo.lineEndOffsets[line].x - lineStartPos.x) / 2.f) * GetWindowSize().x;
							vec2 caretSize = vec2(rectWidth, floatField->textHeightInPixels);
							vec2 caretPixelPos = widget->pos + vec2(4.f, 0.f);
							caretPixelPos.y += (widget->size.y - caretSize.y) * floatField->textAlignment.y;
							NormalizeRect(caretPixelPos, caretSize);

							Sprite sprite;
							sprite.position = vec3(caretPixelPos + lineStartPos, widget->renderDepth);
							sprite.size = caretSize;
							sprite.pivot = BOTTOM_LEFT;
							sprite.color = vec4(1, 1, 1, 0.3f);
							sprite.sequence = spriteSequence;
							sprite.sequenceFrame = 0;
							renderQueue.PushSprite(sprite);

							if (floatField->textInfo.caretPositionOffsets.size() > i - 1)
							{
								lineStartPos = floatField->textInfo.caretPositionOffsets[i + 1];
							}
							
							line++;	
						}
					}

					vec2 endPos = floatField->textInfo.caretPositionOffsets[_selectEnd];
					float rectWidth = ((endPos.x - lineStartPos.x) / 2.f) * GetWindowSize().x;
					vec2 caretSize = vec2(rectWidth, floatField->textHeightInPixels);
					vec2 caretPixelPos = widget->pos + vec2(4.f, 0.f);
					caretPixelPos.y += (widget->size.y - caretSize.y) * floatField->textAlignment.y;
					NormalizeRect(caretPixelPos, caretSize);

					Sprite sprite;
					sprite.position = vec3(caretPixelPos + lineStartPos, widget->renderDepth);
					sprite.size = caretSize;
					sprite.pivot = BOTTOM_LEFT;
					sprite.color = vec4(1, 1, 1, 0.3f);
					sprite.sequence = spriteSequence;
					sprite.sequenceFrame = 0;
					renderQueue.PushSprite(sprite);
				}

				//Caret
				if ((int)((GetTime() - textFieldInputTime) * 2.f) % 2 == 0)
				{
					vec2 caretSize = vec2(2.f, floatField->textHeightInPixels);
					vec2 caretPixelPos = widget->pos + vec2(4.f, 0.f);
					caretPixelPos.y += (widget->size.y - caretSize.y) * floatField->textAlignment.y;
					NormalizeRect(caretPixelPos, caretSize);

					vec2 caretOffset = vec2(0);
					if (info.caretPositionOffsets.size() > 1) caretOffset = info.caretPositionOffsets[textSelectEnd];

					Sprite sprite;
					sprite.position = vec3(caretPixelPos + caretOffset, widget->renderDepth);
					sprite.size = caretSize;
					sprite.pivot = BOTTOM_LEFT;
					sprite.sequence = spriteSequence;
					sprite.sequenceFrame = 0;
					renderQueue.PushSprite(sprite);
				}
			}
		}
	}

	renderQueue.Draw();
}

void GUIContext::BuildWidget(u64 id)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	GUIWidget* widget = &widgetPool[id];
	GUIWidget* parent = &widgetPool[widget->parentID];

	//Recursively build parents up the tree
	if (parent->dirty) BuildWidget(parent->id);

	widget->pos.y = -widget->pos.y; //Invert y, so that origin is in top-left

	//Copy parent rect, so we can modify locally
	vec2 parentPos = parent->pos;
	vec2 parentSize = parent->size;

	//React to parent layout if set
	if (parent->componentType == GUI_ROW)
	{
		GUIRow* parentRow = &rowPool[parent->id];
		parentPos.x += parentRow->offset;
		parentSize.x = parent->size.x;
		widget->pos.x = 0.f;
		widget->anchor.x = 0.f;
		widget->pivot.x = 0.f;
		parentRow->offset += widget->size.x + parentRow->spacing;
	}
	else if (parent->componentType == GUI_COLUMN)
	{
		GUIColumn* parentColumn = &columnPool[parent->id];
		parentPos.y += parentSize.y - widget->size.y - parentColumn->offset;
		parentSize.y = widget->size.y;
		widget->pos.y = 0.f;
		widget->anchor.y = 1.f;
		widget->pivot.y = 1.f;
		parentColumn->offset += widget->size.y + parentColumn->spacing;
	}

	if (widget->marginLeftSet || widget->marginRightSet)
	{
		if (!widget->sizeXSet) widget->size.x = parentSize.x - widget->margin.right - widget->margin.left;
		if (widget->marginLeftSet) widget->pos.x = widget->margin.left - widget->anchor.x * (widget->size.x + widget->margin.left * 2.f);
		widget->pos.x += widget->pivot.x * widget->size.x;
	}

	if (widget->marginBottomSet || widget->marginTopSet)
	{
		if (!widget->sizeYSet) widget->size.y = parentSize.y - widget->margin.top - widget->margin.bottom;
		if (widget->marginBottomSet) widget->pos.y = widget->margin.bottom - widget->anchor.y * (widget->size.y + widget->margin.bottom * 2.f);
		widget->pos.y += widget->pivot.y * widget->size.y;
	}

	//Move into parent-space based on pivot and anchor
	vec2 worldSpaceAnchor = parentPos + widget->anchor * parentSize;
	widget->pos = worldSpaceAnchor + widget->pos - (widget->size * widget->pivot);

	widget->renderDepth = guiDepth;
	guiDepth -= 0.0001f;

	widget->dirty = false;
}

void GUIContext::_Widget(u64 id)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	//Create new widget
	GUIWidget widget;
	widget.id = id;
	widget.parentID = widgetStack.back();
	widgetStack.push_back(id);
	widgetPool[id] = widget;
}

void GUIContext::_Image(u64 id)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	GUIWidget widget;
	widget.id = id;
	widget.parentID = widgetStack.back();
	widget.componentType = GUI_IMAGE;

	widgetStack.push_back(id);
	widgetPool[id] = widget;
	imagePool[id] = defaultImage;
	renderWidgets.push_back(id);
}

void GUIContext::_Label(u64 id)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	GUIWidget widget;
	widget.id = id;
	widget.parentID = widgetStack.back();
	widget.componentType = GUI_LABEL;

	widgetStack.push_back(id);
	widgetPool[id] = widget;
	GUILabel label = defaultLabel;
	label.font = defaultFont;
	labelPool[id] = label;
	renderWidgets.push_back(id);
}

void GUIContext::_Button(u64 id)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	GUIWidget widget;
	widget.id = id;
	widget.parentID = widgetStack.back();
	widget.componentType = GUI_BUTTON;
	widget.receiveInput = true;

	widgetStack.push_back(id);
	widgetPool[id] = widget;
	buttonPool[id] = defaultButton;
	renderWidgets.push_back(id);
}

void GUIContext::_LabelButton(u64 buttonID, u64 labelID, std::string _text)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	_Button(buttonID);
		_Label(labelID);
			text(_text);
			margin(Edges::Zero());
			textAlignment(CENTER);
		EndNode();
}

void GUIContext::_Tickbox(u64 id)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	GUIWidget widget;
	widget.id = id;
	widget.parentID = widgetStack.back();
	widget.componentType = GUI_TICKBOX;
	widget.receiveInput = true;

	widgetStack.push_back(id);
	widgetPool[id] = widget;
	tickboxPool[id] = defaultTickbox;
	renderWidgets.push_back(id);
}

void GUIContext::_Slider(u64 id)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	GUIWidget widget;
	widget.id = id;
	widget.parentID = widgetStack.back();
	widget.componentType = GUI_SLIDER;
	widget.receiveInput = true;

	widgetStack.push_back(id);
	widgetPool[id] = widget;
	GUISlider slider = defaultSlider;
	slider.font = defaultFont;
	sliderPool[id] = slider;
	renderWidgets.push_back(id);
}

void GUIContext::_TextField(u64 id)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	GUIWidget widget;
	widget.id = id;
	widget.parentID = widgetStack.back();
	widget.componentType = GUI_TEXT_FIELD;
	widget.receiveInput = true;

	widgetStack.push_back(id);
	widgetPool[id] = widget;

	if (textFieldPool.find(id) == textFieldPool.end())
	{
		textFieldPool[id] = defaultTextField;
	}

	textFieldPool[id].font = defaultFont;
	
	renderWidgets.push_back(id);
}

void GUIContext::_FloatField(u64 id)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	GUIWidget widget;
	widget.id = id;
	widget.parentID = widgetStack.back();
	widget.componentType = GUI_FLOAT_FIELD;
	widget.receiveInput = true;

	widgetStack.push_back(id);
	widgetPool[id] = widget;

	if (floatFieldPool.find(id) == floatFieldPool.end())
	{
		floatFieldPool[id] = defaultFloatField;
	}

	floatFieldPool[id].font = defaultFont;

	renderWidgets.push_back(id);
}

void GUIContext::_Row(u64 id)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	GUIWidget widget;
	widget.id = id;
	widget.parentID = widgetStack.back();
	widget.componentType = GUI_ROW;

	widgetStack.push_back(id);
	widgetPool[id] = widget;
	rowPool[id] = defaultRow;
}

void GUIContext::_Column(u64 id)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	GUIWidget widget;
	widget.id = id;
	widget.parentID = widgetStack.back();
	widget.componentType = GUI_COLUMN;

	widgetStack.push_back(id);
	widgetPool[id] = widget;
	columnPool[id] = defaultColumn;
}

void GUIContext::pos(vec2 pos)
{
	widgetPool[widgetStack.back()].pos = pos;
}

void GUIContext::posX(float x)
{
	widgetPool[widgetStack.back()].pos.x = x;
}

void GUIContext::posY(float y)
{
	widgetPool[widgetStack.back()].pos.y = y;
}
void GUIContext::size(vec2 size)
{
	widgetPool[widgetStack.back()].size = size;
	widgetPool[widgetStack.back()].sizeXSet = true;
	widgetPool[widgetStack.back()].sizeYSet = true;
}

void GUIContext::width(float width)
{
	widgetPool[widgetStack.back()].size.x = width;
	widgetPool[widgetStack.back()].sizeXSet = true;
}

void GUIContext::height(float height)
{
	widgetPool[widgetStack.back()].size.y = height;
	widgetPool[widgetStack.back()].sizeYSet = true;
}

void GUIContext::pivot(vec2 pivot)
{
	widgetPool[widgetStack.back()].pivot = pivot;
}

void GUIContext::anchor(vec2 anchor)
{
	widgetPool[widgetStack.back()].anchor = anchor;
}

void GUIContext::margin(Edges margin)
{
	widgetPool[widgetStack.back()].margin = margin;
	widgetPool[widgetStack.back()].marginLeftSet = true;
	widgetPool[widgetStack.back()].marginTopSet = true;
	widgetPool[widgetStack.back()].marginRightSet = true;
	widgetPool[widgetStack.back()].marginBottomSet = true;
}

void GUIContext::marginLeft(float left)
{
	widgetPool[widgetStack.back()].margin.left = left;
	widgetPool[widgetStack.back()].marginLeftSet = true;
}

void GUIContext::marginTop(float top)
{
	widgetPool[widgetStack.back()].margin.top = top;
	widgetPool[widgetStack.back()].marginTopSet = true;
}

void GUIContext::marginRight(float right)
{
	widgetPool[widgetStack.back()].margin.right = right;
	widgetPool[widgetStack.back()].marginRightSet = true;
}

void GUIContext::marginBottom(float bottom)
{
	widgetPool[widgetStack.back()].margin.bottom = bottom;
	widgetPool[widgetStack.back()].marginBottomSet = true;
}

void GUIContext::receiveInput(bool receiveInput)
{
	widgetPool[widgetStack.back()].receiveInput = receiveInput;
}

void GUIContext::color(vec4 color)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_IMAGE
		|| widget->componentType == GUI_LABEL
		|| widget->componentType == GUI_BUTTON
		|| widget->componentType == GUI_TICKBOX
		|| widget->componentType == GUI_SLIDER
		|| widget->componentType == GUI_TEXT_FIELD);

	if (widget->componentType == GUI_IMAGE)
	{
		imagePool[widget->id].color = color;
	}
	else if (widget->componentType == GUI_LABEL)
	{
		labelPool[widget->id].color = color;
	}
	else if (widget->componentType == GUI_BUTTON)
	{
		buttonPool[widget->id].color = color;
	}
	else if (widget->componentType == GUI_TICKBOX)
	{
		tickboxPool[widget->id].color = color;
	}
	else if (widget->componentType == GUI_SLIDER)
	{
		sliderPool[widget->id].color = color;
	}
	else if (widget->componentType == GUI_TEXT_FIELD)
	{
		textFieldPool[widget->id].color = color;
	}
}

void GUIContext::source(GUIImageSource source)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_IMAGE);
	imagePool[widget->id].source = source;
}

void GUIContext::nineSliceMargin(Edges nineSliceMargin)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_IMAGE
		|| widget->componentType == GUI_BUTTON
		|| widget->componentType == GUI_SLIDER
		|| widget->componentType == GUI_TEXT_FIELD);

	if (widget->componentType == GUI_IMAGE)
	{
		imagePool[widget->id].nineSliceMargin = nineSliceMargin;
	}
	else if (widget->componentType == GUI_BUTTON)
	{
		buttonPool[widget->id].nineSliceMargin = nineSliceMargin;
	}
	else if (widget->componentType == GUI_SLIDER)
	{
		sliderPool[widget->id].nineSliceMargin = nineSliceMargin;
	}
	else if (widget->componentType == GUI_TEXT_FIELD)
	{
		textFieldPool[widget->id].nineSliceMargin = nineSliceMargin;
	}
}

void GUIContext::text(std::string text)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_LABEL
		|| widget->componentType == GUI_TEXT_FIELD);

	if (widget->componentType == GUI_LABEL)
	{
		labelPool[widget->id].text = text;
	}
	else if (widget->componentType == GUI_TEXT_FIELD)
	{
		textFieldPool[widget->id].text = text;
	}
}
void GUIContext::font(Font* font)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_LABEL
		|| widget->componentType == GUI_SLIDER
		|| widget->componentType == GUI_TEXT_FIELD);

	if (widget->componentType == GUI_LABEL)
	{
		labelPool[widget->id].font = font;
	}
	else if (widget->componentType == GUI_SLIDER)
	{
		sliderPool[widget->id].font = font;
	}
	else if (widget->componentType == GUI_TEXT_FIELD)
	{
		textFieldPool[widget->id].font = font;
	}
}
void GUIContext::textAlignment(vec2 textAlignment)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_LABEL
		|| widget->componentType == GUI_SLIDER
		|| widget->componentType == GUI_TEXT_FIELD);

	if (widget->componentType == GUI_LABEL)
	{
		labelPool[widget->id].textAlignment = textAlignment;
	}
	else if (widget->componentType == GUI_SLIDER)
	{
		sliderPool[widget->id].textAlignment = textAlignment;
	}
	else if (widget->componentType == GUI_TEXT_FIELD)
	{
		textFieldPool[widget->id].textAlignment = textAlignment;
	}
}
void GUIContext::textHeightInPixels(float textHeightInPixels)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_LABEL
		|| widget->componentType == GUI_SLIDER
		|| widget->componentType == GUI_TEXT_FIELD);

	if (widget->componentType == GUI_LABEL)
	{
		labelPool[widget->id].textHeightInPixels = textHeightInPixels;
	}
	else if (widget->componentType == GUI_SLIDER)
	{
		sliderPool[widget->id].textHeightInPixels = textHeightInPixels;
	}
	else if (widget->componentType == GUI_TEXT_FIELD)
	{
		textFieldPool[widget->id].textHeightInPixels = textHeightInPixels;
	}
}

void GUIContext::hoverColor(vec4 color)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_BUTTON);
	buttonPool[widget->id].hoverColor = color;
}

void GUIContext::pressColor(vec4 color)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_BUTTON);
	buttonPool[widget->id].pressColor = color;
}

void GUIContext::onHoverEnter(std::function<void(void)> onHoverEnter)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_BUTTON);
	buttonPool[widget->id].onHoverEnter = onHoverEnter;
}

void GUIContext::onHover(std::function<void(void)> onHover)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_BUTTON);
	buttonPool[widget->id].onHover = onHover;
}

void GUIContext::onHoverExit(std::function<void(void)> onHoverExit)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_BUTTON);
	buttonPool[widget->id].onHoverExit = onHoverExit;
}

void GUIContext::onPress(std::function<void(void)> onPress)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_BUTTON);
	buttonPool[widget->id].onPress = onPress;
}

void GUIContext::onHold(std::function<void(void)> onHold)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_BUTTON);
	buttonPool[widget->id].onHold = onHold;
}

void GUIContext::onRelease(std::function<void(void)> onRelease)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_BUTTON);
	buttonPool[widget->id].onRelease = onRelease;
}

void GUIContext::value(bool* value)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_TICKBOX);
	tickboxPool[widget->id].value = value;
}

void GUIContext::value(float* value)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_SLIDER
		|| widget->componentType == GUI_FLOAT_FIELD);

	if (widget->componentType == GUI_SLIDER)
	{
		sliderPool[widget->id].value = value;
	}
	else if (widget->componentType == GUI_FLOAT_FIELD)
	{
		floatFieldPool[widget->id].value = value;
	}
}

void GUIContext::value(std::string* value)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_TEXT_FIELD);
	textFieldPool[widget->id].value = value;
}

void GUIContext::onValueChanged(std::function<void(float)> onValueChanged)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_FLOAT_FIELD);
	floatFieldPool[widget->id].onValueChanged = onValueChanged;
}

void GUIContext::min(float min)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_SLIDER
		|| widget->componentType == GUI_FLOAT_FIELD);

	if (widget->componentType == GUI_SLIDER)
	{
		sliderPool[widget->id].min = min;
	}
	else if (widget->componentType == GUI_FLOAT_FIELD)
	{
		floatFieldPool[widget->id].min = min;
	}
}

void GUIContext::max(float max)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_SLIDER
		|| widget->componentType == GUI_FLOAT_FIELD);

	if (widget->componentType == GUI_SLIDER)
	{
		sliderPool[widget->id].max = max;
	}
	else if (widget->componentType == GUI_FLOAT_FIELD)
	{
		floatFieldPool[widget->id].max = max;
	}
}

void GUIContext::spacing(float spacing)
{
	GUIWidget* widget = &widgetPool[widgetStack.back()];
	assert(widget->componentType == GUI_ROW
		|| widget->componentType == GUI_COLUMN);

	if (widget->componentType == GUI_ROW)
	{
		rowPool[widget->id].spacing = spacing;
	}
	else if (widget->componentType == GUI_COLUMN)
	{
		columnPool[widget->id].spacing = spacing;
	}
}

void GUIContext::EndNode()
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	GUIWidget* widget = &widgetPool[widgetStack.back()];
	buildWidgets.push_back(widget->id);
	widgetStack.pop_back();
}
