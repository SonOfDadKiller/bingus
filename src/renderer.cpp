#include "bingus.h"
#include <iostream>
#include <map>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

//using std::vector;

//TODO: Pull sprite ownership out of renderer
static vec2 cameraPosition = vec2(-1, -1);
static float cameraSize;
static u32 cameraUBO;
mat4 cameraProjection;
mat4 cameraView;
mat4 cameraViewProjInverse;

static FT_Library ft;
static const char* fontPath = "../res/fonts/";

Font* Fonts::arial = nullptr;

void InitializeRenderer()
{
	//Render settings
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDepthFunc(GL_LEQUAL);
	//glEnable(GL_DEPTH_TEST);
	glfwSwapInterval(0);

	//Initialize camera
	glGenBuffers(1, &cameraUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(mat4) * 2, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	SetCameraPosition(vec2(0));
	SetCameraSize(2);

	//Initialize text and fonts
	if (FT_Init_FreeType(&ft))
	{
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library\n";
		return;
	}

	//Load fonts
	Fonts::arial = LoadFont("arial.ttf", 40);
}

SpriteSheet::SpriteSheet(const char* texturePath, u32 rows, u32 columns)
{
	texture = LoadTexture(texturePath, GL_CLAMP_TO_EDGE);
	this->rows = rows;
	this->columns = columns;
}

void DeleteSprite(Sprite* sprite, std::vector<Sprite*>* container)
{
	auto it = container->begin();
	for (; it != container->end(); it++)
	{
		if (*it == sprite)
		{
			delete (*it);
			break;
		}
	}

	if (it != container->end()) container->erase(it);
}

void DeleteAllSprites(std::vector<Sprite*>* container)
{
	for (auto it = container->begin(); it != container->end(); it++)
	{
		delete (*it);
	}
	container->clear();
}

void DeleteText(Text* text, std::vector<Text*>* container)
{
	auto it = container->begin();
	for (; it != container->end(); it++)
	{
		if (*it == text)
		{
			delete (*it);
			break;
		}
	}

	if (it != container->end()) container->erase(it);
}

void DeleteAllTexts(std::vector<Text*>* container)
{
	for (auto it = container->begin(); it != container->end(); it++)
	{
		delete (*it);
	}
	container->clear();
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
	vertexTotalComponentCount = 0;
	vertexByteWidth = 0;
	for (int i = 0; i < this->attributes.size(); i++)
	{
		const VertAttrib& attrib = this->attributes[i];
		vertexTotalComponentCount += attrib.componentCount;
		vertexByteWidth += attrib.componentCount * attrib.componentWidth;
	}

	for (int i = 0; i < this->attributes.size(); i++)
	{
		const VertAttrib& attrib = this->attributes[i];
		glVertexAttribPointer(i, attrib.componentCount, attrib.type, GL_FALSE, vertexByteWidth, (void*)attrib.offset);
		glEnableVertexAttribArray(i);
	}

	//Unbind and return
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

i32 VertBuffer::GetAttributeOffset(u32 attribute)
{
	VertAttrib* attrib = GetAttribute(attribute);
	if (attrib == nullptr) return -1;
	return attrib->offset;
}

void RenderBatch::LazyInit()
{
	if (!initialized) Init();
	initialized = true;
}

void RenderBatch::GrowVertexCapacity(u32 capacity)
{
	if (capacity > vertexCapacity)
	{
		vertexCapacity = capacity;
		vertexData.resize(capacity * buffer.vertexTotalComponentCount);
	}
}

void RenderBatch::Clear()
{
	vertexCount = 0;
	vertexCapacity = 0;
	vertexData.clear();
	indices.clear();
}

void RenderBatch::SendVertsToGPUBuffer()
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), indices.data(), GL_DYNAMIC_DRAW);
	bufferDirty = false;
}

void RenderBatch::Draw()
{
	SetActiveShader(&shader);
	glBindVertexArray(buffer.vao);

	//Pass verts if necessary
	if (bufferDirty) SendVertsToGPUBuffer();

	if (shader.HasUniform(SHADER_MAIN_TEX))
	{
		//Pass texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		shader.SetUniformInt(SHADER_MAIN_TEX, 0);
	}

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
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
	spriteIndex = 0;

	//Get vertex attribute offsets
	positionAttrib = buffer.GetAttribute(VERTEX_POS);
	uvAttrib = buffer.GetAttribute(VERTEX_UV);
	colorAttrib = buffer.GetAttribute(VERTEX_COLOR);

	assert(positionAttrib != nullptr); //Position is required
}

