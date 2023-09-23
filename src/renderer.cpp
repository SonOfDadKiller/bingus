#include "bingus.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <iostream>
#include <map>
#include <algorithm>

RenderQueue globalRenderQueue;

static vec2 cameraPosition = vec2(-1, -1);
static float cameraSize;
static AABB cameraExtents;
static u32 cameraUBO;
mat4 cameraProjection;
mat4 cameraView;
mat4 cameraViewProj;
mat4 cameraViewProjInverse;

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

	//Load engine fonts
	LoadFont("arial.ttf", 80);
	LoadFont("linux_libertine.ttf", 80);

	//TODO: Load other enegine resources as above?

	//Initialize global render queue
	//globalRenderQueue.buffer = new VertBuffer(POS_UV_COLOR);
	globalRenderQueue.spriteShader = LoadShader("world_vertcolor.vert", "sprite_vertcolor.frag");
	globalRenderQueue.spriteShader->EnableUniforms(SHADER_MAIN_TEX);
	globalRenderQueue.textShader = LoadShader("world_vertcolor.vert", "text_vertcolor.frag");
	globalRenderQueue.textShader->EnableUniforms(SHADER_MAIN_TEX);
	globalRenderQueue.spriteSheet = nullptr; //TODO: Change this?
	globalRenderQueue.font = LoadFont("arial.ttf", 80);
}

// .d88888b  dP                      dP                   
// 88.    "' 88                      88                   
// `Y88888b. 88d888b. .d8888b. .d888b88 .d8888b. 88d888b. 
//       `8b 88'  `88 88'  `88 88'  `88 88ooood8 88'  `88 
// d8'   .8P 88    88 88.  .88 88.  .88 88.  ... 88       
//  Y88888P  dP    dP `88888P8 `88888P8 `88888P' dP       

u32 activeShaderID;

void SetActiveShader(Shader* shader)
{
	assert(shader != nullptr);

	if (shader->id != activeShaderID)
	{
		glUseProgram(shader->id);
		activeShaderID = shader->id;
	}
}

// dP     dP                     dP                     
// 88     88                     88                     
// 88    .8P .d8888b. 88d888b. d8888P .d8888b. dP.  .dP 
// 88    d8' 88ooood8 88'  `88   88   88ooood8  `8bd8'  
// 88  .d8P  88.  ... 88         88   88.  ...  .d88b.  
// 888888'   `88888P' dP         dP   `88888P' dP'  `dP

VertBuffer::VertBuffer(VertexType vertexType)
{
	this->vertexType = vertexType;
	vertexCount = 0;

	//Generate and bind buffers
	glGenVertexArrays(1, &vao);
	glBindVertexArray(this->vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	//Set up vertex attributes
	if (vertexType == POS_COLOR)
	{
		vertexSize = sizeof(vec3) + sizeof(vec4);
		bufferData = posColorVerts.data();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, vertexSize, (void*)sizeof(vec3));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	}
	else if (vertexType == POS_UV)
	{
		vertexSize = sizeof(vec3) + sizeof(vec2);
		bufferData = posUVVerts.data();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertexSize, (void*)sizeof(vec3));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	}
	else if (vertexType == POS_UV_COLOR)
	{
		vertexSize = sizeof(vec3) + sizeof(vec2) + sizeof(vec4);
		bufferData = posUVColorVerts.data();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertexSize, (void*)sizeof(vec3));
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, vertexSize, (void*)(sizeof(vec3) + sizeof(vec2)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
	}

	//Unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void VertBuffer::Clear()
{
	posColorVerts.clear();
	posUVVerts.clear();
	posUVColorVerts.clear();
	vertexIndices.clear();
	dirty = true;
	vertexCount = 0;
}

void VertBuffer::Destroy()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);
}

//  888888ba             dP            dP      
//  88    `8b            88            88      
//  88aaaa8P' .d8888b. d8888P .d8888b. 88d888b.
//  88   `8b. 88'  `88   88   88'  `"" 88'  `88
//  88    .88 88.  .88   88   88.  ... 88    88
//  88888888P `88888P8   dP   `88888P' dP    dP

