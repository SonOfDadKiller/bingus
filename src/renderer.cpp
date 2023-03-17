#include "bingus.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <string>
#include <iostream>
#include <map>
#include <algorithm>

RenderQueue globalRenderQueue;

//TODO: Pull sprite ownership out of renderer
static vec2 cameraPosition = vec2(-1, -1);
static float cameraSize;
static u32 cameraUBO;
mat4 cameraProjection;
mat4 cameraView;
mat4 cameraViewProjInverse;

static const char* fontPath = "../res/fonts/";
Font* Fonts::arial = nullptr;
Font* Fonts::linuxLibertine = nullptr;
void DebugPrintFontData(Font* font);

void InitializeRenderer()
{
	//Render settings
	//TODO: Make these externally accessible, perhaps when adding a platform independence layer
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glfwSwapInterval(0);

	//Initialize camera
	glGenBuffers(1, &cameraUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(mat4) * 2, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	SetCameraPosition(vec2(0));
	SetCameraSize(2);

	//Load fonts
	Fonts::arial = LoadFont("arial.ttf", 80);
	Fonts::linuxLibertine = LoadFont("linux_libertine.ttf", 80);

	//Initialize global render queue
	globalRenderQueue.buffer = VertBuffer({ VERTEX_POS, VERTEX_UV, VERTEX_COLOR });
	globalRenderQueue.spriteShader = Shader("ui_vertcolor.vert", "sprite_vertcolor.frag", SHADER_MAIN_TEX);
	globalRenderQueue.textShader = Shader("ui_vertcolor.vert", "text_vertcolor.frag", SHADER_MAIN_TEX);
	globalRenderQueue.spriteSheet = &defaultGuiSpritesheet;
	globalRenderQueue.font = Fonts::arial;
}

SpriteSequence::SpriteSequence(vec2 firstFramePosition, vec2 frameSize, u32 count, float spacing)
{
	for (u32 frameIndex = 0; frameIndex < count; frameIndex++)
	{
		SpriteSequenceFrame frame(Edges::None(),
			Rect(vec2(firstFramePosition.x + frameIndex * frameSize.x, firstFramePosition.y),
				 vec2(firstFramePosition.x + (frameIndex + 1) * frameSize.x, firstFramePosition.y + frameSize.y)));

		frames.push_back(frame);
	}
}

SpriteSequence::SpriteSequence(std::vector<SpriteSequenceFrame> frames)
{
	this->frames = frames;
}

SpriteSheet::SpriteSheet(const char* texturePath)
{
	texture = Texture(texturePath, GL_CLAMP_TO_EDGE, GL_LINEAR);
}

SpriteSheet::SpriteSheet(const char* texturePath, std::map<std::string, SpriteSequence> sequences)
{
	texture = Texture(texturePath, GL_CLAMP_TO_EDGE, GL_LINEAR);
	this->sequences = sequences;
}

SpriteAnimator::SpriteAnimator(SpriteSheet* sheet, std::string sequenceName, float speed)
{
	this->sheet = sheet;
	sequence = &this->sheet->sequences[sequenceName];
	this->timer = CreateTimer();
	this->timer->speed = speed;
}

u32 SpriteAnimator::GetFrame()
{
	return (u32)timer->timeElapsed % sequence->frames.size();
}

void SpriteAnimator::SetSequence(std::string name)
{
	sequence = &sheet->sequences[name];
	this->timer->Reset();
}

Sprite::Sprite(vec3 position, vec2 size, vec2 pivot, float rotation, Edges nineSliceMargin,
	vec4 color, SpriteSequence* sequence, u32 frame, SpriteAnimator* animator)
{
	this->position = position;
	this->size = size;
	this->pivot = pivot;
	this->nineSliceMargin = nineSliceMargin;
	this->rotation = rotation;
	this->color = color;
	this->sequence = sequence;
	this->sequenceFrame = frame;
	this->animator = animator;
}

Text::Text(std::string data, vec3 position, vec2 extents, vec2 scale, vec2 alignment, float textSize, vec4 color, Font* font)
{
	this->data = data;
	this->position = position;
	this->extents = extents;
	this->scale = scale;
	this->alignment = alignment;
	this->textSize = textSize;
	this->color = color;
	this->font = font;
}

VertAttrib::VertAttrib(u32 attribute, u32 componentCount, u32 componentWidth, u32 type, u32 offset)
{
	this->attribute = attribute;
	this->componentCount = componentCount;
	this->componentWidth = componentWidth;
	this->type = type;
	this->offset = offset;
}

VertBuffer::VertBuffer(const std::vector<u32> attributes)
{
	//Generate attributes
	u32 totalComponentCount = 0;
	for (u32 i = 0; i < attributes.size(); i++)
	{
		u32 componentCount = 0;

		switch (attributes[i])
		{
		case VERTEX_POS: componentCount = 3; break;
		case VERTEX_UV: componentCount = 2; break;
		case VERTEX_COLOR: componentCount = 4; break;
		}

		this->attributes.push_back(VertAttrib(attributes[i], componentCount, sizeof(float),
			GL_FLOAT, totalComponentCount * sizeof(float)));
		totalComponentCount += componentCount;
	}

	//Generate and bind buffers
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	//Set up vertex attributes
	vertexComponentCount = 0;
	vertexByteWidth = 0;

	for (auto it = this->attributes.begin(); it != this->attributes.end(); it++)
	{
		vertexComponentCount += it->componentCount;
		vertexByteWidth += it->componentCount * it->componentWidth;
	}

	u32 i = 0;
	for (auto it = this->attributes.begin(); it != this->attributes.end(); it++)
	{
		glVertexAttribPointer(i, it->componentCount, it->type, GL_FALSE, vertexByteWidth, (void*)it->offset);
		glEnableVertexAttribArray(i);
		i++;
	}

	//Unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

VertAttrib* VertBuffer::GetAttribute(u32 attribute)
{
	for (auto it = attributes.begin(); it != attributes.end(); it++)
	{
		if (it->attribute == attribute) return &(*it);
	}

	return nullptr;
}

u32 VertBuffer::GetAttributeOffset(u32 attribute)
{
	VertAttrib* attrib = GetAttribute(attribute);
	if (attrib == nullptr) return 0; //TODO: Implement better error return code
	return attrib->offset;
}

void VertBuffer::Destroy()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);

	attributes.clear();
	vertexComponentCount = 0;
	vertexByteWidth = 0;
}

