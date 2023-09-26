#pragma once

#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <functional>

#include <glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>

#ifdef TRACY_ENABLE
#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"
#endif

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

using std::cout;

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;

struct Rect
{
	vec2 min, max;
	Rect(vec2 min, vec2 max) : min(min), max(max) { }
};

struct Edges
{
	float top, right, bottom, left;
	Edges() : Edges(0.f, 0.f, 0.f, 0.f) { }
	Edges(float top, float right, float bottom, float left)
		: top(top), right(right), bottom(bottom), left(left) { }
	static Edges Zero() { return Edges(0.f, 0.f, 0.f, 0.f); }
	static Edges All(float value) { return Edges(value, value, value, value); }
};

//Time tracking
extern float avgFrameTime;
extern i32 framesPerSecond;

//TODO: Rethink these getters/setters, should the vars just be accessible?
float GetDeltaTime();
float GetAvgFrameTime();
u32 GetFPS();
float GetTime();

void SetFixedTimestep(float dt);
float GetFixedTimestep();
float GetTimestepAlpha();

struct Timer
{
	float timeElapsed;
	float speed;
	bool paused;

	~Timer();

	void Reset();
	void Stop();
	void Pause();
	void Play();
};

Timer* CreateTimer();

//Colors
void SetClearColor(vec4 color);

//Game Events
void BingusInit();
void RunGame();
bool GameShouldExit();
void ExitGame();
void BingusCleanup();

void SetGameStartFunction(void(*callback)());
void SetGameUpdateFunction(void(*callback)(float));
void SetGameFixedUpdateFunction(void(*callback)(float));
void SetGameDrawFunction(void(*callback)());

//Window
#define DEFAULT_WINDOW_WIDTH 1920
#define DEFAULT_WINDOW_HEIGHT 1080

void SetupWindow(i32 width, i32 height, const char* title);
GLFWwindow* GetWindow();
vec2 GetWindowSize();

void HandleWindowSizeChange(GLFWwindow* window, int width, int height);
void HandeMouseMove(GLFWwindow* window, double mouseX, double mouseY);
void HandleMouseScroll(GLFWwindow* window, double scrollX, double scrollY);

//Utility
double wrapMax(double x, double max);
double wrapMinMax(double x, double min, double max);
float wrapMax(float x, float max);
float wrapMinMax(float x, float min, float max);
vec4 hsv(vec4 hsv);
vec2 PixelToWorld(vec2 pixelCoord);
vec3 PixelToWorld(vec3 pixelCoord);
vec2 PixelToNDC(vec2 pixelCoord);
vec3 PixelToNDC(vec3 pixelCoord);
vec2 WorldToPixel(vec2 worldCoord);
float ParseFloat(std::string input, float oldValue = 0.f);

//Renderer
void InitializeRenderer();

extern struct RenderQueue globalRenderQueue;

//Camera stuff
extern mat4 cameraProjection;
extern mat4 cameraView;
extern mat4 cameraViewProj;
extern mat4 cameraViewProjInverse;

void SetCameraPosition(vec2 position, bool forceUpdateUBO = false);
void SetCameraSize(float size, bool forceUpdateUBO = false);
void TranslateCamera(vec2 translation);
void ZoomCamera(float sizeChange);
vec2 GetCameraPosition();
float GetCameraSize();
struct AABB GetCameraExents();

bool PointIntersectsCamera(vec2 position, float buffer = 0.f);

//Texture
struct Texture
{
	vec2 size;
	u32 id;
	i32 cachedWrapMode;
	i32 cachedFilterMode;

	void SetWrapMode(i32 wrapMode);
	void SetFilterMode(i32 filterMode);
};

//Returns a pointer to a managed texture resource. Will load from disk upon first call.
Texture* LoadTexture(std::string filenameAndPath);

//Shader
enum ShaderType { VERTEX, FRAGMENT };

#define SHADER_COLOR		0x01
#define SHADER_MAIN_TEX		0x02
#define SHADER_SPEC_POW		0x04 

struct Shader
{
	u32 id;
	u32 uniforms;
	std::unordered_map<u32, u32> uniformLocations;

	void EnableUniform(u32 uniformID, const char* uniformName);
	void EnableUniforms(u32 uniformMask);
	bool HasUniform(u32 uniform);