void RenderBatch::Draw()
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	if (buffer->vertexCount == 0) return;

	SetActiveShader(shader);
	glBindVertexArray(buffer->vao);

	//Pass verts if necessary
	if (buffer->dirty)
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);

		void* bufferData;
		if (buffer->vertexType == POS_COLOR)
		{
			bufferData = buffer->posColorVerts.data();
		}
		else if (buffer->vertexType == POS_UV)
		{
			bufferData = buffer->posUVVerts.data();
		}
		else if (buffer->vertexType == POS_UV_COLOR)
		{
			bufferData = buffer->posUVColorVerts.data();
		}
	
		glBufferData(GL_ARRAY_BUFFER, buffer->vertexCount * buffer->vertexSize, bufferData, GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer->vertexIndices.size() * sizeof(u32), buffer->vertexIndices.data(), GL_DYNAMIC_DRAW);
		buffer->dirty = false;
	}

	if (shader->HasUniform(SHADER_MAIN_TEX))
	{
		//Pass texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture->id);
		shader->SetUniformInt(SHADER_MAIN_TEX, 0);
	}

	glDrawElements(drawMode, (GLsizei)buffer->vertexIndices.size(), GL_UNSIGNED_INT, 0);
}

// .d88888b                    oo   dP            
// 88.    "'                        88            
// `Y88888b. 88d888b. 88d888b. dP d8888P .d8888b. 
//       `8b 88'  `88 88'  `88 88   88   88ooood8 
// d8'   .8P 88.  .88 88       88   88   88.  ... 
//  Y88888P  88Y888P' dP       dP   dP   `88888P' 
//           88                                   
//           dP

SpriteSequence::SpriteSequence(vec2 firstFramePosition, vec2 frameSize, u32 count, float spacing)
{
	for (u32 frameIndex = 0; frameIndex < count; frameIndex++)
	{
		SpriteSequenceFrame frame(Edges::Zero(),
			Rect(vec2(firstFramePosition.x + frameIndex * frameSize.x, firstFramePosition.y),
				 vec2(firstFramePosition.x + (frameIndex + 1) * frameSize.x, firstFramePosition.y + frameSize.y)));

		frames.push_back(frame);
	}
}

SpriteSequence::SpriteSequence(std::vector<SpriteSequenceFrame> frames)
{
	this->frames = frames;
}

SpriteSheet::SpriteSheet(Texture* texture)
{
	this->texture = texture;
}

SpriteSheet::SpriteSheet(Texture* texture, std::map<std::string, SpriteSequence> sequences)
{
	this->texture = texture;
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
	if ((u32)timer->timeElapsed == 0 || sequence->frames.size() == 0) return 0;
	return (u32)timer->timeElapsed % sequence->frames.size();
}

void SpriteAnimator::SetSequence(std::string name)
{
	sequence = &sheet->sequences[name];
	this->timer->Reset();
}

SpriteBatch::SpriteBatch(VertBuffer* vertBuffer, Shader* shader, SpriteSheet* spriteSheet)
{
	this->buffer = vertBuffer;
	this->shader = shader;
	this->sheet = spriteSheet;
	this->texture = spriteSheet->texture;
}

void SpriteBatch::PushSprite(const Sprite& sprite)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	buffer->dirty = true;
	
	//Switch to 9 Slice function if the margin is not zero
	if (sprite.nineSliceMargin.top != 0.f && sprite.nineSliceMargin.right != 0.f
		&& sprite.nineSliceMargin.bottom != 0.f && sprite.nineSliceMargin.left != 0.f)
	{
		PushSprite9Slice(sprite);
		return;
	}

	//Positions are calculated for each vertex type
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

	//Here we switch on different vertex types
	if (buffer->vertexType == POS_COLOR)
	{
		//POS_COLOR can just push the positions which are already calculated, and get the color from the sprite
		for (int i = 0; i < 4; i++)
		{
			buffer->posColorVerts.push_back({ cornerPositions[i], sprite.color });
		}
	}
	else if (buffer->vertexType == POS_UV || buffer->vertexType == POS_UV_COLOR)
	{
		//POS_UV needs to get the spritesheet data. 
		SpriteSequence* sequence = nullptr;
		u32 frame;

		//Prefer using animator over sequence if it is set
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
			uvMin = frameRect.min / texture->size;
			uvMax = frameRect.max / texture->size;
		}

		vec2 cornerUVs[] = {
			uvMin,
			vec2(uvMin.x, uvMax.y),
			uvMax,
			vec2(uvMax.x, uvMin.y)
		};

		//Switch on vertex type again to optionally insert color
		if (buffer->vertexType == POS_UV_COLOR)
		{
			for (int i = 0; i < 4; i++)
			{
				buffer->posUVColorVerts.push_back({ cornerPositions[i], cornerUVs[i], sprite.color });
			}
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				buffer->posUVVerts.push_back({ cornerPositions[i], cornerUVs[i] });
			}
		}
	}

	//Indices
	buffer->vertexIndices.push_back(buffer->vertexCount);
	buffer->vertexIndices.push_back(buffer->vertexCount + 1);
	buffer->vertexIndices.push_back(buffer->vertexCount + 2);
	buffer->vertexIndices.push_back(buffer->vertexCount + 2);
	buffer->vertexIndices.push_back(buffer->vertexCount + 3);
	buffer->vertexIndices.push_back(buffer->vertexCount);

	buffer->vertexCount += 4; //This is kinda important
}