void RenderBatch::LazyInit()
{
	//if (!initialized) Init();
	//initialized = true;
	Init();
}

void RenderBatch::GrowVertexCapacity(size_t capacity)
{
	if (capacity > vertexCapacity)
	{
		vertexCapacity = capacity;
		vertexData.resize(capacity * buffer.vertexComponentCount);
	}
}

void RenderBatch::Clear()
{
	vertexCount = 0;
	vertexCapacity = 0;
	vertexData.clear();
	indices.clear();
	bufferDirty = true;
}

void RenderBatch::SendVertsToGPUBuffer()
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), indices.data(), GL_DYNAMIC_DRAW);
	bufferDirty = false;
}

void RenderBatch::Draw()
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	if (vertexCount == 0) return;

	SetActiveShader(&shader);
	glBindVertexArray(buffer.vao);

	//Pass verts if necessary
	if (bufferDirty) SendVertsToGPUBuffer();

	if (shader.HasUniform(SHADER_MAIN_TEX))
	{
		//Pass texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture.id);
		shader.SetUniformInt(SHADER_MAIN_TEX, 0);
	}

	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
}

SpriteBatch::SpriteBatch(VertBuffer vertBuffer, Shader shader, SpriteSheet* spriteSheet)
{
	this->buffer = vertBuffer;
	this->shader = shader;
	this->sheet = spriteSheet;
	this->texture = spriteSheet->texture;
}

void SpriteBatch::Init()
{
	//Get vertex attribute offsets
	positionAttrib = buffer.GetAttribute(VERTEX_POS);
	uvAttrib = buffer.GetAttribute(VERTEX_UV);
	colorAttrib = buffer.GetAttribute(VERTEX_COLOR);

	assert(positionAttrib != nullptr); //Position is required
}

void SpriteBatch::Clear()
{
	RenderBatch::Clear();
}

