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

#define NULL_FLOAT -std::numeric_limits<float>::max()
#define NULL_VEC2 vec2(NULL_FLOAT)

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
	static Edges None() { return Edges(0.f, 0.f, 0.f, 0.f); }
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

//Math
double wrapMax(double x, double max);
double wrapMinMax(double x, double min, double max);
float wrapMax(float x, float max);
float wrapMinMax(float x, float min, float max);
vec4 hsv(vec4 hsv);

#ifndef OLD_RENDERER

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
};

struct Vertex_PosUV
{
	vec3 position;
	vec2 uv;
};

struct Vertex_PosUVColor
{
	vec3 position;
	vec2 uv;
	vec4 color;
};

struct VertBuffer
{
	GLuint vao, vbo, ebo;
	VertexType vertexType;
	std::vector<Vertex_PosColor> posColorVerts;
	std::vector<Vertex_PosUV> posUVVerts;
	std::vector<Vertex_PosUVColor> posUVColorVerts;
	bool dirty; //Is true if vertices need to be sent to GPU

	VertBuffer(VertexType vertexType);
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
	vec4 color;
	Edges nineSliceMargin;
	vec3 position;
	vec2 size;
	vec2 pivot;
	SpriteSequence* sequence;
	SpriteAnimator* animator;
	u32 sequenceFrame;
	float rotation;

	Sprite()
	{
		//Default values
		color = vec4(1);
		nineSliceMargin = Edges::None();
		position = vec3(0);
		size = vec2(0);
		pivot = vec2(0);
		sequence = nullptr;
		animator = nullptr;
		sequenceFrame = 0;
		rotation = 0.f;
	};
};

struct RenderBatch
{
	VertBuffer buffer;
	Shader shader;
	Texture texture;

	std::vector<float> vertexData;
	std::vector<u32> indices;
	u32 drawMode = GL_TRIANGLES;

	bool initialized = false;
	bool bufferDirty = false;

	//RenderBatch() { }
	
	void LazyInit();
	virtual void Init() { };
	virtual void Clear();
	void GrowVertexCapacity(size_t capacity); //TODO: Try and remove this function
	void SendVertsToGPUBuffer();
	void Draw();
};

struct SpriteBatch : RenderBatch
{
	SpriteSheet* sheet;

	SpriteBatch() { }
	SpriteBatch(VertBuffer vertBuffer, Shader shader, SpriteSheet* spriteSheet);

	void Init() override;
	void Clear() override;
	void PushSprite(const Sprite& sprite);
	void PushSprite9Slice(const Sprite& sprite);
	void PushSprites(const std::vector<Sprite*>& sprites);
	void PushSprites(const std::vector<Sprite>& sprites);
	void PushSpritesReverse(const std::vector<Sprite*>& sprites);
	void PushSpritesReverse(const std::vector<Sprite>& sprites);
};


#endif



#ifdef OLD_RENDERER

//Texture
//TODO: Implement proper resource loading system
//TODO: Create texture type

struct Texture
{
	u32 id;
	vec2 size;

	Texture() { }
	Texture(u32 id, vec2 size); //This initializes a texture loaded manually
	Texture(const char* filePath, i32 wrapMode = GL_REPEAT, i32 filter = GL_LINEAR_MIPMAP_LINEAR); // This loads a texture from disk
};

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

	Shader() { }
	Shader(u32 vertShader, u32 fragShader);
	Shader(u32 vertShader, u32 fragShader, u32 uniformMask);
	Shader(const char* vertFilePath, const char* fragFilePath);
	Shader(const char* vertFilePath, const char* fragFilePath, u32 uniformMask);

	void EnableUniform(u32 uniformID, const char* uniformName);
	void EnableUniforms(u32 uniformMask);
	bool HasUniform(u32 uniform);

	void SetUniformInt(u32 uniform, int i);
	void SetUniformFloat(u32 uniform, float f);
	void SetUniformVec2(u32 uniform, vec2 v2);
	void SetUniformVec3(u32 uniform, vec3 v3);
	void SetUniformVec4(u32 uniform, vec4 v4);
};

u32 LoadShader(ShaderType, const char* file_path); //Decide where this should go
void SetActiveShader(Shader* shader);