	void SetUniformInt(u32 uniform, int i);
	void SetUniformFloat(u32 uniform, float f);
	void SetUniformVec2(u32 uniform, vec2 v2);
	void SetUniformVec3(u32 uniform, vec3 v3);
	void SetUniformVec4(u32 uniform, vec4 v4);
};

Shader* LoadShader(std::string vertexFilenameAndPath, std::string fragFilenameAndPath);
extern u32 activeShaderID;

//Vertex
enum VertexType { POS_COLOR, POS_UV, POS_UV_COLOR };

struct Vertex_PosColor
{
	vec3 position;
	vec4 color;

	Vertex_PosColor(vec3 position, vec4 color)
	{
		this->position = position;
		this->color = color;
	}
};

struct Vertex_PosUV
{
	vec3 position;
	vec2 uv;

	Vertex_PosUV(vec3 position, vec2 uv)
	{
		this->position = position;
		this->uv = uv;
	}
};

struct Vertex_PosUVColor
{
	vec3 position;
	vec2 uv;
	vec4 color;

	Vertex_PosUVColor(vec3 position, vec2 uv, vec4 color)
	{
		this->position = position;
		this->uv = uv;
		this->color = color;
	}
};

struct VertBuffer
{
	GLuint vao, vbo, ebo;
	VertexType vertexType;
	std::vector<Vertex_PosColor> posColorVerts;
	std::vector<Vertex_PosUV> posUVVerts;
	std::vector<Vertex_PosUVColor> posUVColorVerts;
	std::vector<u32> vertexIndices;
	void* bufferData;
	u32 vertexCount;
	u32 vertexSize;
	bool dirty; //Is true if vertices need to be sent to GPU

	VertBuffer() : VertBuffer(POS_COLOR) { }
	VertBuffer(VertexType vertexType);
	void Clear();
	void Destroy();
};

//Render pipeline
void SetActiveShader(Shader* shader);
extern Shader* activeShader;

//Rectangle anchors
#define TOP_LEFT		vec2(0.0, 1.0)
#define TOP_CENTER		vec2(0.5, 1.0)
#define TOP_RIGHT		vec2(1.0, 1.0)
#define CENTER_LEFT		vec2(0.0, 0.5)
#define CENTER			vec2(0.5, 0.5)
#define CENTER_RIGHT	vec2(1.0, 0.5)
#define BOTTOM_LEFT		vec2(0.0, 0.0)
#define BOTTOM_CENTER	vec2(0.5, 0.0)
#define BOTTOM_RIGHT	vec2(1.0, 0.0)

//Sprites
struct SpriteSequenceFrame
{
	Edges nineSliceSample;
	Rect rect;
	SpriteSequenceFrame(Edges nineSliceSample, Rect rect)
		: nineSliceSample(nineSliceSample), rect(rect) { }
};

struct SpriteSequence
{
	std::vector<SpriteSequenceFrame> frames;

	SpriteSequence() { }
	SpriteSequence(vec2 firstFramePosition, vec2 frameSize, u32 count, float spacing);
	SpriteSequence(std::vector<SpriteSequenceFrame> frames);
};

struct SpriteSheet
{
	Texture* texture;
	std::map<std::string, SpriteSequence> sequences;

	SpriteSheet() { }
	SpriteSheet(Texture* texture);
	SpriteSheet(Texture* texture, std::map<std::string, SpriteSequence> sequences);
};

struct SpriteAnimator
{
	SpriteSequence* sequence;
	SpriteSheet* sheet;
	Timer* timer;

	SpriteAnimator() { }
	SpriteAnimator(SpriteSheet* sheet, std::string sequenceName, float speed);
	u32 GetFrame();
	void SetSequence(std::string name);
};

struct Sprite
{
	vec4 color = vec4(1);
	Edges nineSliceMargin = Edges::Zero();
	vec3 position = vec3(0);
	vec2 size = vec2(0);
	vec2 pivot = vec2(0);
	SpriteSequence* sequence = nullptr;
	SpriteAnimator* animator = nullptr;
	u32 sequenceFrame = 0;
	float rotation = 0.f;
};

struct RenderBatch
{
	VertBuffer* buffer;
	Shader* shader;
	Texture* texture;
	u32 drawMode = GL_TRIANGLES;
	
	void Draw();
};

struct SpriteBatch : RenderBatch
{
	SpriteSheet* sheet;

