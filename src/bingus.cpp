#include "bingus.h"

//Game Events
static bool exitGameCalled;

static void(*startEvent)();
static void(*updateEvent)(float);
static void(*fixedUpdateEvent)(float);
static void(*drawEvent)();

//Framerate Tracking
#define MAXSAMPLES 100
static int tickindex = 0;
static float ticksum = 0;
static float ticklist[MAXSAMPLES];
float avgFrameTime;
i32 framesPerSecond;

float CalcAverageTick(float newtick)
{
	ticksum -= ticklist[tickindex];
	ticksum += newtick;
	ticklist[tickindex] = newtick;
	if (++tickindex == MAXSAMPLES) tickindex = 0;
	return ticksum / MAXSAMPLES;
}

//Time management
static float gameTime;
static float stepAccumulator = 0.f;
static const float stepTime = 1.f / 1.f;
static float stepAlpha;

float GetTime()
{
	return gameTime;
}

float GetAvgFrameTime()
{
	return avgFrameTime;
}

u32 GetFPS()
{
	return framesPerSecond;
}

//Timers
static std::vector<Timer*> timers;

Timer* CreateTimer()
{
	Timer* timer = new Timer();
	timer->timeElapsed = 0.f;
	timer->speed = 1.f;
	timer->paused = false;
	timers.push_back(timer);
	return timer;
}

Timer::~Timer()
{
	//Remove from global list upon destruction
	auto it = timers.begin();
	while (it != timers.end())
	{
		if (*it == this) break;
	}
	
	if (it != timers.end()) timers.erase(it);
}

void Timer::Reset()
{
	timeElapsed = 0.f;
}

void Timer::Pause()
{
	paused = true;
}

void Timer::Stop()
{
	Reset();
	Pause();
}

void Timer::Play()
{
	paused = false;
}

//Colors
static vec4 clearColor;

void SetClearColor(vec4 color)
{
	clearColor = color;
}

void BingusInit()
{
	InitializeRenderer();
	InitializeInput(GetWindow());
	InitializeUI();
	InitializeDebug();
	exitGameCalled = false;
	clearColor = vec4(0, 0, 0, 1);
}

void RunGame()
{
	if (startEvent != nullptr) startEvent();

	float dt = 0.f;
	float prevTime = 0.f;

	//Game loop
	while (!GameShouldExit())
	{
		prevTime = gameTime;
		gameTime = (float)glfwGetTime();
		dt = gameTime - prevTime;

		UpdateInput(GetWindow(), dt);
		UpdateUI();

		//Update timers
		for (auto it = timers.begin(); it != timers.end(); it++)
		{
			if (!(*it)->paused) (*it)->timeElapsed += dt * (*it)->speed;
		}

		//Fixed timestep
		if (dt > 0.25f) dt = 0.25f;
		stepAccumulator += dt;

		while (stepAccumulator >= stepTime)
		{
			if (fixedUpdateEvent != nullptr) fixedUpdateEvent(stepTime);
			stepAccumulator -= stepTime;
		}

		if (updateEvent != nullptr) updateEvent(dt);

		stepAlpha = stepAccumulator / dt;

		//Draw
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (drawEvent != nullptr) drawEvent();
		DrawDebug(dt);
		DrawUI();

		glfwSwapBuffers(GetWindow());
		glfwPollEvents();

		//Calculate FPS
		avgFrameTime = CalcAverageTick(dt);
		framesPerSecond = (i32)(1.f / avgFrameTime);

		//Catch glfw window close event
		if (glfwWindowShouldClose(GetWindow()))
		{
			ExitGame();
		}
	}

	BingusCleanup();
}

bool GameShouldExit()
{
	return exitGameCalled;
}

void ExitGame()
{
	exitGameCalled = true;
}

void BingusCleanup()
{
	glfwTerminate();
}

//Game Events
void SetGameStartFunction(void(*callback)())
{
	startEvent = callback;
}

void SetGameUpdateFunction(void(*callback)(float))
{
	updateEvent = callback;
}

void SetGameFixedUpdateFunction(void(*callback)(float))
{
	fixedUpdateEvent = callback;
}

void SetGameDrawFunction(void(*callback)())
{
	drawEvent = callback;
}