void SpriteBatch::PushSprite(const Sprite& sprite)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	LazyInit();
	bufferDirty = true;

	if (sprite.nineSliceMargin.top != 0.f && sprite.nineSliceMargin.right != 0.f
		&& sprite.nineSliceMargin.bottom != 0.f && sprite.nineSliceMargin.left != 0.f)
	{
		PushSprite9Slice(sprite);
		return;
	}

	//Vertices
	vec3 cornerPositions[] = {
		vec3(0.f, 0.f, 0),
		vec3(0.f, sprite.size.y, 0),
		vec3(sprite.size.x, sprite.size.y, 0),
		vec3(sprite.size.x, 0.f, 0)
	};

	if (sprite.pivot != BOTTOM_LEFT)
	{
		vec2 offset2 = sprite.size * sprite.pivot;
		vec3 offset3 = vec3(offset2.x, offset2.y, 0.f);
		cornerPositions[0] -= offset3;
		cornerPositions[1] -= offset3;
		cornerPositions[2] -= offset3;
		cornerPositions[3] -= offset3;
	}

	if (sprite.rotation != 0.f)
	{
		glm::quat rotation = glm::angleAxis(glm::radians(sprite.rotation), vec3(0.f, 0.f, 1.f));

		cornerPositions[0] = rotation * cornerPositions[0];
		cornerPositions[1] = rotation * cornerPositions[1];
		cornerPositions[2] = rotation * cornerPositions[2];
		cornerPositions[3] = rotation * cornerPositions[3];
	}

	cornerPositions[0] += sprite.position;
	cornerPositions[1] += sprite.position;
	cornerPositions[2] += sprite.position;
	cornerPositions[3] += sprite.position;

	//Push sprite data
	//Push 4 empty verts onto the buffer, this is so we can use memcpy below.
	for (u32 vert = 0; vert < 4 * buffer.vertexComponentCount; vert++) vertexData.push_back(0.f);

	u32 baseOffset = vertexCount * buffer.vertexComponentCount;

	for (u32 vert = 0; vert < 4; vert++)
	{
		u32 vertOffset = baseOffset + vert * buffer.vertexComponentCount;
		
		//Position
		u32 positionOffset = vertOffset + (positionAttrib->offset / positionAttrib->componentWidth);
		memcpy(vertexData.data() + positionOffset, &cornerPositions[vert], sizeof(vec3));
	}

	if (uvAttrib != nullptr)
	{
		//Get sequence from animator, or fall back to sprite.sequence and sprite.frame
		SpriteSequence* sequence = nullptr;
		u32 frame;

		if (sprite.animator != nullptr)
		{
			sequence = sprite.animator->sequence;
			frame = sprite.animator->GetFrame();
		}
		else if (sprite.sequence != nullptr)
		{
			sequence = sprite.sequence;
			frame = sprite.sequenceFrame;
		}

		//Calculate UVs
		vec2 uvMin = vec2(0);
		vec2 uvMax = vec2(1);
		
		if (sequence != nullptr)
		{
			Rect frameRect = sequence->frames[frame].rect;
			uvMin = frameRect.min / texture.size;
			uvMax = frameRect.max / texture.size;
		}

		vec2 cornerUVs[] = {
			uvMin,
			vec2(uvMin.x, uvMax.y),
			uvMax,
			vec2(uvMax.x, uvMin.y)
		};

		for (u32 vert = 0; vert < 4; vert++)
		{
			u32 vertOffset = baseOffset + vert * buffer.vertexComponentCount;
			u32 uvOffset = vertOffset + (uvAttrib->offset / uvAttrib->componentWidth);
			memcpy(vertexData.data() + uvOffset, &cornerUVs[vert], sizeof(vec2));
		}
	}

	if (colorAttrib != nullptr)
	{
		for (u32 vert = 0; vert < 4; vert++)
		{
			u32 vertOffset = baseOffset + vert * buffer.vertexComponentCount;
			u32 colorOffset = vertOffset + (colorAttrib->offset / colorAttrib->componentWidth);
			memcpy(vertexData.data() + colorOffset, &sprite.color, sizeof(vec4));
		}
	}

	//Indices
	indices.push_back(vertexCount);
	indices.push_back(vertexCount + 1);
	indices.push_back(vertexCount + 2);
	indices.push_back(vertexCount + 2);
	indices.push_back(vertexCount + 3);
	indices.push_back(vertexCount);

	//Update vertex count, this is kinda important
	vertexCount += 4;
}