//Renderer
#define TOP_LEFT		vec2(0.0, 1.0)
#define TOP_CENTER		vec2(0.5, 1.0)
#define TOP_RIGHT		vec2(1.0, 1.0)
#define CENTER_LEFT		vec2(0.0, 0.5)
#define CENTER			vec2(0.5, 0.5)
#define CENTER_RIGHT	vec2(1.0, 0.5)
#define BOTTOM_LEFT		vec2(0.0, 0.0)
#define BOTTOM_CENTER	vec2(0.5, 0.0)
#define BOTTOM_RIGHT	vec2(1.0, 0.0)

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
	Texture texture;
	std::map<std::string, SpriteSequence> sequences;

	SpriteSheet() { }
	//TODO: Should these constructors take in a texture and expect the user to load it seperately?
	//Perhaps having the option to do either would be best.
	//TODO: Add interface for passing in an existing texture
	SpriteSheet(const char* texturePath);
	SpriteSheet(const char* texturePath, std::map<std::string, SpriteSequence> sequences);
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
	vec4 color;
	Edges nineSliceMargin;
	vec3 position;
	vec2 size;
	vec2 pivot;
	SpriteSequence* sequence;
	SpriteAnimator* animator;
	u32 sequenceFrame;
	float rotation;

	Sprite() { };

	Sprite(vec3 position, vec2 size, vec2 pivot = BOTTOM_LEFT, float rotation = 0.f, vec4 color = vec4(1), SpriteSequence* sequence = nullptr, u32 frame = 0)
		: Sprite(position, size, pivot, rotation, Edges::None(), color, sequence, frame, nullptr) { }

	Sprite(vec3 position, vec2 size, vec2 pivot = BOTTOM_LEFT, float rotation = 0.f, Edges nineSliceMargin = Edges::None(), vec4 color = vec4(1), SpriteSequence* sequence = nullptr, u32 frame = 0)
		: Sprite(position, size, pivot, rotation, nineSliceMargin, color, sequence, frame, nullptr) { }

	Sprite(vec3 position, vec2 size, vec2 pivot = BOTTOM_LEFT, float rotation = 0.f, Edges nineSliceMargin = Edges::None(), vec4 color = vec4(1), SpriteAnimator* animator = nullptr)
		: Sprite(position, size, pivot, rotation, nineSliceMargin, color, nullptr, 0, animator) { }

	Sprite(vec3 position, vec2 size, vec2 pivot, float rotation, Edges nineSliceMargin, vec4 color, SpriteSequence* sequence, u32 frame, SpriteAnimator* animator);
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

//TODO: Implement proper resource loading system
Font* LoadFont(const char* filePath, u32 pixelHeight);

struct Fonts
{
	static Font* arial;
	static Font* linuxLibertine;
};

struct TextRenderInfo
{
	vec2 caretOffsetInPixels;
	vec2 renderScale;
	float lineHeightInPixels;
};

struct Text
{
	std::string data;
	Font* font;
	vec4 color;
	vec3 position;
	vec2 extents;
	vec2 scale;
	vec2 alignment;
	float textSize;

	Text() { }
	Text(std::string data, vec3 position, vec2 extents, float textSize, Font* font)
		: Text(data, position, extents, vec2(1), BOTTOM_LEFT, textSize, vec4(1), font) { }
	Text(std::string data, vec3 position, vec2 extents, vec2 scale, vec2 alignment, float textSize, vec4 color, Font* font);
};

#define VERTEX_POS		0
#define VERTEX_UV		1
#define VERTEX_COLOR	2

struct VertAttrib
{
	u32 attribute, componentCount, componentWidth, type, offset;

	VertAttrib(u32 attribute, u32 componentCount, u32 componentWidth, u32 type, u32 offset);
};

struct VertBuffer
{
	GLuint vao, vbo, ebo;
	std::vector<VertAttrib> attributes;
	u32 vertexComponentCount;
	u32 vertexByteWidth;

	VertBuffer() { }
	VertBuffer(const std::vector<u32> attributes);

	VertAttrib* GetAttribute(u32 attribute);
	u32 GetAttributeOffset(u32 attribute);
	void Destroy();
};

struct RenderBatch
{
	VertBuffer buffer;
	Shader shader;
	Texture texture;

	std::vector<float> vertexData;
	std::vector<u32> indices;
	u32 drawMode = GL_TRIANGLES;

	size_t vertexCount = 0;
	size_t vertexCapacity = 0;

	bool initialized = false;
	bool bufferDirty = false;

	RenderBatch() { }
	
	void LazyInit();
	virtual void Init() { };
	virtual void Clear();
	void GrowVertexCapacity(size_t capacity); //TODO: Try and remove this function
	void SendVertsToGPUBuffer();
	void Draw();
};

struct SpriteBatch : RenderBatch
{
	SpriteSheet* sheet;
	VertAttrib* positionAttrib;
	VertAttrib* uvAttrib;
	VertAttrib* colorAttrib;

	SpriteBatch() { }
	SpriteBatch(VertBuffer vertBuffer, Shader shader, SpriteSheet* spriteSheet);