	SpriteBatch() { }
	SpriteBatch(VertBuffer* vertBuffer, Shader* shader, SpriteSheet* spriteSheet);

	void PushSprite(const Sprite& sprite);
	void PushSprite9Slice(const Sprite& sprite);
};

struct FontCharacter
{
	vec2 uvMin, uvMax;
	vec2 size, bearing;
	float advance;
};

struct FontCharacterRect
{
	vec2 position, size;
	FontCharacter* character;
};

struct Font
{
	Texture texture;
	u32 lineHeight;
	std::map<i32, FontCharacter> characters;
};

Font* LoadFont(std::string filePath, u32 pixelHeight);

struct TextRenderInfo
{
	std::vector<vec2> caretPositionOffsets;
	std::vector<u32> lineEndCharacters;
	std::vector<vec2> lineEndOffsets;
	vec2 renderScale;
	float lineHeightInPixels;
};

struct Text
{
	std::string data = "";
	Font* font = nullptr;
	vec4 color = vec4(1);
	vec3 position = vec3(0);
	vec2 extents = vec2(1, 0);
	vec2 scale = vec2(1);
	vec2 alignment = vec2(0);
	float textSize = 1.f;
};

struct TextBatch : RenderBatch
{
	Font* font;

	TextBatch() { }
	TextBatch(VertBuffer* buffer, Shader* shader, Font* font);

	void PushText(const Text& text);
	void PushText(const Text& text, TextRenderInfo& info);
};

struct RenderQueue
{
	//TODO: Perhaps this should allow more control. Could be more flexible if I split up everything that happens in Clear()
	//TODO: Implement a RenderTree - this will save some batches in the GUI render
	std::vector<SpriteBatch> spriteBatches;
	u32 currentSpriteBatchIndex;
	std::vector<TextBatch> textBatches;

	u32 currentTextBatchIndex;

	struct Step
	{
		i32 spriteBatchIndex = -1;
		i32 textBatchIndex = -1;
		void(*preDraw)();
		void(*postDraw)();
	};

	std::vector<Step> steps;
	u32 stepIndex;

	//VertBuffer* buffer;
	Shader* spriteShader;
	Shader* textShader;
	SpriteSheet* spriteSheet;
	Font* font;

	RenderQueue() { }
	
	void Clear();
	void AddStep();
	void AddStep(void(*preDraw)(), void(*postDraw)());
	void PushSprite(const Sprite& sprite);
	void PushText(const Text& text);
	void PushText(const Text& text, TextRenderInfo& info);
	void Draw();
};

//Entity
// struct Entity
// {
// 	virtual void Tick() { }
// 	virtual void Draw(SpriteBatch* batch) { }
// };
// 
// struct Scene
// {
// 	std::vector<Entity> entities;
// 
// 	Scene();
// 	void Draw();
// };
// 
// struct TestEntity : Entity
// {
// 	void Tick() override;
// 	void Draw(SpriteBatch* batch) override;
// };

//Input
//TODO: Add rest of keyboard
#define MOUSE_LEFT			0
#define MOUSE_RIGHT			1
#define MOUSE_MIDDLE		2
#define MOUSE_SCROLL_UP		3
#define MOUSE_SCROLL_DOWN	4
							
#define KEY_A				5
#define KEY_B				6
#define KEY_C				7
#define KEY_D				8
#define KEY_E				9
#define KEY_F				10
#define KEY_G				11
#define KEY_H				12
#define KEY_I				13
#define KEY_J				14
#define KEY_K				15
#define KEY_L				16
#define KEY_M				17
#define KEY_N				18
#define KEY_O				19
#define KEY_P				20
#define KEY_Q				21
#define KEY_R				22
#define KEY_S				23
#define KEY_T				24
#define KEY_U				25
#define KEY_V				26
#define KEY_W				27
#define KEY_X				28
#define KEY_Y				29
#define KEY_Z				30
							
#define KEY_0				31
#define KEY_1				32
#define KEY_2				33
#define KEY_3				34
#define KEY_4				35
#define KEY_5				36
#define KEY_6				37
#define KEY_7				38
#define KEY_8				39
#define KEY_9				40
							
