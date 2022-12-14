#pragma once

#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <functional>

#include <glad.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>

#define i8 int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define f32 float
#define f64 double

using std::cout;

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;

//Time tracking
extern float avgFrameTime;
extern i32 framesPerSecond;

float GetAvgFrameTime();
u32 GetFPS();
float GetTime();

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

void SetupWindow(i32 width, i32 height);
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

//Texture
//TODO: Implement proper resource loading system
u32 LoadTexture(const char* path, u32 wrapMode = GL_REPEAT);

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

u32 LoadShader(ShaderType, const char* file_path);
void SetActiveShader(Shader* shader);

//Renderer
//1. Store state of all sprites, state of camera
//2. Generate and draw sprite batches every frame
//    a. Perform culling
//    b. Handle blending

#define TOP_LEFT		vec2(0.0, 1.0)
#define TOP_CENTER		vec2(0.5, 1.0)
#define TOP_RIGHT		vec2(1.0, 1.0)
#define CENTER_LEFT		vec2(0.0, 0.5)
#define CENTER			vec2(0.5, 0.5)
#define CENTER_RIGHT	vec2(1.0, 0.5)
#define BOTTOM_LEFT		vec2(0.0, 0.0)
#define BOTTOM_CENTER	vec2(0.5, 0.0)
#define BOTTOM_RIGHT	vec2(1.0, 0.0)

struct SpriteSheet
{
	u32 texture, rows, columns;

	SpriteSheet() { }
	SpriteSheet(const char* filePath, u32 rows, u32 columns);
};

struct Sprite
{
	vec3 position;
	vec2 size;
	vec2 pivot;
	vec4 color;
	u32 row, column;
	float rotation;

	//TODO: Clean up these constructors?
	Sprite(vec3 position, vec2 size) : Sprite(position, size, 1, 1) { }
	Sprite(vec3 position, vec2 size, u32 row, u32 column)
	{
		this->position = position;
		this->size = size;
		this->row = row;
		this->column = column;
		this->rotation = 0.f;
		this->pivot = BOTTOM_LEFT;
		this->color = vec4(1);
	}
};

struct FontCharacter
{
	vec2 uvMin, uvMax;
	glm::ivec2 size, bearing;
	u32 advance;
};

struct FontCharacterRect
{
	vec2 position, size;
	FontCharacter* character;
};

struct Font
{
	u32 texture;
	u32 lineHeight;
	std::map<char, FontCharacter> characters;
};

//TODO: Implement proper resource loading system
Font* LoadFont(const char* filePath, u32 pixelHeight);

struct Fonts
{
	static Font* arial;
};

//enum Alignment { TOP_LEFT, TOP_CENTER, TOP_RIGHT,
//				 CENTER_LEFT, CENTER, CENTER_RIGHT,
//				 BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT };

struct Text
{
	vec3 position;
	vec2 extents;
	vec2 scale;
	vec2 alignment;
	float textSize;
	vec4 color;
	Font* font;
	std::string data;

	Text(std::string data, vec3 position, vec2 extents, float textSize, Font* font)
		: Text(data, position, extents, vec2(1), TOP_LEFT, textSize, vec4(1), font) { }
	Text(std::string data, vec3 position, vec2 extents, vec2 scale, vec2 alignment, float textSize, vec4 color, Font* font)
	{
		this->data = data;
		this->position = position;
		this->extents = extents;
		this->alignment = alignment;
		this->textSize = textSize;
		this->color = color;
		this->font = font;
		this->scale = scale;
	}
};

//struct Vertex
//{
//	vec3 position;
//	vec2 uv;
//	Vertex() { Vertex(vec3(0), vec2(0)); }
//	Vertex(vec3 position, vec2 uv) : position(position), uv(uv) { }
//};
//
//struct ColorVertex
//{
//	vec3 position;
//	vec2 uv;
//	vec4 color;
//	ColorVertex() { ColorVertex(vec3(0), vec2(0), vec4(1)); }
//	ColorVertex(vec3 position, vec2 uv, vec4 color) : position(position), uv(uv), color(color) { }
//};

#define VERTEX_POS		0
#define VERTEX_UV		1
#define VERTEX_COLOR	2

struct VertAttrib
{
	u32 attribute, componentCount, componentWidth, type, offset;

	VertAttrib(u32 attribute, u32 componentCount, u32 componentWidth, u32 type, u32 offset)
	{
		this->attribute = attribute;
		this->componentCount = componentCount;
		this->componentWidth = componentWidth;
		this->type = type;
		this->offset = offset;
	}
};

struct VertBuffer
{
	GLuint vao, vbo, ebo;
	std::vector<VertAttrib> attributes;
	u32 vertexTotalComponentCount;
	u32 vertexByteWidth;

	VertBuffer() { }
	VertBuffer(const std::vector<u32> attributes);