void SpriteBatch::PushSprite9Slice(const Sprite& sprite)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	//Get frame early, so we can use the 9 slice data
	SpriteSequence* sequence = nullptr;
	SpriteSequenceFrame* frame = nullptr;

	if (sprite.animator != nullptr)
	{
		sequence = sprite.animator->sequence;
		frame = &sequence->frames[sprite.animator->GetFrame()];
	}
	else if (sprite.sequence != nullptr)
	{
		sequence = sprite.sequence;
		frame = &sequence->frames[sprite.sequenceFrame];
	}

	assert(frame != nullptr);

	//Vertices
	vec3 vertPositions[] = {
		//First row
		vec3(0,												0, 0),
		vec3(sprite.nineSliceMargin.left,					0, 0),
		vec3(sprite.size.x - sprite.nineSliceMargin.right,	0, 0),
		vec3(sprite.size.x,									0, 0),
		//Second row
		vec3(0,												sprite.nineSliceMargin.top, 0),
		vec3(sprite.nineSliceMargin.left,					sprite.nineSliceMargin.top, 0),
		vec3(sprite.size.x - sprite.nineSliceMargin.right,	sprite.nineSliceMargin.top, 0),
		vec3(sprite.size.x,									sprite.nineSliceMargin.top, 0),
		//Third row
		vec3(0,												sprite.size.y - sprite.nineSliceMargin.bottom, 0),
		vec3(sprite.nineSliceMargin.left,					sprite.size.y - sprite.nineSliceMargin.bottom, 0),
		vec3(sprite.size.x - sprite.nineSliceMargin.right,	sprite.size.y - sprite.nineSliceMargin.bottom, 0),
		vec3(sprite.size.x,									sprite.size.y - sprite.nineSliceMargin.bottom, 0),
		//Fourth row
		vec3(0,												sprite.size.y, 0),
		vec3(sprite.nineSliceMargin.left,					sprite.size.y, 0),
		vec3(sprite.size.x - sprite.nineSliceMargin.right,	sprite.size.y, 0),
		vec3(sprite.size.x,									sprite.size.y, 0)
	};

	if (sprite.pivot != BOTTOM_LEFT)
	{
		vec2 offset2 = sprite.size * sprite.pivot;
		vec3 offset3 = vec3(offset2, 0.f);

		//This feels dumb...
		for (u32 vert = 0; vert < 16; vert++) vertPositions[vert] -= offset3;
	}

	if (sprite.rotation != 0.f)
	{
		glm::quat rotation = glm::angleAxis(glm::radians(sprite.rotation), vec3(0.f, 0.f, 1.f));
		for (u32 vert = 0; vert < 16; vert++) vertPositions[vert] = rotation * vertPositions[vert];
	}

	for (u32 vert = 0; vert < 16; vert++) vertPositions[vert] += sprite.position;

	//Push sprite data
	//Push 4 empty verts onto the buffer, this is so we can use memcpy below.
	for (u32 vert = 0; vert < 16 * buffer.vertexComponentCount; vert++) vertexData.push_back(0.f);

	u32 baseOffset = vertexCount * buffer.vertexComponentCount;

	for (u32 vert = 0; vert < 16; vert++)
	{
		u32 vertOffset = baseOffset + vert * buffer.vertexComponentCount;

		//Position
		u32 positionOffset = vertOffset + (positionAttrib->offset / positionAttrib->componentWidth);
		memcpy(vertexData.data() + positionOffset, &vertPositions[vert], sizeof(vec3));
	}

	//NOTE: UVs should probably be non-optional, as theres no point not having them when using a 9 slice

	if (uvAttrib != nullptr)
	{
		//Calculate UVs
		vec2 uvMin = vec2(0);
		vec2 uvMax = vec2(1);
		Edges scaledUVEdges = Edges(frame->nineSliceSample.top / texture.size.y,
									frame->nineSliceSample.right / texture.size.x,
									frame->nineSliceSample.bottom / texture.size.y,
									frame->nineSliceSample.left / texture.size.x);

		if (frame != nullptr)
		{
			uvMin = frame->rect.min / texture.size;
			uvMax = frame->rect.max / texture.size;
		}

		//First row
		vec2 vertUVs[16];
		vertUVs[0] = uvMin;
		vertUVs[1] = vec2(uvMin.x + scaledUVEdges.left, uvMin.y);
		vertUVs[2] = vec2(uvMax.x - scaledUVEdges.right, uvMin.y);
		vertUVs[3] = vec2(uvMax.x, uvMin.y);
		//Second row
		vertUVs[4] = vec2(uvMin.x, uvMin.y + scaledUVEdges.top);
		vertUVs[5] = vec2(uvMin.x + scaledUVEdges.left, uvMin.y + scaledUVEdges.top);
		vertUVs[6] = vec2(uvMax.x - scaledUVEdges.right, uvMin.y + scaledUVEdges.top);
		vertUVs[7] = vec2(uvMax.x, uvMin.y + scaledUVEdges.top);
		//Third row
		vertUVs[8] = vec2(uvMin.x, uvMax.y - scaledUVEdges.bottom);
		vertUVs[9] = vec2(uvMin.x + scaledUVEdges.left, uvMax.y - scaledUVEdges.bottom);
		vertUVs[10] = vec2(uvMax.x - scaledUVEdges.right, uvMax.y - scaledUVEdges.bottom);
		vertUVs[11] = vec2(uvMax.x, uvMax.y - scaledUVEdges.bottom);
		//Fourth row
		vertUVs[12] = vec2(uvMin.x, uvMax.y);
		vertUVs[13] = vec2(uvMin.x + scaledUVEdges.left, uvMax.y);
		vertUVs[14] = vec2(uvMax.x - scaledUVEdges.right, uvMax.y);
		vertUVs[15] = uvMax;

		for (u32 vert = 0; vert < 16; vert++)
		{
			u32 vertOffset = baseOffset + vert * buffer.vertexComponentCount;
			u32 uvOffset = vertOffset + (uvAttrib->offset / uvAttrib->componentWidth);
			memcpy(vertexData.data() + uvOffset, &vertUVs[vert], sizeof(vec2));
		}
	}

	if (colorAttrib != nullptr)
	{
		for (u32 vert = 0; vert < 16; vert++)
		{
			u32 vertOffset = baseOffset + vert * buffer.vertexComponentCount;
			u32 colorOffset = vertOffset + (colorAttrib->offset / colorAttrib->componentWidth);
			memcpy(vertexData.data() + colorOffset, &sprite.color, sizeof(vec4));
		}
	}

	//Indices
	for (size_t quadY = 0; quadY < 3; quadY++)
	{
		for (size_t quadX = 0; quadX < 3; quadX++)
		{
			size_t offset = vertexCount + quadX + quadY * 4;
			indices.push_back(offset);
			indices.push_back(offset + 1);
			indices.push_back(offset + 5);
			indices.push_back(offset + 5);
			indices.push_back(offset + 4);
			indices.push_back(offset);
		}
	}

	//Update vertex count, this is kinda important
	vertexCount += 16;
}