void SpriteBatch::PushSprite9Slice(const Sprite& sprite)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	 //UV channel is required for 9 slice sprites
	assert(buffer->vertexType != POS_COLOR);

	//We should only come here from PushSprite()
	assert(sprite.nineSliceMargin.top != 0.f && sprite.nineSliceMargin.right != 0.f
		&& sprite.nineSliceMargin.bottom != 0.f && sprite.nineSliceMargin.left != 0.f); 

	buffer->dirty = true;

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

	//Calculate UVs
	vec2 uvMin = vec2(0);
	vec2 uvMax = vec2(1);
	Edges scaledUVEdges = Edges(frame->nineSliceSample.top / texture->size.y,
								frame->nineSliceSample.right / texture->size.x,
								frame->nineSliceSample.bottom / texture->size.y,
								frame->nineSliceSample.left / texture->size.x);

	if (frame != nullptr)
	{
		uvMin = frame->rect.min / texture->size;
		uvMax = frame->rect.max / texture->size;
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

	if (buffer->vertexType == POS_UV_COLOR)
	{
		for (int i = 0; i < 16; i++)
		{
			buffer->posUVColorVerts.push_back({ vertPositions[i], vertUVs[i], sprite.color });
		}
	}
	else
	{
		for (int i = 0; i < 16; i++)
		{
			buffer->posUVVerts.push_back({ vertPositions[i], vertUVs[i] });
		}
	}

	//Indices
	for (size_t quadY = 0; quadY < 3; quadY++)
	{
		for (size_t quadX = 0; quadX < 3; quadX++)
		{
			size_t offset = buffer->vertexCount + quadX + quadY * 4;
			buffer->vertexIndices.push_back(offset);
			buffer->vertexIndices.push_back(offset + 1);
			buffer->vertexIndices.push_back(offset + 5);
			buffer->vertexIndices.push_back(offset + 5);
			buffer->vertexIndices.push_back(offset + 4);
			buffer->vertexIndices.push_back(offset);
		}
	}

	buffer->vertexCount += 16; //This is kinda important
}

// d888888P                     dP   
//    88                        88   
//    88    .d8888b. dP.  .dP d8888P 
//    88    88ooood8  `8bd8'    88   
//    88    88.  ...  .d88b.    88   
//    dP    `88888P' dP'  `dP   dP

TextBatch::TextBatch(VertBuffer* buffer, Shader* shader, Font* font)
{
	this->buffer = buffer;
	this->shader = shader;
	this->font = font;
	this->texture = &font->texture;
}

void TextBatch::PushText(const Text& text)
{
	PushText(text, TextRenderInfo());
}

