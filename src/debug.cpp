#pragma once

#include "bingus.h"

static VertBuffer spriteBuffer;
static Shader spriteShader;
static SpriteSheet spriteSheet;

static VertBuffer textBuffer;
static Shader textShader;

static std::vector<DebugWidget*> widgets;

//TODO: Implement space var
void InitializeDebug()
{
	//Set up renderer
	spriteBuffer = VertBuffer({ VERTEX_POS, VERTEX_UV, VERTEX_COLOR });

	spriteSheet = SpriteSheet("debug.png", 1, 7);
	spriteShader = Shader("world_vertcolor.vert", "sprite_vertcolor.frag");
	spriteShader.EnableUniforms(SHADER_MAIN_TEX);

	textBuffer = VertBuffer({ VERTEX_POS, VERTEX_UV, VERTEX_COLOR });
	textShader = Shader("world_vertcolor.vert", "text_vertcolor.frag");
	textShader.EnableUniforms(SHADER_MAIN_TEX);
}

void DrawDebugIcon(u32 space, u32 icon, vec3 position, float size, vec4 color, float timer)
{
	widgets.push_back(new DebugIcon(space, icon, position, size, color, timer));
}

void DrawDebugLine(u32 space, vec3 from, vec3 to, float thickness, vec4 color, float timer)
{
	widgets.push_back(new DebugLine(space, from, to, thickness, color, timer));
}

void DrawDebugText(u32 space, vec3 position, float size, vec4 color, std::string data, float timer)
{
	widgets.push_back(new DebugText(space, position, size, color, data, timer));
}

void DebugIcon::PushToBatch(SpriteBatch* spriteBatch, TextBatch* textBatch)
{
	if (PointIntersectsCamera(vec2(position.x, position.y), size))
	{
		Sprite sprite = Sprite(position, vec2(size), 0, icon + 1);
		sprite.color = color;
		sprite.pivot = CENTER;
		spriteBatch->PushSprite(sprite);
	}
}

void DebugLine::PushToBatch(SpriteBatch* spriteBatch, TextBatch* textBatch)
{
	//WARNING: Bad hack - need to implement line-rect intersection test
	if (PointIntersectsCamera(vec2(from.x, from.y), 2.f) && PointIntersectsCamera(vec2(to.x, to.y), 2.f))
	{
		vec3 diff = to - from;
		Sprite sprite = Sprite(from, vec2(glm::length(diff), thickness), 0, 0);
		sprite.color = color;
		sprite.pivot = CENTER_LEFT;
		sprite.rotation = glm::degrees(atan2(diff.y, diff.x));
		spriteBatch->PushSprite(sprite);
	}
}

void DebugText::PushToBatch(SpriteBatch* spriteBatch, TextBatch* textBatch)
{
	if (PointIntersectsCamera(vec2(position.x, position.y), size))
	{
		vec3 extents = vec3(10);
		Text text = Text(data, position - extents / 2.f, vec2(extents), vec2(1.f), CENTER, size * 10.f, color, Fonts::arial);
		textBatch->PushText(&text);
	}
}

void DrawDebug(float dt)
{
	SpriteBatch spriteBatch;
	spriteBatch.buffer = spriteBuffer;
	spriteBatch.shader = spriteShader;
	spriteBatch.sheet = &spriteSheet;
	spriteBatch.texture = spriteSheet.texture;
	spriteBatch.Init();

	TextBatch textBatch;
	textBatch.buffer = textBuffer;
	textBatch.shader = textShader;
	textBatch.texture = Fonts::arial->texture;
	textBatch.Init();

	int i = 0;
	while (i != widgets.size())
	{
		widgets[i]->timer -= dt;
		widgets[i]->PushToBatch(&spriteBatch, &textBatch);

		if (widgets[i]->timer <= 0.f)
		{
			delete widgets[i];
			widgets.erase(widgets.begin() + i);
		}
		else
		{
			i++;
		}
	}

	spriteBatch.Draw();
	textBatch.Draw();
}