void SpriteBatch::PushSprites(const std::vector<Sprite*>& sprites)
{
	GrowVertexCapacity(vertexCount + sprites.size() * 4);
	indices.reserve(indices.size() + sprites.size() * 6);

	for (Sprite* sprite : sprites)
	{
		PushSprite(*sprite);
	}
}

void SpriteBatch::PushSprites(const std::vector<Sprite>& sprites)
{
	GrowVertexCapacity(vertexCount + sprites.size() * 4);
	indices.reserve(indices.size() + sprites.size() * 6);

	for (const Sprite& sprite : sprites)
	{
		PushSprite(sprite);
	}
}

void SpriteBatch::PushSpritesReverse(const std::vector<Sprite*>& sprites)
{
	GrowVertexCapacity(vertexCount + sprites.size() * 4);
	indices.reserve(indices.size() + sprites.size() * 6);

	for (auto it = sprites.rbegin(); it != sprites.rend(); it++)
	{
		PushSprite(**it);
	}
}

void SpriteBatch::PushSpritesReverse(const std::vector<Sprite>& sprites)
{
	GrowVertexCapacity(vertexCount + sprites.size() * 4);
	indices.reserve(indices.size() + sprites.size() * 6);

	for (auto it = sprites.rbegin(); it != sprites.rend(); it++)
	{
		PushSprite(*it);
	}
}

TextBatch::TextBatch(VertBuffer buffer, Shader shader, Font* font)
{
	this->buffer = buffer;
	this->shader = shader;
	this->font = font;
	this->texture = font->texture;
}

void TextBatch::Init()
{
	//Get vertex attribute offsets
	positionAttrib = buffer.GetAttribute(VERTEX_POS);
	uvAttrib = buffer.GetAttribute(VERTEX_UV);
	colorAttrib = buffer.GetAttribute(VERTEX_COLOR);

	//Required attributes
	assert(positionAttrib != nullptr);
	assert(uvAttrib != nullptr);
}

void TextBatch::Clear()
{
	RenderBatch::Clear();
}