	void Init() override;
	void Clear() override;
	void PushSprite(const Sprite& sprite);
	void PushSprite9Slice(const Sprite& sprite);
	void PushSprites(const std::vector<Sprite*>& sprites);
	void PushSprites(const std::vector<Sprite>& sprites);
	void PushSpritesReverse(const std::vector<Sprite*>& sprites);
	void PushSpritesReverse(const std::vector<Sprite>& sprites);
};

struct TextBatch : RenderBatch
{
	Font* font;
	VertAttrib* positionAttrib;
	VertAttrib* uvAttrib;
	VertAttrib* colorAttrib;
	//u32 glyphIndex = 0;

	TextBatch() { }
	TextBatch(VertBuffer buffer, Shader shader, Font* font);

	void Init() override;
	void Clear() override;

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

	VertBuffer buffer;
	Shader spriteShader;
	Shader textShader;
	SpriteSheet* spriteSheet;
	Font* font;

	void Clear();
	void PushStep();
	void PushStep(void(*preDraw)(), void(*postDraw)());
	void PushSprite(const Sprite& sprite);
	void PushText(const Text& text);
	void PushText(const Text& text, TextRenderInfo& info);
	void Draw();
};

void InitializeRenderer();

extern RenderQueue globalRenderQueue;

//Camera stuff
extern mat4 cameraProjection;
extern mat4 cameraView;
extern mat4 cameraViewProjInverse;

void SetCameraPosition(vec2 position, bool forceUpdateUBO = false);
void SetCameraSize(float size, bool forceUpdateUBO = false);
void TranslateCamera(vec2 translation);
void ZoomCamera(float sizeChange);
vec2 GetCameraPosition();
float GetCameraSize();

bool PointIntersectsCamera(vec2 position, float buffer = 0.f);

#endif

//Entity
struct Entity
{
	virtual void Tick() { }
	virtual void Draw(SpriteBatch* batch) { }
};

struct Scene
{
	std::vector<Entity> entities;

	Scene();
	void Draw();
};

struct TestEntity : Entity
{
	void Tick() override;
	void Draw(SpriteBatch* batch) override;
};

//Input
//TODO: Add rest of keyboard
#define MOUSE_LEFT			0
#define MOUSE_RIGHT			1
#define MOUSE_SCROLL_UP		2
#define MOUSE_SCROLL_DOWN	3

#define KEY_A				4
#define KEY_B				5
#define KEY_C				6
#define KEY_D				7
#define KEY_E				8
#define KEY_F				9
#define KEY_G				10
#define KEY_H				11
#define KEY_I				12
#define KEY_J				13
#define KEY_K				14
#define KEY_L				15
#define KEY_M				16
#define KEY_N				17
#define KEY_O				18
#define KEY_P				19
#define KEY_Q				20
#define KEY_R				21
#define KEY_S				22
#define KEY_T				23
#define KEY_U				24
#define KEY_V				25
#define KEY_W				26
#define KEY_X				27
#define KEY_Y				28
#define KEY_Z				29

#define KEY_0				30
#define KEY_1				31
#define KEY_2				32
#define KEY_3				33
#define KEY_4				34
#define KEY_5				35
#define KEY_6				36
#define KEY_7				37
#define KEY_8				38
#define KEY_9				39

#define KEY_APOSTROPHE		40
#define KEY_COMMA			41
#define KEY_MINUS			42
#define KEY_PERIOD			43
#define KEY_SLASH			44
#define KEY_BACKSLASH		45
#define KEY_EQUAL			46
#define KEY_LEFT_BRACKET	47
#define KEY_RIGHT_BRACKET	48
#define KEY_GRAVE_ACCENT	49

#define KEY_SPACE			50
#define KEY_ESCAPE			51
#define KEY_ENTER			52
#define KEY_TAB				53
#define KEY_BACKSPACE		54
#define KEY_INSERT			55
#define KEY_DELETE			56
#define KEY_UP				57
#define KEY_RIGHT			58
#define KEY_DOWN			59
#define KEY_LEFT			60
#define KEY_PAGE_UP			61
#define KEY_PAGE_DOWN		62
#define KEY_HOME			63
#define KEY_END				64
#define KEY_CAPS_LOCK		65
#define KEY_SCROLL_LOCK		66
#define KEY_NUM_LOCK		67
#define KEY_PRINT_SCREEN	68
#define KEY_PAUSE			69
#define KEY_F0				70
#define KEY_F1				71
#define KEY_F2				72
#define KEY_F3				73
#define KEY_F4				74
#define KEY_F5				75
#define KEY_F6				76
#define KEY_F7				77
#define KEY_F8				78
#define KEY_F9				79
#define KEY_LAST			80

enum InputState
{
	PRESS, HOLD, RELEASE, UP
};

typedef std::function<void(void)> InputCallback;

struct InputEvent
{
	u32 key;
	InputState state;

