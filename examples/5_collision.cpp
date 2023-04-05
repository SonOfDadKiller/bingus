#include "bingus.h"

using namespace gui;

void Start();
void Update(float dt);
void Draw();

GUIWindow window;

AABB aabb1;
AABB aabb2;
Circle circle1;
Circle circle2;
Segment segment;

enum TestMode { AABBToAABB, CircleToCircle, CircleToAABB, SegmentToAABB, SegmentToCircle, CircleSweepToAABB };
TestMode testMode;

int main()
{
	SetupWindow(1280, 720, "Collision");
	BingusInit();

	SetGameStartFunction(Start);
	SetGameUpdateFunction(Update);
	SetGameDrawFunction(Draw);

	testMode = AABBToAABB;
	aabb1 = AABB(vec2(0.2), vec2(0.4));
	aabb2 = AABB(vec2(-0.6), vec2(0.8));
	circle1 = Circle(vec2(-0.4, 0.7), 0.2f);
	circle2 = Circle(vec2(0.8, 0.6), 0.3f);
	segment = Segment(vec2(-0.3), vec2(0.3));

	window.pos = vec2(15, 70);
	window.size = vec2(250, 600);
	window.minSize = window.size;
	window.maxSize = window.size;

	RunGame();

	return 0;
}

void Start()
{
	globalInputListener.BindAction(KEY_ESCAPE, HOLD, []() {
		ExitGame();
	});

	globalInputListener.BindAction(KEY_Q, HOLD, []() {
		aabb1.min = mouseWorldPosition;
	});

	globalInputListener.BindAction(KEY_W, HOLD, []() {
		aabb1.max = mouseWorldPosition;
	});

	globalInputListener.BindAction(KEY_E, HOLD, []() {
		aabb2.min = mouseWorldPosition;
	});

	globalInputListener.BindAction(KEY_R, HOLD, []() {
		aabb2.max = mouseWorldPosition;
	});

	globalInputListener.BindAction(KEY_A, HOLD, []() {
		circle1.position = mouseWorldPosition;
	});

	globalInputListener.BindAction(KEY_S, HOLD, []() {
		circle1.radius = glm::distance(circle1.position, vec2(mouseWorldPosition.x, mouseWorldPosition.y));
	});

	globalInputListener.BindAction(KEY_D, HOLD, []() {
		circle2.position = mouseWorldPosition;
	});

	globalInputListener.BindAction(KEY_F, HOLD, []() {
		circle2.radius = glm::distance(circle2.position, vec2(mouseWorldPosition.x, mouseWorldPosition.y));
	});

	globalInputListener.BindAction(KEY_Z, HOLD, []() {
		segment.start = mouseWorldPosition;
	});

	globalInputListener.BindAction(KEY_X, HOLD, []() {
		segment.end = mouseWorldPosition;
	});
}

void ModeButton(std::string text, TestMode mode)
{
	Button();
		vars.size = vec2(0, 60);
		vars.margin = Edges::All(0.1f);
		vars.onPress = [mode]() {
			testMode = mode;
		};
		gui::Text(text);
			vars.margin = Edges::All(0.1f);
			vars.size = vec2(0);
			vars.textAlignment = CENTER;
		EndNode();
	EndNode();
}

void Update(float dt)
{
	Window(&window);
		Layout(VERTICAL);
			vars.size = vec2(0);
			vars.margin = Edges(0.f, 15.f, 15.f, 15.f);
			vars.spacing = 4.f;
			vars.stretch = true;
			gui::Text("Collision Tests:");
				vars.size = vec2(0, 40);
				vars.margin = Edges::All(0.1f);
				vars.textAlignment = CENTER;
			EndNode();
			ModeButton("AABB to AABB", AABBToAABB);
			ModeButton("Circle to Circle", CircleToCircle);
			ModeButton("Circle to AABB", CircleToAABB);
			ModeButton("Segment to AABB", SegmentToAABB);
			ModeButton("Segment to Circle", SegmentToCircle);
			ModeButton("Circle Sweep to AABB", CircleSweepToAABB);
		EndNode();
	EndNode();

	gui::Text("fps: " + std::to_string(GetFPS()) + "(" + std::to_string(GetAvgFrameTime()) + "ms)");
		vars.margin = Edges::All(25);
		vars.size = vec2(0);
	EndNode();
}