void TextBatch::PushText(const Text& text)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	if (text.data == "") return;

	LazyInit();
	bufferDirty = true;

	float actualSize = text.textSize / 1000;
	vec2 origin = vec2(0);

	GrowVertexCapacity(vertexCount + text.data.size() * 4);
	indices.reserve(indices.size() + text.data.size() * 6);

	//Create temporary array of rectangles, this is so we can replace things after the fact.
	std::vector<FontCharacterRect> glyphRects;
	glyphRects.reserve(text.data.size());

	vec2 extents = vec2(0);

	for (auto it = text.data.begin(); it != text.data.end(); it++)
	{
		FontCharacter* glyph = &text.font->characters[*it];

		vec2 glyphPos = (origin + vec2(glyph->bearing.x, glyph->bearing.y - glyph->size.y)) * actualSize * text.scale;
		vec2 glyphSize = vec2(glyph->size) * actualSize * text.scale;

		//TODO: Implement smarter new-lining that doesn't split words
		bool newLine = false;

		if (glyphPos.x + glyphSize.x >= text.extents.x)
		{
			newLine = true;
		}
		else if (*it == '\n')
		{
			newLine = true;
		}

		if (newLine)
		{
			//New line, move origin back to start
			origin = vec2(0, origin.y - (float)text.font->lineHeight);
			glyphPos = (origin + vec2(glyph->bearing.x, glyph->bearing.y - glyph->size.y)) * actualSize; //Recalc
		}

		//Step extents
		if (glyphPos.x + glyphSize.x > extents.x) extents.x = glyphPos.x + glyphSize.x;
		if (glyphPos.y + glyphSize.y > extents.y) extents.y = glyphPos.y + glyphSize.y;

		//Move glyph into world space
		glyphPos += vec2(text.position.x, text.position.y);

		origin.x += glyph->advance;

		glyphRects.push_back({ glyphPos, glyphSize, glyph });
	}

	vec2 offset = ((text.extents) - extents) * text.alignment;

	for (const FontCharacterRect& glyphRect : glyphRects)
	{
		vec2 pos = glyphRect.position + offset;

		vec3 positions[] = {
			vec3(pos, text.position.z),
			vec3(pos.x, pos.y + glyphRect.size.y, text.position.z),
			vec3(pos + glyphRect.size, text.position.z),
			vec3(pos.x + glyphRect.size.x, pos.y, text.position.z)
		};

		vec2 UVs[] = {
			vec2(glyphRect.character->uvMin.x, glyphRect.character->uvMax.y),
			glyphRect.character->uvMin,
			vec2(glyphRect.character->uvMax.x, glyphRect.character->uvMin.y),
			glyphRect.character->uvMax
		};

		u32 baseOffset = (vertexCount * buffer.vertexComponentCount);

		for (u32 vertIndex = 0; vertIndex < 4; vertIndex++)
		{
			u32 vertOffset = baseOffset + vertIndex * buffer.vertexComponentCount;
			u32 positionOffset = vertOffset + (positionAttrib->offset / positionAttrib->componentWidth);
			u32 uvOffset = vertOffset + (uvAttrib->offset / uvAttrib->componentWidth);
			memcpy(vertexData.data() + positionOffset, &positions[vertIndex], sizeof(vec3));
			memcpy(vertexData.data() + uvOffset, &UVs[vertIndex], sizeof(vec2));

			if (colorAttrib != nullptr)
			{
				u32 colorOffset = vertOffset + (colorAttrib->offset / colorAttrib->componentWidth);
				memcpy(vertexData.data() + colorOffset, &text.color, sizeof(vec4));
			}
		}

		indices.push_back(vertexCount + 0);
		indices.push_back(vertexCount + 1);
		indices.push_back(vertexCount + 2);
		indices.push_back(vertexCount + 2);
		indices.push_back(vertexCount + 3);
		indices.push_back(vertexCount + 0);

		vertexCount += 4;
	}
}

void RenderQueue::Clear()
{
	//Clear batches that were used last frame
	for (int i = 0; i < currentSpriteBatchIndex; i++)
	{
		spriteBatches[i].Clear();
	}

	for (int i = 0; i < currentTextBatchIndex; i++)
	{
		textBatches[i].Clear();
	}

	currentSpriteBatchIndex = 0;
	currentTextBatchIndex = 0;
	stepIndex = 0;
	steps.clear();
}

void RenderQueue::PushStep()
{
	steps.push_back(Step());
	stepIndex++;
}

void RenderQueue::PushStep(void(*preDraw)(), void(*postDraw)())
{
	Step step;
	step.preDraw = preDraw;
	step.postDraw = postDraw;
	steps.push_back(step);
	stepIndex++;
}

