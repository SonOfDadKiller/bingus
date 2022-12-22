#include "bingus.h"

static SpriteBatch spriteBatch;
static SpriteSheet spriteSheet;

static TextBatch textBatch;

static std::vector<DebugWidget*> widgets;

//TODO: Implement space var
void InitializeDebug()
{
	//Set up renderer
	spriteSheet = SpriteSheet("debug.png");
	spriteSheet.sequences["debug"] = SpriteSequence(vec2(0), vec2(128, 128), 4, 0.f);

	spriteBatch = SpriteBatch(VertBuffer({ VERTEX_POS, VERTEX_UV, VERTEX_COLOR }),
		Shader("world_vertcolor.vert", "sprite_vertcolor.frag", SHADER_MAIN_TEX),
		&spriteSheet);

	textBatch = TextBatch(VertBuffer({ VERTEX_POS, VERTEX_UV, VERTEX_COLOR }), 
		Shader("world_vertcolor.vert", "text_vertcolor.frag", SHADER_MAIN_TEX), 
		Fonts::arial);
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

DebugIcon::DebugIcon(u32 space, u32 icon, vec3 position, float size, vec4 color, float timer)
{
	this->space = space;
	this->icon = icon;
	this->position = position;
	this->size = size;
	this->color = color;
	this->timer = timer;
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

DebugLine::DebugLine(u32 space, vec3 from, vec3 to, float thickness, vec4 color, float timer)
{
	this->space = space;
	this->from = from;
	this->to = to;
	this->thickness = thickness;
	this->color = color;
	this->timer = timer;
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

DebugText::DebugText(u32 space, vec3 position, float size, vec4 color, std::string data, float timer)
{
	this->space = space;
	this->position = position;
	this->size = size;
	this->color = color;
	this->data = data;
	this->timer = timer;
}

void DebugText::PushToBatch(SpriteBatch* spriteBatch, TextBatch* textBatch)
{
	if (PointIntersectsCamera(vec2(position.x, position.y), size))
	{
		vec3 extents = vec3(10);
		textBatch->PushText(Text(data, position - extents / 2.f, vec2(extents), vec2(1.f), CENTER, size * 10.f, color, Fonts::arial));
	}
}

void DrawDebug(float dt)
{
	spriteBatch.Clear();
	textBatch.Clear();

	size_t i = 0;
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
