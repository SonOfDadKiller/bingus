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

#ifdef LIVEPP_ENABLE
//LIVE++
lpp::LppDefaultAgent lppAgent;
#endif

//Time management
static float dt = 0.f;
static float gameTime = 0.f;
static float stepAccumulator = 0.f;
static float timestep = 1.f / 60.f;
static float timestepAlpha = 0.f;

float GetDeltaTime()
{
	return dt;
}

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

void SetFixedTimestep(float _timestep)
{
	timestep = _timestep;
}

float GetFixedTimestep()
{
	return timestep;
}

float GetTimestepAlpha()
{
	return timestepAlpha;
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
	InitializeDebug();
	exitGameCalled = false;
	clearColor = vec4(0, 0, 0, 1);

	//Connect Live Plus Plus if enabled
#ifdef LIVEPP_ENABLE
	// create a default agent, loading the Live++ agent from the given path, e.g. "ThirdParty/LivePP"
	lppAgent = lpp::LppCreateDefaultAgent(nullptr, L"../include/LivePP");

	// bail out in case the agent is not valid
	if (!lpp::LppIsValidDefaultAgent(&lppAgent))
	{
		std::cout << "ERROR: LIVEPP AGENT NOT VALID";
		return;
	}

	// enable Live++ for all loaded modules
	lppAgent.EnableModule(lpp::LppGetCurrentModulePath(), lpp::LPP_MODULES_OPTION_ALL_IMPORT_MODULES, nullptr, nullptr);
#endif
}

void SwapBuffers()
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	glfwSwapInterval(1);
	glfwSwapBuffers(GetWindow());
}

void RunGame()
{
	if (startEvent != nullptr) startEvent();

	float prevTime = 0.f;

	//Game loop
	while (!GameShouldExit())
	{
		prevTime = gameTime;
		gameTime = (float)glfwGetTime();
		dt = gameTime - prevTime;

		UpdateInput(GetWindow(), dt);

		//Update timers
		for (auto it = timers.begin(); it != timers.end(); it++)
		{
			if (!(*it)->paused) (*it)->timeElapsed += dt * (*it)->speed;
		}

		globalGUIContext.Start();

		//Fixed timestep
		if (dt > 0.25f) dt = 0.25f;
		stepAccumulator += dt;

		while (stepAccumulator >= timestep)
		{
			if (fixedUpdateEvent != nullptr) fixedUpdateEvent(timestep);
			stepAccumulator -= timestep;
		}

		if (updateEvent != nullptr) updateEvent(dt);

		//UpdateDebug();


		timestepAlpha = stepAccumulator / timestep;

		//Draw
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (drawEvent != nullptr) drawEvent();

		globalGUIContext.EndAndDraw();

		globalRenderQueue.Draw();
		globalRenderQueue.Clear();

		DrawDebug(dt);

		SwapBuffers();
		glfwPollEvents();

		//Calculate FPS
		avgFrameTime = CalcAverageTick(dt);
		framesPerSecond = (i32)(1.f / avgFrameTime);

		//Catch glfw window close event
		if (glfwWindowShouldClose(GetWindow()))
		{
			ExitGame();
		}

#ifdef TRACY_ENABLE
		FrameMark;
#endif
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

#ifdef LIVEPP_ENABLE
	// destroy the Live++ agent
	lpp::LppDestroyDefaultAgent(&lppAgent);
#endif
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