void RenderQueue::PushSprite(const Sprite& sprite)
{
	u32 spriteBatchIndex;

	//Create new steps if the index has grown past the current size
	if ((size_t)stepIndex + 1 > steps.size())
	{
		for (u32 diff = (stepIndex + 1) - steps.size(); diff != 0; diff--)
		{
			steps.push_back(Step());
		}
	}

	if (steps[stepIndex].spriteBatchIndex != -1)
	{
		//Sprite batch for this step already exists
		spriteBatchIndex = steps[stepIndex].spriteBatchIndex;
	}
	else
	{
		//Request/create sprite batch for this step
		if ((size_t)currentSpriteBatchIndex + 1 > spriteBatches.size())
		{
			spriteBatches.push_back(SpriteBatch(buffer, spriteShader, spriteSheet));
		}

		spriteBatchIndex = currentSpriteBatchIndex;
		steps[stepIndex].spriteBatchIndex = spriteBatchIndex;
		currentSpriteBatchIndex++;
	}

	spriteBatches[spriteBatchIndex].PushSprite(sprite);
}

void RenderQueue::PushText(const Text& text)
{
	u32 textBatchIndex;

	//Create new steps if the index has grown past the current size
	if ((size_t)stepIndex + 1 > steps.size())
	{
		for (u32 diff = (stepIndex + 1) - steps.size(); diff != 0; diff--)
		{
			steps.push_back(Step());
		}
	}

	if (steps[stepIndex].textBatchIndex != -1)
	{
		//Sprite batch for this step already exists
		textBatchIndex = steps[stepIndex].textBatchIndex;
	}
	else
	{
		//Request/create sprite batch for this step
		if ((size_t)currentTextBatchIndex + 1 > textBatches.size())
		{
			textBatches.push_back(TextBatch(buffer, textShader, font));
		}

		textBatchIndex = currentTextBatchIndex;
		steps[stepIndex].textBatchIndex = textBatchIndex;
		currentTextBatchIndex++;
	}

	textBatches[textBatchIndex].PushText(text);
}

void RenderQueue::Draw()
{
	//Iterate through steps in queue, triggering events and drawing their contents. Sprites draw before text.
	for (u32 i = 0; i < steps.size(); i++)
	{
		if (steps[i].preDraw != nullptr) steps[i].preDraw();

		i32 sbIndex = steps[i].spriteBatchIndex;
		if (sbIndex != -1)
		{
			spriteBatches[sbIndex].Draw();
		}

		i32 tbIndex = steps[i].textBatchIndex;
		if (tbIndex != -1)
		{
			textBatches[tbIndex].Draw();
		}

		if (steps[i].postDraw != nullptr) steps[i].postDraw();
	}
}

