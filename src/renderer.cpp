#include "bingus.h"
#include <iostream>
#include <map>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <string>

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
Font* LoadFontSTB(const char* filepath, u32 pixelHeight);
void DebugPrintFontData(Font* font);

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

	//Load fonts
	Fonts::arial = LoadFont("arial.ttf", 80);
	Fonts::linuxLibertine = LoadFont("linux_libertine.ttf", 80);
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

	for (auto it = this->attributes.begin(); it != this->attributes.end(); it++)
	{
		vertexTotalComponentCount += it->componentCount;
		vertexByteWidth += it->componentCount * it->componentWidth;
	}

	u32 i = 0;
	for (auto it = this->attributes.begin(); it != this->attributes.end(); it++)
	{
		glVertexAttribPointer(i, it->componentCount, it->type, GL_FALSE, vertexByteWidth, (void*)it->offset);
		glEnableVertexAttribArray(i);
		i++;
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

u32 VertBuffer::GetAttributeOffset(u32 attribute)
{
	VertAttrib* attrib = GetAttribute(attribute);
	if (attrib == nullptr) return 0; //TODO: Implement better error return code
	return attrib->offset;
}

void RenderBatch::LazyInit()
{
	if (!initialized) Init();
	initialized = true;
}

void RenderBatch::GrowVertexCapacity(size_t capacity)
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
	bufferDirty = true;
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
	float spriteUVWidth = 1.f / (float)sheet->columns;

	//Push sprite data
	//TODO: Figure out when to grow verts... it should only happen here if not coming from PushSprites()
	GrowVertexCapacity((size_t)vertexCount + 4);
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
		float fColumn = (float)sprite.column;
		vec2 cornerUVs[] = {
			vec2(fColumn * spriteUVWidth, 0.f),
			vec2(fColumn * spriteUVWidth, 1.f),
			vec2((fColumn + 1.f) * spriteUVWidth, 1.f),
			vec2((fColumn + 1.f) * spriteUVWidth, 0.f)
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
	glyphIndex = 0;

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
	glyphIndex = 0;
}

void TextBatch::PushText(const Text& text)
{
	//TODO: Make color a vertex attributes
	//textShader->SetUniformVec4(SHADER_COLOR, color);
	//TODO: Decide whether to enforce correct textures somehow
	//batch.texture = glyph.texture;

	LazyInit();
	bufferDirty = true;

	float actualSize = text.textSize / 1000;
	vec2 origin = vec2(0);

	vertexCount += text.data.size() * 4;
	GrowVertexCapacity(vertexCount);
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
		glyphPos += vec2(text.position);

		origin.x += glyph->advance;

		glyphRects.push_back({ glyphPos, glyphSize, glyph });
	}

	vec2 offset = ((text.extents) - extents) * text.alignment;

	for (const FontCharacterRect& glyphRect : glyphRects)
	{
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

		u32 baseOffset = (glyphIndex * 4 * buffer.vertexTotalComponentCount);

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
				memcpy(vertexData.data() + colorOffset, &text.color, sizeof(vec4));
			}
		}

		u32 indexOffset = (glyphIndex++) * 4;
		indices.push_back(indexOffset + 0);
		indices.push_back(indexOffset + 1);
		indices.push_back(indexOffset + 2);
		indices.push_back(indexOffset + 2);
		indices.push_back(indexOffset + 3);
		indices.push_back(indexOffset + 0);
	}
}

Font* LoadFont(const char* filepath, u32 pixelHeight)
{
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
	u32 texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //disable byte-alignment restriction
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pixelBuffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	Font* font = new Font();
	font->lineHeight = pixelHeight;
	font->texture = texture;

	for (i32 c = unicodeCharStart; c < unicodeCharEnd; c++)
	{
		const stbtt_packedchar& packedChar = packedChars[c - unicodeCharStart];

		//Create character and store in map
		FontCharacter character;

		float xpos;
		float ypos;
		stbtt_aligned_quad quad;
		stbtt_GetPackedQuad(packedChars, atlasWidth, atlasHeight, c, &xpos, &ypos, &quad, 0);

		character.uvMin = vec2((float)packedChar.x0 / (float)atlasWidth, (float)packedChar.y0 / (float)atlasHeight);
		character.uvMax = vec2((float)packedChar.x1 / (float)atlasWidth, (float)packedChar.y1 / (float)atlasHeight);

		character.size = vec2(packedChar.xoff2 - packedChar.xoff, packedChar.yoff2 - packedChar.yoff);

		character.bearing = vec2(packedChar.xoff, 1.f - packedChar.yoff);
		character.advance = packedChar.xadvance;

		font->characters.insert(std::pair<i32, FontCharacter>(c, character));
	}

	std::cout << "Successfully loaded font : @" << fullPath << "!\n";

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