void TextBatch::PushText(const Text& text, TextRenderInfo& info)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	//UV channel is required for text
	assert(buffer->vertexType != POS_COLOR);

	if (text.data == "") return;

	buffer->dirty = true;

	vec2 actualSize = text.scale * text.textSize;
	vec2 origin = vec2(0); //Origin represents the rolling glyph position before it is scaled

	//Create temporary array of rectangles, this is so we can replace things after the fact.
	std::vector<FontCharacterRect> glyphRects;
	glyphRects.reserve(text.data.size());

	//TODO: Make extents y based on line height not glyph height
	vec2 extents = vec2(0);
	//info.caretPositionOffsets = std::vector<vec2>(1, vec2(0));
	info.lineHeightInPixels = text.textSize;
	info.renderScale = actualSize * text.scale;

	//First we loop through the characters and find their local coordinates, this is so
	//we can calculate the extents of the rendered text to later shift the coordinates
	//around the pivot point
	u32 index = 0;
	for (auto it = text.data.begin(); it != text.data.end(); it++)
	{
		FontCharacter* glyph = &text.font->characters[*it];

		vec2 glyphPos = (origin + vec2(glyph->bearing.x, glyph->bearing.y - glyph->size.y)) * actualSize;
		vec2 glyphSize = glyph->size * actualSize;

		

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
			info.lineEndCharacters.push_back(index - 1);
			info.lineEndOffsets.push_back(origin * actualSize);

			//Move origin to new line position
			origin.x = 0.f;
			origin.y -= 1.f;
			glyphPos = (origin + vec2(glyph->bearing.x, glyph->bearing.y - glyph->size.y)) * actualSize; //Copy of glyphpos calc above
		}

		info.caretPositionOffsets.push_back(origin * actualSize);

		//Step extents
		if (glyphPos.x + glyphSize.x > extents.x) extents.x = glyphPos.x + glyphSize.x;
		if (glyphPos.y + glyphSize.y > extents.y) extents.y = glyphPos.y + glyphSize.y;

		//Move glyph into world space
		glyphPos += vec2(text.position.x, text.position.y);

		origin.x += glyph->advance;
		//info.caretPositionOffsets.push_back(origin * actualSize);

		glyphRects.push_back({ glyphPos, glyphSize, glyph });
		
		index++;
	}

	info.caretPositionOffsets.push_back(origin * actualSize);

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

		if (buffer->vertexType == POS_UV_COLOR)
		{
			for (int i = 0; i < 4; i++)
			{
				buffer->posUVColorVerts.push_back({ positions[i], UVs[i], text.color });
			}
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				buffer->posUVVerts.push_back({ positions[i], UVs[i] });
			}
		}

		buffer->vertexIndices.push_back(buffer->vertexCount + 0);
		buffer->vertexIndices.push_back(buffer->vertexCount + 1);
		buffer->vertexIndices.push_back(buffer->vertexCount + 2);
		buffer->vertexIndices.push_back(buffer->vertexCount + 2);
		buffer->vertexIndices.push_back(buffer->vertexCount + 3);
		buffer->vertexIndices.push_back(buffer->vertexCount + 0);

		buffer->vertexCount += 4;
	}
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

//  888888ba                          dP                       .88888.                                       
//  88    `8b                         88                      d8'   `8b                                      
//  88aaaa8P' .d8888b. 88d888b. .d888b88 .d8888b. 88d888b.    88     88  dP    dP .d8888b. dP    dP .d8888b. 
//  88   `8b. 88ooood8 88'  `88 88'  `88 88ooood8 88'  `88    88  db 88  88    88 88ooood8 88    88 88ooood8 
//  88     88 88.  ... 88    88 88.  .88 88.  ... 88          Y8.  Y88P  88.  .88 88.  ... 88.  .88 88.  ... 
//  dP     dP `88888P' dP    dP `88888P8 `88888P' dP           `8888PY8b `88888P' `88888P' `88888P' `88888P'

void RenderQueue::Clear()
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	//Clear batches that were used last frame
	for (int i = 0; i < currentSpriteBatchIndex; i++)
	{
		spriteBatches[i].buffer->Clear();
	}

	for (int i = 0; i < currentTextBatchIndex; i++)
	{
		textBatches[i].buffer->Clear();
	}

	currentSpriteBatchIndex = 0;
	currentTextBatchIndex = 0;
	stepIndex = 0;
	steps.clear();
}

void RenderQueue::AddStep()
{
	steps.push_back(Step());
	stepIndex++;
}

void RenderQueue::AddStep(void(*preDraw)(), void(*postDraw)())
{
	Step step;
	step.preDraw = preDraw;
	step.postDraw = postDraw;
	steps.push_back(step);
	stepIndex++;
}