//TODO: Move this into a different file? Or move texture and shader loading into this file? Questions indeed...
Font* LoadFont(const char* filepath, u32 pixelHeight)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	std::string fullPath = std::string(fontPath) + filepath;

	//Load .ttf file
	long size;
	unsigned char* fontBuffer;
	FILE* fontFile = fopen(fullPath.c_str(), "rb"); //Open file
	fseek(fontFile, 0, SEEK_END); //Seek to end
	size = ftell(fontFile); //Get length
	fseek(fontFile, 0, SEEK_SET); //Seek back to start
	fontBuffer = new unsigned char[(size_t)size]; //Allocate buffer
	fread(fontBuffer, (size_t)size, 1, fontFile); //Read file into buffer
	fclose(fontFile); //Close file

	//Create font
	stbtt_fontinfo fontInfo;
	if (stbtt_InitFont(&fontInfo, fontBuffer, 0) == 0)
	{
		std::cout << "Loading font @" << fullPath << " failed!\n";
		delete[] fontBuffer;
		return nullptr;
	}

	stbtt_pack_context packContext;

	const u32 unicodeCharStart = 32;
	const u32 unicodeCharEnd = 127;
	const u32 unicodeCharRange = unicodeCharEnd - unicodeCharStart;

	const u32 atlasWidth = 1024;
	const u32 atlasHeight = 1024;
	const u32 atlasSize = atlasWidth * atlasHeight;
	stbtt_packedchar packedChars[unicodeCharRange];
	unsigned char* pixelBuffer = new unsigned char[atlasSize];

	if (stbtt_PackBegin(&packContext, pixelBuffer, atlasWidth, atlasHeight, 0, 1, 0) == 0)
	{
		std::cout << "Packing font @" << fullPath << " failed!\n";
		delete[] fontBuffer;
		delete[] pixelBuffer;
		return nullptr;
	}

	stbtt_PackSetOversampling(&packContext, 2, 2);
	stbtt_PackFontRange(&packContext, fontBuffer, 0, (float)pixelHeight, unicodeCharStart, unicodeCharRange, packedChars);
	stbtt_PackEnd(&packContext);

	//Create atlas texture
	u32 textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //disable byte-alignment restriction
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pixelBuffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	Font* font = new Font();
	font->lineHeight = pixelHeight;
	font->texture = Texture(textureID, vec2(atlasWidth, atlasHeight));

	for (i32 c = unicodeCharStart; c < unicodeCharEnd; c++)
	{
		const stbtt_packedchar& packedChar = packedChars[c - unicodeCharStart];

		//Create character and store in map
		FontCharacter character;
		character.uvMin = vec2((float)packedChar.x0 / (float)atlasWidth, (float)packedChar.y0 / (float)atlasHeight);
		character.uvMax = vec2((float)packedChar.x1 / (float)atlasWidth, (float)packedChar.y1 / (float)atlasHeight);
		character.size = vec2(packedChar.xoff2 - packedChar.xoff, packedChar.yoff2 - packedChar.yoff);
		character.bearing = vec2(packedChar.xoff, 1.f - packedChar.yoff);
		character.advance = packedChar.xadvance;

		font->characters.insert(std::pair<i32, FontCharacter>(c, character));
	}

	std::cout << "Successfully loaded font : @" << fullPath << "\n";

	delete[] fontBuffer;
	delete[] pixelBuffer;

	return font;
}

void SetCameraPosition(vec2 position, bool forceUpdateUBO)
{
	if (forceUpdateUBO || cameraPosition != position)
	{
		cameraPosition = position;

		//Step view matrix
		vec3 pos = vec3(position.x, position.y, 1);
		cameraView = glm::lookAt(pos, pos + vec3(0, 0, -1), vec3(0, 1, 0));
		glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), glm::value_ptr(cameraView));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		cameraViewProjInverse = glm::inverse(cameraProjection * cameraView);
	}
}

void SetCameraSize(float size, bool forceUpdateUBO)
{
	if (forceUpdateUBO || cameraSize != size)
	{
		cameraSize = size;

		//Step projection matrix
		float actualCameraSize = cameraSize / GetWindowSize().y;
		float halfWidth = GetWindowSize().x * actualCameraSize * 0.5f;
		float halfHeight = GetWindowSize().y * actualCameraSize * 0.5f;
		cameraProjection = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight);
		glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), glm::value_ptr(cameraProjection));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		cameraViewProjInverse = glm::inverse(cameraProjection * cameraView);
	}
}

void TranslateCamera(vec2 translation)
{
	SetCameraPosition(GetCameraPosition() + translation);
}

void ZoomCamera(float sizeChange)
{
	SetCameraSize(cameraSize + sizeChange);
}

vec2 GetCameraPosition()
{
	return cameraPosition;
}

float GetCameraSize()
{
	return cameraSize;
}

bool PointIntersectsCamera(vec2 position, float buffer)
{
	float actualCameraSize = cameraSize / GetWindowSize().y;
	float halfWidth = GetWindowSize().x * actualCameraSize * 0.5f;
	float halfHeight = GetWindowSize().y * actualCameraSize * 0.5f;

	float left = cameraPosition.x - halfWidth - buffer;
	float right = cameraPosition.x + halfWidth + buffer;
	float top = cameraPosition.y + halfHeight + buffer;
	float bottom = cameraPosition.y - halfHeight - buffer;

	return position.x > left && position.x < right && position.y > bottom && position.y < top;
}

void DebugPrintFontData(Font* font)
{
	std::cout << "DEBUG PRINT FONT:\n\n";

	for (auto it = font->characters.begin(); it != font->characters.end(); it++)
	{
		std::cout << it->first;
		std::cout << ":  bearing=(" << std::to_string(it->second.bearing.x) << "," << std::to_string(it->second.bearing.x) << ")	";
		std::cout << "size=(" << std::to_string(it->second.size.x) << "," << std::to_string(it->second.size.x) << ")	";
		std::cout << "advance=" << std::to_string(it->second.advance) << "\n";
	}

	std::cout << "\n";
}