void Draw()
{
	if (testMode == AABBToAABB)
	{
		bool intersects = TestAABBAABB(aabb1, aabb2);
		vec4 color = intersects ? vec4(1, 0, 0, 1) : vec4(1);
		DrawDebugAABB(DEBUG_WORLD, aabb1, color, false);
		DrawDebugAABB(DEBUG_WORLD, aabb2, color, false);
	}
	else if (testMode == CircleToCircle)
	{
		bool intersects = TestCircleCircle(circle1, circle2);
		vec4 color = intersects ? vec4(1, 0, 0, 1) : vec4(1);
		DrawDebugCircle(DEBUG_WORLD, circle1, 64, color, false);
		DrawDebugCircle(DEBUG_WORLD, circle2, 64, color, false);
	}
	else if (testMode == CircleToAABB)
	{
		bool intersects = TestCircleAABB(circle1, aabb1);
		vec4 color = intersects ? vec4(1, 0, 0, 1) : vec4(1);
		DrawDebugCircle(DEBUG_WORLD, circle1, 64, color, false);
		DrawDebugAABB(DEBUG_WORLD, aabb1, color, false);
	}
	else if (testMode == SegmentToAABB)
	{
		float t;
		vec2 p;
		bool intersects = IntersectSegmentAABB(segment, aabb1, p, t);
		vec4 color = intersects ? vec4(1, 0, 0, 1) : vec4(1);
		DrawDebugLine(DEBUG_WORLD, segment.start, segment.end, 1.f, color);
		DrawDebugAABB(DEBUG_WORLD, aabb1, color, false);

		if (intersects)
		{
			DrawDebugCircle(DEBUG_WORLD, Circle(p, 0.02f), 16, vec4(1), false);
			DrawDebugText(DEBUG_WORLD, vec3(p.x + 0.2, p.y + 0.2f, 0), 0.8f, vec4(1), "t = " + std::to_string(t));
		}
	}
	else if (testMode == SegmentToCircle)
	{
		float t;
		vec2 p;
		bool intersects = IntersectSegmentCircle(segment, circle1, p, t);
		vec4 color = intersects ? vec4(1, 0, 0, 1) : vec4(1);
		DrawDebugLine(DEBUG_WORLD, segment.start, segment.end, 1.f, color);
		DrawDebugCircle(DEBUG_WORLD, circle1, 64, color, false);

		if (intersects)
		{
			DrawDebugCircle(DEBUG_WORLD, Circle(p, 0.02f), 16, vec4(1), false);
			DrawDebugText(DEBUG_WORLD, vec3(p.x + 0.2, p.y + 0.2f, 0), 0.8f, vec4(1), "t = " + std::to_string(t));
		}
	}
	else if (testMode == CircleSweepToAABB)
	{
		float t;
		vec2 velocity = circle2.position - circle1.position;
		vec2 direction = glm::normalize(velocity);
		vec2 p = circle2.position;
		bool intersects = SweepCircleAABB(circle1, velocity, aabb1, t);
		vec4 color = intersects ? vec4(1, 0, 0, 1) : vec4(1);
		DrawDebugCircle(DEBUG_WORLD, circle1, 64, color, false);
		DrawDebugCircle(DEBUG_WORLD, Circle(circle2.position, circle1.radius), 64, vec4(1), false);
		DrawDebugAABB(DEBUG_WORLD, aabb1, color, false);

		if (intersects)
		{
			p = circle1.position + direction * t;
			DrawDebugCircle(DEBUG_WORLD, Circle(p, circle1.radius), 64, color, false);
			DrawDebugLine(DEBUG_WORLD, circle1.position, p, 1.f, color);
			DrawDebugLine(DEBUG_WORLD, p, circle2.position, 1.f, vec4(1));
		}
		else
		{
			DrawDebugLine(DEBUG_WORLD, circle1.position, circle2.position, 1.f, color);
		}
	}
}