#define KEY_APOSTROPHE		41
#define KEY_COMMA			42
#define KEY_MINUS			43
#define KEY_PERIOD			44
#define KEY_SLASH			45
#define KEY_BACKSLASH		46
#define KEY_EQUAL			47
#define KEY_LEFT_BRACKET	48
#define KEY_RIGHT_BRACKET	49
#define KEY_GRAVE_ACCENT	50
#define KEY_SEMICOLON		51

#define KEY_SPACE			52
#define KEY_ESCAPE			53
#define KEY_ENTER			54
#define KEY_TAB				55
#define KEY_BACKSPACE		56
#define KEY_INSERT			57
#define KEY_DELETE			58
#define KEY_UP				59
#define KEY_RIGHT			60
#define KEY_DOWN			61
#define KEY_LEFT			62
#define KEY_PAGE_UP			63
#define KEY_PAGE_DOWN		64
#define KEY_HOME			65
#define KEY_END				66
#define KEY_CAPS_LOCK		67
#define KEY_SCROLL_LOCK		68
#define KEY_NUM_LOCK		69
#define KEY_PRINT_SCREEN	70
#define KEY_PAUSE			71

#define KEY_CONTROL_LEFT	72
#define KEY_ALT_LEFT		73
#define KEY_SHIFT_LEFT		74
#define KEY_CONTROL_RIGHT	75
#define KEY_ALT_RIGHT		76
#define KEY_SHIFT_RIGHT		77

#define KEY_F1				78
#define KEY_F2				79
#define KEY_F3				80
#define KEY_F4				81
#define KEY_F5				82
#define KEY_F6				83
#define KEY_F7				84
#define KEY_F8				85
#define KEY_F9				86
#define KEY_F10				87
#define KEY_F11				88
#define KEY_F12				89

#define KEY_LAST			90

#define KEY_MOD_NONE		0
#define KEY_MOD_CONTROL		1
#define KEY_MOD_ALT			2
#define KEY_MOD_SHIFT		4
#define KEY_MOD_ANY			8

enum InputState
{
	PRESS, HOLD, RELEASE, UP
};

typedef std::function<void(void)> InputCallback;

struct InputEvent
{
	u32 key;
	InputState state;
	u32 modifier;

	bool operator==(const InputEvent& other) const
	{
		return key == other.key
			&& state == other.state
			&& modifier == other.modifier;
	}

	bool operator!= (const InputEvent& other) const
	{
		return !(*this == other);
	}
};

struct InputEventKeyHasher
{
	std::size_t operator()(const InputEvent& ie) const
	{
		return ((std::hash<u32>()(ie.key)
			 ^ (std::hash<InputState>()(ie.state) << 1)) >> 1)
			 ^ (std::hash<u32>()(ie.modifier) << 1);
	}
};

struct InputBinding
{
	InputCallback callback;
	std::string eventName;
	bool blocking;
};

struct InputListener
{
	std::unordered_map<InputEvent, InputBinding, InputEventKeyHasher> bindings;
	std::unordered_map<std::string, InputCallback> namedEvents;
	std::function<void(u32)> onCharacterTyped;
	i32 priority;
	bool blocking;

	InputListener() : InputListener(0, true) { }
	InputListener(i32 priority, bool blocking = true);

	void BindAction(u32 key, InputState state, InputCallback callback);
	void BindAction(u32 key, InputState state, std::string namedEvent);
	void BindAction(u32 key, InputState state, u32 modifier, InputCallback callback);
	void BindAction(u32 key, InputState state, u32 modifier, std::string namedEvent);
	//void BindAction(u32 key, InputState state, bool blocking, InputCallback callback);
	void BindAction(u32 key, InputState state, u32 modifier, bool blocking, InputCallback callback);
	void BindAction(u32 key, InputState state, u32 modifier, bool blocking, std::string namedEvent);
	void UnbindAction(u32 key, InputState state);
	void UnbindAction(u32 key, InputState state, u32 modifier);
	void BindNamedEvent(std::string name, InputCallback callback);
	void UnbindNamedEvent(std::string name);
	void SetPriority(i32 priority);
};

extern InputListener globalInputListener;

extern vec2 mousePosition;
extern vec3 mouseWorldPosition;
extern vec2 mouseDelta;
extern vec3 mouseWorldDelta;

void InitializeInput(GLFWwindow* window);
void UpdateInput(GLFWwindow* window, float dt);
void RegisterInputListener(InputListener* listener);
void UnregisterInputListener(InputListener* listener);
std::string GetInputBindingName(u32 binding);