void SpriteBatch::Clear()
{
	RenderBatch::Clear();
	spriteIndex = 0;
	bufferDirty = true;
}

void SpriteBatch::PushSprite(const Sprite& sprite)
{
	LazyInit();
	bufferDirty = true;

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

	//TODO: Rows
	float spriteUVWidth = 1.f / sheet->columns;

	//Push sprite data
	//TODO: Figure out when to grow verts... it should only happen here if not coming from PushSprites()
	GrowVertexCapacity(vertexCount + 4);
	vertexCount += 4;

	u32 baseOffset = spriteIndex * buffer.vertexTotalComponentCount * 4;

	assert(positionAttrib != nullptr);

	for (u32 vert = 0; vert < 4; vert++)
	{
		u32 vertOffset = baseOffset + vert * buffer.vertexTotalComponentCount;
		
		//Position
		u32 positionOffset = vertOffset + (positionAttrib->offset / positionAttrib->componentWidth);
		memcpy(vertexData.data() + positionOffset, &cornerPositions[vert], sizeof(vec3));
	}

	if (uvAttrib != nullptr)
	{
		vec2 cornerUVs[] = {
			vec2(sprite.column * spriteUVWidth, 0),
			vec2(sprite.column * spriteUVWidth, 1),
			vec2((sprite.column + 1) * spriteUVWidth, 1),
			vec2((sprite.column + 1) * spriteUVWidth, 0)
		};

		for (u32 vert = 0; vert < 4; vert++)
		{
			u32 vertOffset = baseOffset + vert * buffer.vertexTotalComponentCount;
			u32 uvOffset = vertOffset + (uvAttrib->offset / uvAttrib->componentWidth);
			memcpy(vertexData.data() + uvOffset, &cornerUVs[vert], sizeof(vec2));
		}
	}

	if (colorAttrib != nullptr)
	{
		for (u32 vert = 0; vert < 4; vert++)
		{
			u32 vertOffset = baseOffset + vert * buffer.vertexTotalComponentCount;
			u32 colorOffset = vertOffset + (colorAttrib->offset / colorAttrib->componentWidth);
			memcpy(vertexData.data() + colorOffset, &sprite.color, sizeof(vec4));
		}
	}

	//Indices
	u32 quadIndex = (spriteIndex++) * 4;
	indices.push_back(quadIndex);
	indices.push_back(quadIndex + 1);
	indices.push_back(quadIndex + 2);
	indices.push_back(quadIndex + 2);
	indices.push_back(quadIndex + 3);
	indices.push_back(quadIndex);
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

	for (Sprite sprite : sprites)
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

void TextBatch::Init()
{
	totalGlyphIndex = 0;

	//Get vertex attribute offsets
	positionAttrib = buffer.GetAttribute(VERTEX_POS);
	uvAttrib = buffer.GetAttribute(VERTEX_UV);
	colorAttrib = buffer.GetAttribute(VERTEX_COLOR);

	//Required attributes
	assert(positionAttrib != nullptr);
	assert(uvAttrib != nullptr);
}

void TextBatch::PushText(Text* text)
{
	//TODO: Make color a vertex attributes
	//textShader->SetUniformVec4(SHADER_COLOR, color);
	//TODO: Decide whether to enforce correct textures somehow
	//batch.texture = glyph.texture;

	LazyInit();
	bufferDirty = true;

	float actualSize = text->textSize / 1000;
	vec2 origin = vec2(0);

	vertexCount += text->data.size() * 4;
	GrowVertexCapacity(vertexCount);
	indices.reserve(indices.size() + text->data.size() * 6);

	//Create temporary array of rectangles, this is so we can replace things after the fact.
	std::vector<FontCharacterRect> glyphRects;
	glyphRects.reserve(text->data.size());

	vec2 extents = vec2(0);

	for (auto it = text->data.begin(); it != text->data.end(); it++)
	{
		FontCharacter* glyph = &text->font->characters[*it];

		vec2 glyphPos = (origin + vec2(glyph->bearing.x, glyph->bearing.y - glyph->size.y)) * actualSize * text->scale;
		vec2 glyphSize = vec2(glyph->size) * actualSize * text->scale;

		bool newLine = false;

		if (glyphPos.x + glyphSize.x >= text->extents.x)
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
			origin = vec2(0, origin.y - text->font->lineHeight);
			glyphPos = (origin + vec2(glyph->bearing.x, glyph->bearing.y - glyph->size.y)) * actualSize; //Recalc
		}

		//Step extents
		if (glyphPos.x + glyphSize.x > extents.x) extents.x = glyphPos.x + glyphSize.x;
		if (glyphPos.y + glyphSize.y > extents.y) extents.y = glyphPos.y + glyphSize.y;

		//Move glyph into world space
		glyphPos += vec2(text->position);

		origin.x += (glyph->advance >> 6); // bitshift by 6 to get value in pixels (2^6 = 64)

		glyphRects.push_back({ glyphPos, glyphSize, glyph });
	}

	vec2 offset = ((text->extents) - extents) * text->alignment;

	for (u32 glyphIndex = 0; glyphIndex < glyphRects.size(); glyphIndex++)
	{
		FontCharacterRect glyphRect = glyphRects[glyphIndex];
		vec2 pos = glyphRect.position + offset;

		vec3 positions[] = {
			vec3(pos, 0),
			vec3(pos.x, pos.y + glyphRect.size.y, 0),
			vec3(pos + glyphRect.size, 0),
			vec3(pos.x + glyphRect.size.x, pos.y, 0)
		};

		vec2 UVs[] = {
			vec2(glyphRect.character->uvMin.x, glyphRect.character->uvMax.y),
			glyphRect.character->uvMin,
			vec2(glyphRect.character->uvMax.x, glyphRect.character->uvMin.y),
			glyphRect.character->uvMax
		};

		
		u32 baseOffset = (totalGlyphIndex * 4 * buffer.vertexTotalComponentCount);

		for (u32 vertIndex = 0; vertIndex < 4; vertIndex++)
		{
			u32 vertOffset = baseOffset + vertIndex * buffer.vertexTotalComponentCount;
			u32 positionOffset = vertOffset + (positionAttrib->offset / positionAttrib->componentWidth);
			u32 uvOffset = vertOffset + (uvAttrib->offset / uvAttrib->componentWidth);
			memcpy(vertexData.data() + positionOffset, &positions[vertIndex], sizeof(vec3));
			memcpy(vertexData.data() + uvOffset, &UVs[vertIndex], sizeof(vec2));

			if (colorAttrib != nullptr)
			{
				u32 colorOffset = vertOffset + (colorAttrib->offset / colorAttrib->componentWidth);
				memcpy(vertexData.data() + colorOffset, &text->color, sizeof(vec4));
			}
		}

		u32 indexOffset = (totalGlyphIndex++) * 4;
		indices.push_back(indexOffset + 0);
		indices.push_back(indexOffset + 1);
		indices.push_back(indexOffset + 2);
		indices.push_back(indexOffset + 2);
		indices.push_back(indexOffset + 3);
		indices.push_back(indexOffset + 0);
	}
}



