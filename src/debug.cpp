#include "bingus.h"

static TextBatch textBatchWorld;
static RenderBatch lineBatchWorld;
static RenderBatch polyBatchWorld;

static TextBatch textBatchScreen;
static RenderBatch lineBatchScreen;
static RenderBatch polyBatchScreen;

struct DebugLine
{
	vec4 color;
	vec3 start, end;
	float timer;
	u32 space;
};

struct DebugAABB
{
	vec4 color;
	AABB aabb;
	float timer;
	bool fill;
	u32 space;
};

struct DebugCircle
{
	vec4 color;
	Circle circle;
	u32 pointCount;
	float timer;
	bool fill;
	u32 space;
};

struct DebugPolygon
{
	vec4 color;
	Polygon polygon;
	float timer;
	bool fill;
	u32 space;
};

struct DebugText
{
	vec4 color;
	vec3 position;
	std::string data;
	float size;
	float timer;
	u32 space;
};

std::vector<DebugLine> lines;
std::vector<DebugAABB> aabbs;
std::vector<DebugCircle> circles;
std::vector<DebugPolygon> polygons;
std::vector<DebugText> texts;

//TODO: Implement space var
void InitializeDebug()
{
	//Set up renderer
	//TODO: Figure out if I really want to store the buffer in heap memory like this
	textBatchWorld.buffer = new VertBuffer(POS_UV_COLOR);
	textBatchWorld.shader = LoadShader("world_vertcolor.vert", "text_vertcolor.frag");
	textBatchWorld.shader->EnableUniforms(SHADER_MAIN_TEX);
	textBatchWorld.font = LoadFont("arial.ttf", 80);
	textBatchWorld.texture = &textBatchWorld.font->texture;

	textBatchScreen.buffer = new VertBuffer(POS_UV_COLOR);
	textBatchScreen.shader = LoadShader("ui_vertcolor.vert", "text_vertcolor.frag");
	textBatchScreen.shader->EnableUniforms(SHADER_MAIN_TEX);
	textBatchScreen.font = LoadFont("arial.ttf", 80);
	textBatchScreen.texture = &textBatchWorld.font->texture;

	lineBatchWorld.shader = LoadShader("world_shape.vert", "shape.frag");
	lineBatchWorld.buffer = new VertBuffer(POS_COLOR);
	lineBatchWorld.drawMode = GL_LINES;

	lineBatchScreen.shader = LoadShader("ui_shape.vert", "shape.frag");
	lineBatchScreen.buffer = new VertBuffer(POS_COLOR);
	lineBatchScreen.drawMode = GL_LINES;

	polyBatchWorld.shader = LoadShader("world_shape.vert", "shape.frag");
	polyBatchWorld.buffer = new VertBuffer(POS_COLOR);
	polyBatchWorld.drawMode = GL_TRIANGLES;

	polyBatchScreen.shader = LoadShader("ui_shape.vert", "shape.frag");
	polyBatchScreen.buffer = new VertBuffer(POS_COLOR);
	polyBatchScreen.drawMode = GL_TRIANGLES;

	glLineWidth(3.f);
	glEnable(GL_LINE_SMOOTH);
}

void DrawDebugIcon(u32 space, u32 icon, vec3 position, float size, vec4 color, float timer)
{
	std::cout << "DrawDebugIcon Not Implemented\n";
}

void DrawDebugLine(u32 space, vec3 from, vec3 to, float thickness, vec4 color, float timer)
{
	DebugLine d_line;
	d_line.start = from;
	d_line.end = to;
	d_line.color = color;
	d_line.timer = timer;
	d_line.space = space;
	lines.push_back(d_line);
}

void DrawDebugLine(u32 space, vec2 from, vec2 to, float thickness, vec4 color, float timer)
{
	DebugLine d_line;
	d_line.start = vec3(from, 0);
	d_line.end = vec3(to, 0);
	d_line.color = color;
	d_line.timer = timer;
	d_line.space = space;
	lines.push_back(d_line);
}

