#include "bingus.h"

static TextBatch textBatch;

static RenderBatch lineBatch;
static RenderBatch polyBatch;

struct DebugLine
{
	vec4 color;
	vec3 start, end;
	float timer;
};

struct DebugAABB
{
	vec4 color;
	AABB aabb;
	float timer;
	bool fill;
};

struct DebugCircle
{
	vec4 color;
	Circle circle;
	u32 pointCount;
	float timer;
	bool fill;
};

struct DebugText
{
	vec4 color;
	vec3 position;
	std::string data;
	float size;
	float timer;
};

std::vector<DebugLine> lines;
std::vector<DebugAABB> aabbs;
std::vector<DebugCircle> circles;
std::vector<DebugText> texts;

//TODO: Implement space var
void InitializeDebug()
{
	//Set up renderer

	//TODO: Figure out if I really want to store the buffer in heap memory like this
	textBatch.buffer = new VertBuffer(POS_UV_COLOR);
	textBatch.shader = LoadShader("world_vertcolor.vert", "text_vertcolor.frag");
	textBatch.shader->EnableUniforms(SHADER_MAIN_TEX);
	textBatch.font = LoadFont("arial.ttf", 80);
	textBatch.texture = &textBatch.font->texture;

	//Shader shapeShader = 

	lineBatch.shader = LoadShader("world_shape.vert", "shape.frag");
	lineBatch.buffer = new VertBuffer(POS_COLOR);
	lineBatch.drawMode = GL_LINES;

	polyBatch.shader = LoadShader("world_shape.vert", "shape.frag");
	polyBatch.buffer = new VertBuffer(POS_COLOR);
	polyBatch.drawMode = GL_TRIANGLES;

	glLineWidth(3.f);
	glEnable(GL_LINE_SMOOTH);
}

void DrawDebugIcon(u32 space, u32 icon, vec3 position, float size, vec4 color, float timer)
{
	
}

void DrawDebugLine(u32 space, vec3 from, vec3 to, float thickness, vec4 color, float timer)
{
	DebugLine d_line;
	d_line.start = from;
	d_line.end = to;
	d_line.color = color;
	d_line.timer = timer;
	lines.push_back(d_line);
}

void DrawDebugLine(u32 space, vec2 from, vec2 to, float thickness, vec4 color, float timer)
{
	DebugLine d_line;
	d_line.start = vec3(from, 0);
	d_line.end = vec3(to, 0);
	d_line.color = color;
	d_line.timer = timer;
	lines.push_back(d_line);
}

void DrawDebugAABB(u32 space, const AABB& aabb, vec4 color, bool fill, float timer)
{
	DebugAABB d_aabb;
	d_aabb.aabb = aabb;
	d_aabb.color = color;
	d_aabb.fill = fill;
	d_aabb.timer = timer;
	aabbs.push_back(d_aabb);
}

void DrawDebugCircle(u32 space, const Circle& circle, u32 pointCount, vec4 color, bool fill, float timer)
{
	DebugCircle d_circle;
	d_circle.circle = circle;
	d_circle.pointCount = pointCount;
	d_circle.color = color;
	d_circle.fill = fill;
	d_circle.timer = timer;
	circles.push_back(d_circle);
}

void DrawDebugText(u32 space, vec3 position, float size, vec4 color, std::string data, float timer)
{
	DebugText text;
	text.position = position;
	text.size = size;
	text.color = color;
	text.data = data;
	text.timer = timer;
	texts.push_back(text);
}

void DrawDebugText(u32 space, vec2 position, float size, vec4 color, std::string data, float timer)
{
	DrawDebugText(space, vec3(position, 0.f), size, color, data, timer);
}

void PushLineToBatch(vec3 start, vec3 end, vec4 color)
{
	lineBatch.buffer->posColorVerts.push_back(Vertex_PosColor(start, color));
	lineBatch.buffer->posColorVerts.push_back(Vertex_PosColor(end, color));
	lineBatch.buffer->vertexIndices.push_back(lineBatch.buffer->vertexCount);
	lineBatch.buffer->vertexIndices.push_back(lineBatch.buffer->vertexCount + 1);
	lineBatch.buffer->vertexCount += 2;
	lineBatch.buffer->dirty = true;
}

void PushTriToBatch(vec3 a, vec3 b, vec3 c, vec4 color)
{

}

void PushAABBToBatch(AABB box, vec4 color, bool fill)
{
	if (fill)
	{

	}
	else
	{
		PushLineToBatch(vec3(box.min, 0.f),				 vec3(box.max.x, box.min.y, 0.f), color);
		PushLineToBatch(vec3(box.max.x, box.min.y, 0.f), vec3(box.max, 0.f), color);
		PushLineToBatch(vec3(box.max, 0.f),				 vec3(box.min.x, box.max.y, 0.f), color);
		PushLineToBatch(vec3(box.min.x, box.max.y, 0.f), vec3(box.min, 0.f), color);
	}
}

void PushCircleToBatch(Circle circle, vec4 color, u32 pointCount, bool fill)
{
	if (fill)
	{
		
	}
	else
	{
		vec3 lastPos = vec3(circle.position, 0) + vec3(0, circle.radius, 0); //sin(0), cos(0)
		for (int i = 0; i < pointCount; i++)
		{
			float c = (float)(i + 1) / pointCount;
			c = 3.1415926f * c * 2.f;
			vec3 pos = vec3(circle.position, 0) + vec3(sin(c) * circle.radius, cos(c) * circle.radius, 0.f);
			PushLineToBatch(lastPos, pos, color);
			lastPos = pos;
		}
	}
}

void DrawDebug(float dt)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	lineBatch.buffer->Clear();
	polyBatch.buffer->Clear();
	textBatch.buffer->Clear();

	//Process lines
	auto line_it = lines.begin();
	while (line_it != lines.end())
	{
		PushLineToBatch(line_it->start, line_it->end, line_it->color);

		//Update timer, remove if needed
		line_it->timer -= dt;
		if (line_it->timer <= 0) line_it = lines.erase(line_it);
		else line_it++;
	}

	//Process aabbs
	auto aabb_it = aabbs.begin();
	while (aabb_it != aabbs.end())
	{
		PushAABBToBatch(aabb_it->aabb, aabb_it->color, aabb_it->fill);

		//Update timer, remove if needed
		aabb_it->timer -= dt;
		if (aabb_it->timer <= 0) aabb_it = aabbs.erase(aabb_it);
		else aabb_it++;
	}

	//Process circles
	auto circle_it = circles.begin();
	while (circle_it != circles.end())
	{
		PushCircleToBatch(circle_it->circle, circle_it->color, circle_it->pointCount, circle_it->fill);

		//Update timer, remove if needed
		circle_it->timer -= dt;
		if (circle_it->timer <= 0) circle_it = circles.erase(circle_it);
		else circle_it++;
	}

	//Process text
	auto texts_it = texts.begin();
	while (texts_it != texts.end())
	{
		//Add text
		Text text;
		text.data = texts_it->data;
		text.position = texts_it->position - vec3(5.f, 5.f, 0.f);
		text.extents = vec2(10.f);
		text.alignment = CENTER;
		text.textSize = texts_it->size;
		text.color = texts_it->color;
		text.font = textBatch.font;
		textBatch.PushText(text);

		//Update timer, remove if needed
		texts_it->timer -= dt;
		if (texts_it->timer <= 0) texts_it = texts.erase(texts_it);
		else texts_it++;
	}

	//Draw text over lines over polys
	polyBatch.Draw();
	lineBatch.Draw();
	textBatch.Draw();
}