Font* LoadFont(const char* filePath, u32 pixelHeight)
{
	Font* font = new Font();
	font->lineHeight = pixelHeight;
	std::string fullPath = std::string(fontPath) + filePath;

	//Load TTF
	FT_Face face;
	if (FT_New_Face(ft, "../res/fonts/arial.ttf", 0, &face))
	{
		std::cout << "Loading font " << filePath << " failed!\n";
		return nullptr;
	}

	FT_Set_Pixel_Sizes(face, 0, pixelHeight); //0 width means automatic based on height

	//Determine atlas size
	int atlasWidth = 0;
	int atlasHeight = 0;

	for (FT_ULong c = 32; c < 128; c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			cout << "Loading character " << c << " of font" << filePath << " failed!\n";
			continue;
		}

		atlasWidth += face->glyph->bitmap.width;
		atlasHeight = glm::max(atlasHeight, (int)face->glyph->bitmap.rows);
	}

	//Create atlas texture
	u32 texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //disable byte-alignment restriction
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight,
		0, GL_RED, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	i32 x = 0;

	for (FT_ULong c = 32; c < 128; c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			continue;

		glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, face->glyph->bitmap.width, face->glyph->bitmap.rows,
			GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

		int bitmapWidth = face->glyph->bitmap.width;
		int bitmapHeight = face->glyph->bitmap.rows;

		float xMin = (float)x / (float)atlasWidth;
		float xMax = ((float)x + (float)bitmapWidth) / (float)atlasWidth;

		float yMin = 0.f;
		float yMax = (float)bitmapHeight / atlasHeight;

		x += face->glyph->bitmap.width;

		//Create character and store in map
		FontCharacter character;

		character.uvMin = vec2(xMin, 0);
		character.uvMax = vec2(xMax, yMax);
		character.size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
		character.bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
		character.advance = face->glyph->advance.x;

		font->characters.insert(std::pair<char, FontCharacter>(c, character));
	}

	font->texture = texture;

	FT_Done_Face(face);
	//FT_Done_FreeType(ft);

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
	size = glm::clamp(size, 2.f, 40.f);

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