	VertAttrib* GetAttribute(u32 attribute);
	i32 GetAttributeOffset(u32 attribute);
};

//NOTE: What do I think of this batch object design? I do like having
//		the rendering be programmable from other parts of the code - 
//		the minimal design of it is good. Why does it not quite feel
//		right?
//TODO: Think about this more - take note of how all the responsibilities
//		are shared

struct RenderBatch
{
	VertBuffer buffer;
	Shader shader;
	u32 texture;

	std::vector<float> vertexData;
	std::vector<u32> indices;

	u32 vertexCount = 0;
	u32 vertexCapacity = 0;

	bool initialized = false;
	bool bufferDirty = false;

	RenderBatch() { }
	
	void LazyInit();
	virtual void Init() { };
	virtual void Clear();
	void GrowVertexCapacity(u32 capacity);
	void SendVertsToGPUBuffer();
	void Draw();
};

struct SpriteBatch : RenderBatch
{
	SpriteSheet* sheet;
	VertAttrib* positionAttrib;
	VertAttrib* uvAttrib;
	VertAttrib* colorAttrib;
	u32 spriteIndex = 0;
	bool initialized;

	SpriteBatch() { }
	SpriteBatch(VertBuffer vertBuffer, Shader shader, SpriteSheet* spriteSheet);

	void Init() override;
	void Clear() override;
	void PushSprite(const Sprite& sprite);
	void PushSprites(const std::vector<Sprite*>& sprites);
	void PushSprites(const std::vector<Sprite>& sprites);
	void PushSpritesReverse(const std::vector<Sprite*>& sprites);
	void PushSpritesReverse(const std::vector<Sprite>& sprites);
};

struct TextBatch : RenderBatch
{
	u32 totalGlyphIndex = 0;
	VertAttrib* positionAttrib;
	VertAttrib* uvAttrib;
	VertAttrib* colorAttrib;
	void Init() override;
	void PushText(Text* text);
};

void InitializeRenderer();
void DeleteSprite(Sprite* sprite, std::vector<Sprite*>* container);
void DeleteAllSprites(std::vector<Sprite*>* container);
void DeleteText(Text* text, std::vector<Text*>* container);
void DeleteAllTexts(std::vector<Text*>* container);

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

//Debug
#define DEBUG_TRIANGLE 0
#define DEBUG_DIAMOND 1
#define DEBUG_PENTAGON 2
#define DEBUG_HEXAGON 3
#define DEBUG_CIRCLE 4

#define DEBUG_WORLD 5
#define DEBUG_SCREEN 6

void InitializeDebug();

struct DebugWidget
{
	float timer;

	virtual void PushToBatch(SpriteBatch* spriteBatch, TextBatch* textBatch) = 0;
};

struct DebugIcon : DebugWidget
{
	u32 space;
	u32 icon;
	vec3 position;
	float size;
	vec4 color;

	DebugIcon(u32 space, u32 icon, vec3 position, float size, vec4 color, float timer)
	{
		this->space = space;
		this->icon = icon;
		this->position = position;
		this->size = size;
		this->color = color;
		this->timer = timer;
	}

	void PushToBatch(SpriteBatch* spriteBatch, TextBatch* textBatch) override;
};

struct DebugLine : DebugWidget
{
	u32 space;
	vec3 from;
	vec3 to;
	float thickness;
	vec4 color;

	DebugLine(u32 space, vec3 from, vec3 to, float thickness, vec4 color, float timer)
	{
		this->space = space;
		this->from = from;
		this->to = to;
		this->thickness = thickness;
		this->color = color;
		this->timer = timer;
	}

	void PushToBatch(SpriteBatch* spriteBatch, TextBatch* textBatch) override;
};

struct DebugText : DebugWidget
{
	u32 space;
	vec3 position;
	float size;
	vec4 color;
	std::string data;

	DebugText(u32 space, vec3 position, float size, vec4 color, std::string data, float timer)
	{
		this->space = space;
		this->position = position;
		this->size = size;
		this->color = color;
		this->data = data;
		this->timer = timer;
	}

	void PushToBatch(SpriteBatch* spriteBatch, TextBatch* textBatch) override;
};

void DrawDebugIcon(u32 space, u32 icon, vec3 position, float size, vec4 color, float timer = 0.f);
void DrawDebugLine(u32 space, vec3 from, vec3 to, float thickness, vec4 color, float timer = 0.f);
void DrawDebugText(u32 space, vec3 position, float size, vec4 color, std::string data, float timer = 0.f);
void DrawDebug(float dt);

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
typedef std::function<void(float)> InputEvent;

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
#define KEY_SPACE			30
#define KEY_ESCAPE			31
#define KEY_END				32

enum InputState
{
	PRESS, HOLD, RELEASE, UP
};

extern vec2 mousePosition;
extern vec3 mouseWorldPosition;

