#include "bingus.h"

#include <iomanip>
#include <sstream>

using namespace gui;

void Start();
void Update(float dt);
void FixedUpdate(float dt);
void Draw();
void Reset();

struct Boid
{
	vec2 position;
	vec2 oldPosition;
	vec2 size;
	vec2 velocity;
	vec2 oldVelocity;
	vec4 color;
};

GUIWindow controlWindow;
bool tickboxState;

SpriteBatch spriteBatch;
SpriteSheet spriteSheet;

std::vector<Boid> boids;

float timestep = 1.f / 60.f;
float alignmentWeight = 0.f;// 0.0005f;
float cohesionWeight = 0.f;// 0.005f;
float separationWeight = 0.f;// 0.001f;
float cursorWeight = 0.008f;
float acceleration = 2.75f;
float drag = 0.00009f;
float maxSpeed = 0.55f;

int main()
{
	SetupWindow(1280, 720, "Boids");
	BingusInit();

	SetGameStartFunction(Start);
	SetGameUpdateFunction(Update);
	SetGameFixedUpdateFunction(FixedUpdate);
	SetGameDrawFunction(Draw);
	
	RunGame();

	return 0;
}

void Start()
{
	//Control Window
	controlWindow.pos = vec2(25, 25);
	controlWindow.minSize = vec2(500, 600);
	controlWindow.maxSize = controlWindow.minSize;

	//Set up sprite batch
	spriteSheet = SpriteSheet("triangle.png", { { "triangle", SpriteSequence(vec2(0), vec2(128, 128), 4, 0.f) } });

	spriteBatch = SpriteBatch(VertBuffer({ VERTEX_POS, VERTEX_UV, VERTEX_COLOR }),
		Shader("world_vertcolor.vert", "sprite_vertcolor.frag", SHADER_MAIN_TEX),
		&spriteSheet);

	SetCameraSize(7.f);

	//Input Bindings
	BindInputAction(KEY_ESCAPE, HOLD, [](float dt)
	{
		ExitGame();
	});

	BindInputAction(KEY_W, HOLD, [](float dt)
	{
		TranslateCamera(vec2(0, 3) * dt);
	});

	BindInputAction(KEY_A, HOLD, [](float dt)
	{
		TranslateCamera(vec2(-3, 0) * dt);
	});

	BindInputAction(KEY_S, HOLD, [](float dt)
	{
		TranslateCamera(vec2(0, -3) * dt);
	});

	BindInputAction(KEY_D, HOLD, [](float dt)
	{
		TranslateCamera(vec2(3, 0) * dt);
	});

	BindInputAction(MOUSE_SCROLL_UP, PRESS, [](float dt)
	{
		ZoomCamera(-0.2f);
	});

	BindInputAction(MOUSE_SCROLL_DOWN, PRESS, [](float dt)
	{
		ZoomCamera(0.2f);
	});

	BindInputAction(KEY_R, PRESS, [](float dt)
	{
		Reset();
	});

	Reset();
}

void Reset()
{
	boids.clear();

	//Create flock
	int flockCount = 100;
	#ifdef NDEBUG
	flockCount = 35000;
	#endif

	for (int i = 0; i < flockCount; i++)
	{
		static const float pi = 3.14159265;
		float circlePos = (float)i / flockCount;
		float f = circlePos * pi * 2.f;
		vec2 pos = vec2(sin(f), cos(f)) * 1.f;

		Boid boid;
		boid.position = pos;
		boid.oldPosition = pos;
		boid.size = vec2(0.1f, 0.15f) * 1.5f;
		boid.velocity = vec2(0);

		if (i % 2 == 0)
		{
			boid.color = hsv(vec4(circlePos, 0.6f, 1.f, 0.8f));
		}
		else
		{
			boid.color = hsv(vec4(0.5f - circlePos * 0.5f, 0.6f, 1.f, 0.8f));
		}

		boids.push_back(boid);
	}
}

void GUIControl(std::string label, float* value)
{
	Widget();
		vars.margin = Edges::None();
		vars.size = vec2(0, 50);
		Layout(HORIZONTAL);
			vars.size = vec2(0);
			vars.margin = Edges::All(0.001f);
			vars.spacing = 4.f;
			gui::Text(label);
				vars.size = vec2(200, 50);
				vars.textAlignment = CENTER_LEFT;
			EndNode();
			Button();
				vars.size = vec2(50);
				vars.onPress = [value]() {
					*value *= 0.95f;
				};
				Image(MINUS);
					vars.pivot = vars.anchor = CENTER;
					vars.size = vec2(35);
				EndNode();
			EndNode();
			gui::Text(std::to_string(*value));
				vars.size = vec2(150, 50);
				vars.textAlignment = CENTER;
			EndNode();
			Button();
				vars.size = vec2(50);
				vars.onPress = [value]() {
					*value *= 1.05f;
				};
				Image(PLUS);
					vars.pivot = vars.anchor = CENTER;
					vars.size = vec2(35);
				EndNode();
			EndNode();
		EndNode();
	EndNode();
}

