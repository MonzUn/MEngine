#include "interface/mengine.h"
#include "mengineGraphicsInternal.h"
#include "mengineInputInternal.h"
#include <MUtilityLog.h>
#include <SDL.h>
#include <cassert>
#include <iostream>

#define MUTILITY_LOG_CATEGORY_GENERAL "MEngine"

bool Initialized = false;
bool QuitRequested = false;

bool MEngine::Initialize(const char* appName, int32_t windowWidth, int32_t windowHeight)
{
	assert(!IsInitialized() && "Calling SDLWrapper::Initialize but it has already been initialized");

	MUtilityLog::Initialize();

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		MLOG_ERROR("MEngine initialization failed; SDL_Init Error: " + std::string(SDL_GetError()), MUTILITY_LOG_CATEGORY_GENERAL);
		return false;
	}

	if (!MEngineGraphics::Initialize(appName, windowWidth, windowHeight))
	{
		MLOG_ERROR("Failed to initialize MEngineGraphics", MUTILITY_LOG_CATEGORY_GENERAL);
		return false;
	}

	MEngineInput::Initialize();

	MLOG_INFO("MEngine initialized successfully", MUTILITY_LOG_CATEGORY_GENERAL);

	Initialized = true;
	return true;
}

void MEngine::Shutdown()
{
	assert(IsInitialized() && "Calling SDLWrapper::Shutdown but it has not yet been initialized");

	Initialized = false;
	SDL_Quit();

	MLOG_INFO("MEngine terminated gracefully", MUTILITY_LOG_CATEGORY_GENERAL);
	MUtilityLog::Shutdown();
}

bool MEngine::IsInitialized()
{
	return Initialized;
}

bool MEngine::ShouldQuit()
{
	return QuitRequested;
}

void MEngine::Update()
{
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		if (event.type == SDL_QUIT)
			QuitRequested = true;
	};

	MEngineInput::Update();
}

void MEngine::Render()
{
	MEngineGraphics::Render();
}