//GUI
enum GUIWidgetComponent { GUI_NONE, GUI_IMAGE, GUI_LABEL, GUI_BUTTON, GUI_TICKBOX, GUI_SLIDER, GUI_TEXT_FIELD, GUI_FLOAT_FIELD, GUI_ROW, GUI_COLUMN };
enum GUIImageSource { BLOCK, BOX, CROSS, TICK, MINUS, PLUS, ARROW_UP, ARROW_RIGHT, ARROW_DOWN, ARROW_LEFT, GLASS, TEXT_FIELD_BG };

struct GUIWidget
{
	u64 id;
	u64 parentID;
	vec2 pos;
	vec2 size;
	vec2 pivot;
	vec2 anchor;
	Edges margin;
	bool sizeXSet;
	bool sizeYSet;
	bool marginLeftSet;
	bool marginRightSet;
	bool marginTopSet;
	bool marginBottomSet;
	float renderDepth;
	bool receiveInput;
	bool dirty;
	GUIWidgetComponent componentType;

	GUIWidget()
	{
		id = 0;
		parentID = 0;
		pos = vec2(0);
		size = vec2(0);
		pivot = TOP_LEFT;
		anchor = TOP_LEFT;
		margin = Edges::Zero();
		sizeXSet = false;
		sizeYSet = false;
		marginLeftSet = false;
		marginRightSet = false;
		marginTopSet = false;
		marginBottomSet = false;
		renderDepth = 0.f;
		receiveInput = false;
		dirty = true;
		componentType = GUI_NONE;
	}
};

struct GUIImage
{
	vec4 color;
	Edges nineSliceMargin;
	GUIImageSource source;

	GUIImage()
	{
		color = vec4(1);
		nineSliceMargin = Edges::Zero();
		source = BLOCK;
	}
};

struct GUILabel
{
	std::string text;
	Font* font;
	vec4 color;
	vec2 textAlignment;
	float textHeightInPixels;

	GUILabel()
	{
		text = "";
		font = nullptr; //This is set to defaultFont in the _Label() method of GUIContext
		color = vec4(1);
		textAlignment = TOP_LEFT;
		textHeightInPixels = 26.f;
	}
};

struct GUIButton
{
	vec4 color;
	vec4 hoverColor;
	vec4 pressColor;
	Edges nineSliceMargin;
	std::function<void(void)> onHoverEnter;
	std::function<void(void)> onHover;
	std::function<void(void)> onHoverExit;
	std::function<void(void)> onPress;
	std::function<void(void)> onHold;
	std::function<void(void)> onRelease;

	GUIButton()
	{
		color = vec4(1);
		hoverColor = vec4(vec3(0.9), 1.0);
		pressColor = vec4(vec3(0.8), 1.0);
		nineSliceMargin = Edges::All(8.f);
		onHoverEnter = nullptr;
		onHover = nullptr;
		onHoverExit = nullptr;
		onPress = nullptr;
		onHold = nullptr;
		onRelease = nullptr;
	}
};

struct GUITickbox
{
	vec4 color;
	vec4 hoverColor;
	vec4 pressColor;
	Edges nineSliceMargin;
	bool* value;

	GUITickbox()
	{
		color = vec4(1);
		hoverColor = vec4(vec3(0.9), 1.0);
		pressColor = vec4(vec3(0.8), 1.0);
		nineSliceMargin = Edges::All(8.f);
		value = nullptr;
	}
};

struct GUISlider
{
	vec4 color;
	vec4 hoverColor;
	vec4 pressColor;
	Edges nineSliceMargin;
	vec2 textAlignment;
	Font* font;
	float* value;
	float textHeightInPixels;
	float min;
	float max;

	GUISlider()
	{
		color = vec4(1);
		hoverColor = vec4(vec3(0.9), 1.0);
		pressColor = vec4(vec3(0.8), 1.0);
		nineSliceMargin = Edges::All(8.f);
		textAlignment = CENTER_LEFT;
		font = nullptr; //This is set to defaultFont in the _Slider() method of GUIContext
		value = nullptr;
		textHeightInPixels = 26.f;
		min = 0.f;
		max = 1.f;
	}
};

struct GUITextField
{
	Edges nineSliceMargin;
	vec4 color;