void FixedUpdate(float dt)
{
	//Flocking algorithm
	for (auto it = boids.begin(); it != boids.end(); it++)
	{
		//u32 neighbourCount = 0;
		//vec2 alignment = vec2(0);
		//vec2 cohesion = vec2(0);
		//vec2 separation = vec2(0);

		//vec2 avgNeighbourPos = vec2(0);
		//vec2 avgNeighbourVel = vec2(0);
		//vec2 avgNeighbourDist = vec2(0);

		///*for (auto neighbour_it = boids.begin(); neighbour_it != boids.end(); neighbour_it++)
		//{
		//	if (it != neighbour_it)
		//	{
		//		if (glm::distance(it->position, neighbour_it->position) < 3.f)
		//		{
		//			avgNeighbourVel += neighbour_it->velocity;
		//			avgNeighbourPos += neighbour_it->position;
		//			avgNeighbourDist += neighbour_it->position - it->position;
		//			neighbourCount++;
		//		}
		//	}
		//}*/

		//if (neighbourCount != 0)
		//{
		//	avgNeighbourVel /= neighbourCount;
		//	avgNeighbourPos /= neighbourCount;
		//	avgNeighbourDist /= neighbourCount;
		//}

		//alignment = avgNeighbourVel;
		//cohesion = avgNeighbourPos - transform.position;
		//separation = -avgNeighbourDist;
		vec2 cursorMove = (vec2(mouseWorldPosition) - it->position);

		/*if (alignment != vec2(0)) alignment = glm::normalize(alignment);
		if (cohesion != vec2(0)) cohesion = glm::normalize(cohesion);
		if (separation != vec2(0)) separation = glm::normalize(separation);*/
		if (cursorMove != vec2(0)) cursorMove = glm::normalize(cursorMove);

		it->oldVelocity = it->velocity;
		
		/*it->velocity += (alignment * alignmentWeight
						+ cohesion * cohesionWeight
						+ separation * separationWeight
						+ cursorMove * cursorWeight)
						* acceleration;*/

		it->velocity += cursorMove * cursorWeight * acceleration;

		float speed = glm::length(it->velocity);
		float newSpeed = glm::clamp(speed - drag, 0.f, maxSpeed);
		it->velocity = glm::normalize(it->velocity) * newSpeed;

		it->oldPosition = it->position;
		it->position += it->velocity;
	}
}

void Update(float dt)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	//GUI
	Window(&controlWindow);
		Layout(VERTICAL);
			vars.size = vec2(0);
			vars.margin = Edges::All(15.f);
			vars.spacing = 4.f;

			GUIControl("Timestep: ", &timestep);
			GUIControl("Alignment: ", &alignmentWeight);
			GUIControl("Cohesion: ", &cohesionWeight);
			GUIControl("Separation: ", &separationWeight);
			GUIControl("Cursor Weight: ", &cursorWeight);
			GUIControl("Acceleration: ", &acceleration);
			GUIControl("Drag: ", &drag);
			GUIControl("Max Speed: ", &maxSpeed);
		EndNode();

		Button();
			vars.pivot = vars.anchor = BOTTOM_CENTER;
			vars.margin = Edges::All(15.f);
			vars.size = vec2(140, 50);
			vars.onPress = Reset;
			gui::Text("Reset");
				vars.size = vec2(0);
				vars.margin = Edges::All(0.001f);
				vars.textAlignment = CENTER;
			EndNode();
		EndNode();
	EndNode();
	
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << GetAvgFrameTime() * 1000.f;
	gui::Text("fps: " + std::to_string(GetFPS()) + "(" + stream.str() + "ms)");
		gui::vars.margin = Edges::All(25);
		gui::vars.size = vec2(0);
	gui::EndNode();

	//Logic
	SetFixedTimestep(timestep);
}

void Draw()
{
	//Draw sprites
	spriteBatch.Clear();

	for (auto it = boids.begin(); it != boids.end(); it++)
	{
		vec2 pos2 = glm::mix(it->oldPosition, it->position, GetTimestepAlpha());
		vec3 pos = vec3(pos2.x, pos2.y, 0.f);
		vec2 mixVelocity = glm::mix(it->oldVelocity, it->velocity, GetTimestepAlpha());
		float rotation = glm::degrees(atan2(mixVelocity.y, mixVelocity.x)) + 270.f;
		spriteBatch.PushSprite(Sprite(pos, it->size, CENTER, rotation, Edges::None(), 
			it->color, &spriteSheet.sequences["triangle"], 0));
	}

	spriteBatch.Draw();
}