void DrawDebugAABB(u32 space, const AABB& aabb, vec4 color, bool fill, float timer)
{
	DebugAABB d_aabb;
	d_aabb.aabb = aabb;
	d_aabb.color = color;
	d_aabb.fill = fill;
	d_aabb.timer = timer;
	d_aabb.space = space;
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
	d_circle.space = space;
	circles.push_back(d_circle);
}

void DrawDebugPolygon(u32 space, const Polygon& polygon, vec4 color, bool fill, float timer)
{
	DebugPolygon d_polygon;
	d_polygon.polygon = polygon;
	d_polygon.color = color;
	d_polygon.fill = fill;
	d_polygon.timer = timer;
	d_polygon.space = space;
	polygons.push_back(d_polygon);
}

void DrawDebugText(u32 space, vec3 position, float size, vec4 color, std::string data, float timer)
{
	DebugText text;
	text.position = position;
	text.size = size;
	text.color = color;
	text.data = data;
	text.timer = timer;
	text.space = space;
	texts.push_back(text);
}

void DrawDebugText(u32 space, vec2 position, float size, vec4 color, std::string data, float timer)
{
	DrawDebugText(space, vec3(position, 0.f), size, color, data, timer);
}

void PushLineToBatch(vec3 start, vec3 end, vec4 color, u32 space)
{
	if (space == DEBUG_WORLD)
	{
		lineBatchWorld.buffer->posColorVerts.push_back(Vertex_PosColor(start, color));
		lineBatchWorld.buffer->posColorVerts.push_back(Vertex_PosColor(end, color));
		lineBatchWorld.buffer->vertexIndices.push_back(lineBatchWorld.buffer->vertexCount);
		lineBatchWorld.buffer->vertexIndices.push_back(lineBatchWorld.buffer->vertexCount + 1);
		lineBatchWorld.buffer->vertexCount += 2;
		lineBatchWorld.buffer->dirty = true;
	}
	else if (space == DEBUG_SCREEN)
	{
		start = PixelToNDC(start);
		end = PixelToNDC(end);

		lineBatchScreen.buffer->posColorVerts.push_back(Vertex_PosColor(start, color));
		lineBatchScreen.buffer->posColorVerts.push_back(Vertex_PosColor(end, color));
		lineBatchScreen.buffer->vertexIndices.push_back(lineBatchScreen.buffer->vertexCount);
		lineBatchScreen.buffer->vertexIndices.push_back(lineBatchScreen.buffer->vertexCount + 1);
		lineBatchScreen.buffer->vertexCount += 2;
		lineBatchScreen.buffer->dirty = true;
	}
}

void PushTriToBatch(vec3 a, vec3 b, vec3 c, vec4 color)
{

}

void PushAABBToBatch(const AABB& box, vec4 color, bool fill, u32 space)
{
	if (fill)
	{

	}
	else
	{
		PushLineToBatch(vec3(box.min, 0.f),				 vec3(box.max.x, box.min.y, 0.f), color, space);
		PushLineToBatch(vec3(box.max.x, box.min.y, 0.f), vec3(box.max, 0.f), color, space);
		PushLineToBatch(vec3(box.max, 0.f),				 vec3(box.min.x, box.max.y, 0.f), color, space);
		PushLineToBatch(vec3(box.min.x, box.max.y, 0.f), vec3(box.min, 0.f), color, space);
	}
}

void PushPolygonToBatch(const Polygon& polygon, vec4 color, bool fill, u32 space)
{
	if (fill)
	{
		
	}
	else
	{
		for (int i = 0; i < polygon.vertices.size(); i++)
		{
			int next = i + 1;
			if (next == polygon.vertices.size()) next = 0;
			PushLineToBatch(vec3(polygon.vertices[i], 0), vec3(polygon.vertices[next], 0), color, space);
		}
	}
}