void InitializeInput(GLFWwindow* window);
u32 BindInputAction(u32 key, InputState state, InputEvent fun);
void UnbindInputAction(u32 key, InputState state, u32 id);
void UpdateInput(GLFWwindow* window, float dt);
InputState GetInputState(u32 key);

//UI
void InitializeUI();
void SetUICanvasSize(vec2 size);
void UpdateUI();
void DrawUI();

struct UINode;
extern UINode* canvas;

struct Font;

struct UIMouseEvent
{
	vec2 position;
	InputState state;
};

//TODO: Replace with immediate UI?
//TODO: Rethink Interface
//TODO: Margin
//TODO: Vertical/Horizontal Layout
//TODO: Slider
//TODO: Scroll Rect
//TODO: Radial Button

struct UINode
{
	bool dirty;
	UINode* parent;
	vec2 position, screenPosition;
	vec2 size;
	vec2 pivot;
	vec2 anchor;
	std::vector<struct UINode*> children;

	UINode(vec2 position, vec2 size)
		: UINode(canvas, position, size, vec2(0), vec2(0)) { }
	UINode(vec2 position, vec2 size, vec2 pivot, vec2 anchor)
		: UINode(canvas, position, size, pivot, anchor) { }
	UINode(UINode* parent, vec2 position, vec2 size)
		: UINode(parent, position, size, vec2(0), vec2(0)) { }
	UINode(UINode* parent, vec2 position, vec2 size, vec2 pivot, vec2 anchor)
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

	virtual void Step(vec2 parentPosition, vec2 parentSize, UIMouseEvent _mouseEvent);
	virtual void Draw();

	void NormalizeRect(vec2& nPos, vec2& nSize);
};

struct UIImage : UINode
{
	u32 spriteRow, spriteColumn;
	vec4 color;

	UIImage(vec2 position, vec2 size)
		: UIImage(canvas, position, size, vec2(0), vec2(0)) { }
	UIImage(vec2 position, vec2 size, vec2 pivot, vec2 anchor)
		: UIImage(canvas, position, size, pivot, anchor) { }
	UIImage(UINode* parent, vec2 position, vec2 size)
		: UIImage(parent, position, size, vec2(0), vec2(0)) { }
	UIImage(UINode* parent, vec2 position, vec2 size, vec2 pivot, vec2 anchor)
		: UINode(parent, position, size, pivot, anchor)
	{
		spriteRow = 0;
		spriteColumn = 0;
		color = vec4(1);
	}

	void Step(vec2 parentPosition, vec2 parentSize, UIMouseEvent _mouseEvent) override;
	void Draw() override;
};

struct UIText : UINode
{
	float fontSize;
	vec4 color;
	Font* font;
	vec2 alignment;
	std::string data;

	UIText(vec2 position, vec2 size)
		: UIText(canvas, position, size, vec2(0), vec2(0)) { }
	UIText(vec2 position, vec2 size, vec2 pivot, vec2 anchor)
		: UIText(canvas, position, size, pivot, anchor) { }
	UIText(UINode* parent, vec2 position, vec2 size)
		: UIText(parent, position, size, vec2(0), vec2(0)) { }
	UIText(UINode* parent, vec2 position, vec2 size, vec2 pivot, vec2 anchor)
		: UINode(parent, position, size, pivot, anchor)
	{
		fontSize = 1.f;
		color = vec4(1);
		font = nullptr;
		data = "TextTextText";
		alignment = pivot;
	}

	void Step(vec2 parentPosition, vec2 parentSize, UIMouseEvent _mouseEvent) override;
	void Draw() override;
};

enum ButtonState { RELEASED, HOVER, PRESSED, HELD };

struct UIButton : UINode
{
	ButtonState state;
	u32 spriteRow, spriteColumn;
	vec4 color;
	std::function<void(void)> onHover;
	std::function<void(void)> onPress;
	std::function<void(void)> onHold;
	std::function<void(void)> onRelease;

	UIButton(vec2 position, vec2 size)
		: UIButton(canvas, position, size, vec2(0), vec2(0)) { }
	UIButton(vec2 position, vec2 size, vec2 pivot, vec2 anchor)
		: UIButton(canvas, position, size, pivot, anchor) { }
	UIButton(UINode* parent, vec2 position, vec2 size)
		: UIButton(parent, position, size, vec2(0), vec2(0)) { }
	UIButton(UINode* parent, vec2 position, vec2 size, vec2 pivot, vec2 anchor)
		: UINode(parent, position, size, pivot, anchor)
	{
		state = RELEASED;
		onPress = nullptr;
		spriteRow = 0;
		spriteColumn = 0;
		color = vec4(1.f);
	}

	void Step(vec2 parentPosition, vec2 parentSize, UIMouseEvent _mouseEvent) override;
	void Draw() override;
};