void RenderQueue::PushSprite(const Sprite& sprite)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

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
			SpriteBatch batch;
			batch.buffer = new VertBuffer(POS_UV_COLOR);
			batch.shader = spriteShader;
			batch.sheet = spriteSheet;
			batch.texture = batch.sheet->texture;
			spriteBatches.push_back(batch);
		}

		spriteBatchIndex = currentSpriteBatchIndex;
		steps[stepIndex].spriteBatchIndex = spriteBatchIndex;
		currentSpriteBatchIndex++;
	}

	spriteBatches[spriteBatchIndex].PushSprite(sprite);
}

void RenderQueue::PushText(const Text& text)
{
	PushText(text, TextRenderInfo());
}

void RenderQueue::PushText(const Text& text, TextRenderInfo& info)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

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
			TextBatch batch;
			batch.buffer = new VertBuffer(POS_UV_COLOR);
			batch.shader = textShader;
			batch.font = font;
			batch.texture = &batch.font->texture;
			textBatches.push_back(batch);
		}

		textBatchIndex = currentTextBatchIndex;
		steps[stepIndex].textBatchIndex = textBatchIndex;
		currentTextBatchIndex++;
	}

	textBatches[textBatchIndex].PushText(text, info);
}

void RenderQueue::Draw()
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

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

//  a88888b.                                                
// d8'   `88                                                
// 88        .d8888b. 88d8b.d8b. .d8888b. 88d888b. .d8888b. 
// 88        88'  `88 88'`88'`88 88ooood8 88'  `88 88'  `88 
// Y8.   .88 88.  .88 88  88  88 88.  ... 88       88.  .88 
//  Y88888P' `88888P8 dP  dP  dP `88888P' dP       `88888P8 

void SetCameraPosition(vec2 position, bool forceUpdateUBO)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	if (forceUpdateUBO || cameraPosition != position)
	{
		cameraPosition = position;

		float aspectRatio = GetWindowSize().x / GetWindowSize().y;
		vec2 halfSize = vec2((cameraSize * aspectRatio), cameraSize) / 2.f;
		cameraExtents.min = cameraPosition - halfSize;
		cameraExtents.max = cameraPosition + halfSize;

		//Step view matrix
		vec3 pos = vec3(position.x, position.y, 1);
		cameraView = glm::lookAt(pos, pos + vec3(0, 0, -1), vec3(0, 1, 0));
		glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), glm::value_ptr(cameraView));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		 
		cameraViewProj = cameraProjection * cameraView;
		cameraViewProjInverse = glm::inverse(cameraViewProj);
	}
}

void SetCameraSize(float size, bool forceUpdateUBO)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	if (forceUpdateUBO || cameraSize != size)
	{
		cameraSize = size;

		//Step projection matrix
		float actualCameraSize = cameraSize / GetWindowSize().y;

		float aspectRatio = GetWindowSize().x / GetWindowSize().y;
		vec2 halfSize = vec2((cameraSize * aspectRatio), cameraSize) / 2.f;
		cameraExtents.min = cameraPosition - halfSize;
		cameraExtents.max = cameraPosition + halfSize;

		float halfWidth = GetWindowSize().x * actualCameraSize * 0.5f;
		float halfHeight = GetWindowSize().y * actualCameraSize * 0.5f;
		cameraProjection = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight);
		glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), glm::value_ptr(cameraProjection));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		cameraViewProj = cameraProjection * cameraView;
		cameraViewProjInverse = glm::inverse(cameraViewProj);
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

AABB GetCameraExents()
{
	return cameraExtents;
}

bool PointIntersectsCamera(vec2 position, float buffer)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	float actualCameraSize = cameraSize / GetWindowSize().y;
	float halfWidth = GetWindowSize().x * actualCameraSize * 0.5f;
	float halfHeight = GetWindowSize().y * actualCameraSize * 0.5f;

	float left = cameraPosition.x - halfWidth - buffer;
	float right = cameraPosition.x + halfWidth + buffer;
	float top = cameraPosition.y + halfHeight + buffer;
	float bottom = cameraPosition.y - halfHeight - buffer;

	return position.x > left && position.x < right && position.y > bottom && position.y < top;
}