	bool operator==(const InputEvent& other) const
	{
		return key == other.key && state == other.state;
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
		return ((std::hash<u32>()(ie.key) ^ (std::hash<InputState>()(ie.state) << 1)));
	}
};

struct InputBinding
{
	InputCallback callback;
	bool blocking;
};

struct InputListener
{
	std::unordered_map<InputEvent, InputBinding, InputEventKeyHasher> bindings;
	std::function<u32(void)> onKeyReceive;
	i32 priority;
	bool blocking;

	InputListener() : InputListener(0, true) { }
	InputListener(i32 priority, bool blocking = true);

	void BindAction(u32 key, InputState state, InputCallback callback);
	void BindAction(u32 key, InputState state, bool blocking, InputCallback callback);
	void UnbindAction(u32 key, InputState state);
	void SetPriority(i32 priority);
};

extern InputListener globalInputListener;
extern std::string* inputString;

extern vec2 mousePosition;
extern vec3 mouseWorldPosition;
extern vec2 mouseDelta;
extern vec3 mouseWorldDelta;

void InitializeInput(GLFWwindow* window);
void UpdateInput(GLFWwindow* window, float dt);
void RegisterInputListener(InputListener* listener);
void UnregisterInputListener(InputListener* listener);

//GUI
extern SpriteSheet defaultGuiSpritesheet;
extern SpriteSequence* defaultGuiSpriteSequence;
extern Font* defaultGuiFont;
extern bool mouseAboveGUI;

void InitializeGUI();
void SetGUICanvasSize(vec2 size);
void BeginGUI();
void BuildGUI();
void ProcessGUIInput();
void DrawGUI();

//TODO: Slider
//TODO: Scroll Rect
//TODO: Radial Button

enum LayoutType { NONE, HORIZONTAL, VERTICAL };
enum WidgetType { WIDGET, IMAGE, TEXT, BUTTON, TICKBOX, SLIDER, TEXTFIELD, LAYOUT, MASK, WINDOW };
enum GUIImageSource { BLOCK, BOX, CROSS, TICK, MINUS, PLUS, ARROW_UP, ARROW_RIGHT, ARROW_DOWN, ARROW_LEFT, GLASS, TEXT_FIELD_BG };

struct GUIWidgetVars
{
	vec2 pos;
	vec2 size;
	vec2 pivot;
	vec2 anchor;
	Edges margin;

	GUIImageSource source;
	vec4 color;
	Edges nineSliceMargin;

	bool receiveInput;

	LayoutType layoutType;
	float spacing;
	bool stretch;
	
	std::string text;
	std::string dummyText;
	vec2 textAlignment;
	float textHeightInPixels;
	Font* font;

	bool* state;
	bool hoveredState;
	InputState eventState;
	std::function<void(void)> onPress;
	std::function<void(void)> onHold;
	
	float* value;
	float min;
	float max;
	std::function<void(float)> onValueChanged;

	std::string* textValue;
	std::function<void(std::string)> onTextValueChanged;

	GUIWidgetVars();
};

struct GUIWidget
{
	u32 id;
	u32 parentID;
	std::vector<u32> children;
	
	float depth;
	float layoutOffset;
	GUIWidgetVars vars;
	WidgetType type;

	void Build();
	void Draw();
	void ProcessInput();
};

struct GUIWindow
{
	vec2 pos;
	vec2 size;
	vec2 minSize = vec2(100);
	vec2 maxSize = vec2(1000);
	vec2 grabSize = NULL_VEC2;
	vec2 grabPos = NULL_VEC2;
	bool grabbed = false;
	bool grabbedLeft = false;
	bool grabbedTop = false;
	bool grabbedRight = false;
	bool grabbedBottom = false;
	bool grabbedTopLeft = false;
	bool grabbedTopRight = false;
	bool grabbedBottomLeft = false;
	bool grabbedBottomRight = false;
};

//TODO: Rethink how to do this namespacing - it's very messy at the moment
namespace gui
{
	u32 Widget();
	u32 Image(GUIImageSource source);
	u32 Text(std::string text);
	u32 Button();
	u32 Button(bool* state, InputState eventState);
	u32 Tickbox(bool* state);
	u32 Slider(float* value);
	u32 TextField(std::string* textValue);
	u32 Layout(LayoutType type);
	u32 Mask();
	u32 Window(GUIWindow* window);
	void EndNode();

	extern GUIWidgetVars vars;
	GUIWidget& GetWidget(u32 id);
}

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
void DrawDebugText(u32 space, vec3 position, float size, vec4 color, std::string data, float timer = 0.f);
void DrawDebugText(u32 space, vec2 position, float size, vec4 color, std::string data, float timer = 0.f);
void DrawDebug(float dt);