	std::string text;
	std::string* value;
	Font* font;
	vec2 textAlignment;
	float textHeightInPixels;
	
	//TODO: Try to move this state out of here, it can be global.
	//That way we don't have to track the state of existing text fields
	
	TextRenderInfo textInfo;
	
	GUITextField()
	{
		nineSliceMargin = Edges::All(8.f);
		color = vec4(1);
		value = nullptr;
		font = nullptr; //This is set to defaultFont in the _TextField() method of GUIContext
		textAlignment = TOP_LEFT;
		textHeightInPixels = 26.f;
	}
};

struct GUIFloatField
{
	vec4 color;
	vec4 hoverColor;
	vec4 pressColor;
	Edges nineSliceMargin;
	TextRenderInfo textInfo;
	vec2 textAlignment;
	Font* font;
	std::string text;
	float* value;
	float textHeightInPixels;
	float min;
	float max;

	GUIFloatField()
	{
		color = vec4(1);
		hoverColor = vec4(vec3(0.9), 1.0);
		pressColor = vec4(vec3(0.8), 1.0);
		nineSliceMargin = Edges::All(8.f);
		textAlignment = CENTER_LEFT;
		font = nullptr; //This is set to defaultFont in the _Slider() method of GUIContext
		text = "";
		value = nullptr;
		textHeightInPixels = 26.f;
		min = -FLT_MAX;
		max = FLT_MAX;
	}
};

//TODO: Refactor so that Row and Column are one thing (but still keep seperate Row() and Column() functions in GUIContext)
struct GUIRow
{
	float spacing;
	float offset;

	GUIRow()
	{
		spacing = 0.f;
		offset = 0.f;
	}
};

struct GUIColumn
{
	float spacing;
	float offset;

	GUIColumn()
	{
		spacing = 0.f;
		offset = 0.f;
	}
};

struct GUIContext
{
	SpriteSheet spriteSheet;
	SpriteSequence* spriteSequence;
	Texture* spriteTexture;
	Font* defaultFont;
	float guiDepth;

	GUIImage defaultImage;
	GUILabel defaultLabel;
	GUIButton defaultButton;
	GUITickbox defaultTickbox;
	GUISlider defaultSlider;
	GUITextField defaultTextField;
	GUIFloatField defaultFloatField;
	GUIRow defaultRow;
	GUIColumn defaultColumn;

	void Start();
	void EndAndDraw();

	void _Widget(u64 id);
	void _Image(u64 id);
	void _Label(u64 id);
	void _Button(u64 id);
	void _LabelButton(u64 buttonID, u64 labelID, std::string _text);
	void _Tickbox(u64 id);
	void _Slider(u64 id);
	void _TextField(u64 id);
	void _FloatField(u64 id);
	void _Row(u64 id);
	void _Column(u64 id);

	void pos(vec2 pos);
	void posX(float x);
	void posY(float y);
	void size(vec2 size);
	void width(float width);
	void height(float height);
	void pivot(vec2 pivot);
	void anchor(vec2 anchor);
	void margin(Edges margin);
	void marginLeft(float left);
	void marginTop(float top);
	void marginRight(float right);
	void marginBottom(float bottom);
	void receiveInput(bool receiveInput);

	void color(vec4 color);
	void source(GUIImageSource source);
	void nineSliceMargin(Edges nineSliceMargin);

	void text(std::string text);
	void font(Font* font);
	void textAlignment(vec2 textAlignment);
	void textHeightInPixels(float textHeightInPixels);

	void hoverColor(vec4 color);
	void pressColor(vec4 color);
	void onHoverEnter(std::function<void(void)> onHoverEnter);
	void onHover(std::function<void(void)> onHover);
	void onHoverExit(std::function<void(void)> onHoverExit);
	void onPress(std::function<void(void)> onPress);
	void onHold(std::function<void(void)> onHold);
	void onRelease(std::function<void(void)> onRelease);

	void min(float min);
	void max(float max);

	void value(bool* value);
	void value(float* value);
	void value(std::string* value);

	void spacing(float spacing);

	void EndNode();

	void BuildWidget(u64 id);
};

extern GUIContext globalGUIContext;

#define WidgetKey(key) _Widget(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__ + key)))
#define Widget() _Widget(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__)))

#define ImageKey(key) _Image(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__ + key)))
#define Image() _Image(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__)))