void PushCircleToBatch(const Circle& circle, vec4 color, u32 pointCount, bool fill, u32 space)
{
	if (fill)
	{
		
	}
	else
	{
		vec3 lastPos = vec3(circle.position, 0) + vec3(0, circle.radius, 0); //sin(0), cos(0)
		//if (space == DEBUG_SCREEN) lastPos = PixelToNDC(lastPos);

		for (int i = 0; i < pointCount; i++)
		{
			float c = (float)(i + 1) / pointCount;
			c = 3.1415926f * c * 2.f;
			vec3 pos = vec3(circle.position, 0) + vec3(sin(c) * circle.radius, cos(c) * circle.radius, 0.f);
			//if (space == DEBUG_SCREEN) pos = PixelToNDC(pos);

			PushLineToBatch(lastPos, pos, color, space);
			lastPos = pos;
		}
	}
}

void DrawDebug(float dt)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	lineBatchWorld.buffer->Clear();
	polyBatchWorld.buffer->Clear();
	textBatchWorld.buffer->Clear();

	lineBatchScreen.buffer->Clear();
	polyBatchScreen.buffer->Clear();
	textBatchScreen.buffer->Clear();

	//Process lines
	auto line_it = lines.begin();
	while (line_it != lines.end())
	{
		PushLineToBatch(line_it->start, line_it->end, line_it->color, line_it->space);

		//Update timer, remove if needed
		line_it->timer -= dt;
		if (line_it->timer <= 0) line_it = lines.erase(line_it);
		else line_it++;
	}

	//Process aabbs
	auto aabb_it = aabbs.begin();
	while (aabb_it != aabbs.end())
	{
		PushAABBToBatch(aabb_it->aabb, aabb_it->color, aabb_it->fill, aabb_it->space);

		//Update timer, remove if needed
		aabb_it->timer -= dt;
		if (aabb_it->timer <= 0) aabb_it = aabbs.erase(aabb_it);
		else aabb_it++;
	}

	//Process circles
	auto circle_it = circles.begin();
	while (circle_it != circles.end())
	{
		PushCircleToBatch(circle_it->circle, circle_it->color, circle_it->pointCount, circle_it->fill, circle_it->space);

		//Update timer, remove if needed
		circle_it->timer -= dt;
		if (circle_it->timer <= 0) circle_it = circles.erase(circle_it);
		else circle_it++;
	}

	//Process polygons
	auto polygon_it = polygons.begin();
	while (polygon_it != polygons.end())
	{
		PushPolygonToBatch(polygon_it->polygon, polygon_it->color, polygon_it->fill, polygon_it->space);

		//Update timer, remove if needed
		polygon_it->timer -= dt;
		if (polygon_it->timer <= 0) polygon_it = polygons.erase(polygon_it);
		else polygon_it++;
	}

	//Process text
	auto texts_it = texts.begin();
	while (texts_it != texts.end())
	{
		//Add text
		Text text;
		text.data = texts_it->data;
		
		text.color = texts_it->color;
		text.font = textBatchWorld.font;

		if (texts_it->space == DEBUG_WORLD)
		{
			text.position = texts_it->position - vec3(5.f, 5.f, 0.f);
			text.extents = vec2(10);
			text.textSize = texts_it->size;
			text.alignment = CENTER;
			textBatchWorld.PushText(text);
		}
		else if (texts_it->space == DEBUG_SCREEN)
		{
			text.position = PixelToNDC(texts_it->position);
			text.extents = PixelToNDC(vec2(300)) + vec2(1);
			text.textSize = PixelToNDC(vec2(0, texts_it->size)).y + 1.f;
			text.alignment = BOTTOM_RIGHT;
			text.scale = vec2(GetWindowSize().y / GetWindowSize().x, 1.f);
			textBatchScreen.PushText(text);
		}

		//Update timer, remove if needed
		texts_it->timer -= dt;
		if (texts_it->timer <= 0) texts_it = texts.erase(texts_it);
		else texts_it++;
	}

	//Draw text over lines over polys
	//Draw screen over world
	polyBatchWorld.Draw();
	lineBatchWorld.Draw();
	textBatchWorld.Draw();

	polyBatchScreen.Draw();
	lineBatchScreen.Draw();
	textBatchScreen.Draw();
}