#define LabelKey(key) _Label(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__ + key)))
#define Label() _Label(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__)))

#define ButtonKey(key) _Button(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__ + key)))
#define Button() _Button(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__)))

#define LabelButtonKey(key) _LabelButton(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__ + key)), std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__ + key) + "label"))
#define LabelButton(text) _LabelButton(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__)), std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__) + "label"), text)

#define TickboxKey(key) _Tickbox(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__ + key)))
#define Tickbox() _Tickbox(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__)))

#define SliderKey(key) _Slider(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__ + key)))
#define Slider() _Slider(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__)))

#define FloatFieldKey(key) _FloatField(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__ + key)))
#define FloatField() _FloatField(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__)))

#define TextFieldKey(key) _TextField(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__ + key)))
#define TextField() _TextField(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__)))

#define RowKey(key) _Row(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__ + key)))
#define Row() _Row(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__)))

#define ColumnKey(key) _Column(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__ + key)))
#define Column() _Column(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__)))

//Collision
struct Circle
{
	vec2 position;
	float radius;
	Circle() : position(vec2(0)), radius(0.f) { }
	Circle(vec2 position, float radius)
	{
		this->position = position;
		this->radius = radius;
	}
};

struct AABB
{
	vec2 min, max;
	AABB() : min(vec2(0)), max(vec2(0)) { }
	AABB(vec2 min, vec2 max)
	{
		this->min = min;
		this->max = max;
	}
};

struct Line
{
	vec2 a, b;
	Line() : a(vec2(0)), b(vec2(1, 0)) { }
	Line(vec2 a, vec2 b)
	{
		this->a = a;
		this->b = b;
	}
};

struct Ray
{
	vec2 start, direction;
	Ray() : start(vec2(0)), direction(vec2(1, 0)) { }
	Ray(vec2 start, vec2 direction)
	{
		this->start = start;
		this->direction = direction;
	}
};

struct Segment
{
	vec2 start, end;
	Segment() : start(vec2(0)), end(vec2(1, 0)) { }
	Segment(vec2 start, vec2 end)
	{
		this->start = start;
		this->end = end;
	}
};

struct Polygon
{
	std::vector<vec2> vertices;
};

float SqDistPointToAABB(vec2 point, AABB box);
vec2 ClosestPtPointAABB(vec2 point, AABB box);
bool TestAABBAABB(const AABB& a, const AABB& b);
bool TestCircleCircle(const Circle& a, const Circle& b);
bool TestCircleAABB(const Circle& circle, const AABB& box);
bool IntersectRayAABB(const Ray& ray, const AABB& box, vec2& p, float& t);
bool IntersectSegmentAABB(const Segment& segment, const AABB& box, vec2& p, float& t);
bool IntersectRayCircle(const Ray& ray, const Circle& circle, vec2& p, float& t);
bool IntersectSegmentCircle(const Segment& segment, const Circle& circle, vec2& p, float& t);
bool SweepCircleAABB(const Circle& circle, vec2 velocity, const AABB& box, float& t);

//Debug
#define DEBUG_LINE 1
#define DEBUG_DIAMOND 2
#define DEBUG_PENTAGON 3
#define DEBUG_HEXAGON 4
#define DEBUG_CIRCLE 5

#define DEBUG_WORLD 5
#define DEBUG_SCREEN 6

void InitializeDebug();
//void UpdateDebug();

void DrawDebugIcon(u32 space, u32 icon, vec3 position, float size, vec4 color, float timer = 0.f);
void DrawDebugLine(u32 space, vec3 from, vec3 to, float thickness, vec4 color, float timer = 0.f);
void DrawDebugLine(u32 space, vec2 from, vec2 to, float thickness, vec4 color, float timer = 0.f);
void DrawDebugAABB(u32 space, const AABB& aabb, vec4 color, bool fill, float timer = 0.f);
void DrawDebugCircle(u32 space, const Circle& circle, u32 subdiv, vec4 color, bool fill, float timer = 0.f);
void DrawDebugPolygon(u32 space, const Polygon& polygon, vec4 color, bool fill, float timer = 0.f);
void DrawDebugText(u32 space, vec3 position, float size, vec4 color, std::string data, float timer = 0.f);
void DrawDebugText(u32 space, vec2 position, float size, vec4 color, std::string data, float timer = 0.f);
void DrawDebug(float dt);
