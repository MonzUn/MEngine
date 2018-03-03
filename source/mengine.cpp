#include "interface/mengine.h"
#include "interface/mengineUtility.h"
#include "mengineComponentManagerInternal.h"
#include "mengineConfigInternal.h"
#include "mengineEntityManagerInternal.h"
#include "mengineGraphicsInternal.h"
#include "mengineInternalComponentsInternal.h"
#include "mengineInputInternal.h"
#include "mengineSystemManagerInternal.h"
#include "mengineTextInternal.h"
#include "mengineUtilityInternal.h"
#include <MUtilityLog.h>
#include <MUtilitySystem.h>
#include <SDL.h>
#include <cassert>
#include <iostream>

#define LOG_CATEGORY_GENERAL "MEngine"

namespace MEngine
{
	bool m_Initialized = false;
	bool m_QuitRequested = false;
}

bool MEngine::Initialize(const char* appName, int32_t windowWidth, int32_t windowHeight)
{
	assert(!IsInitialized() && "Calling SDLWrapper::Initialize but it has already been initialized");

	MUtilityLog::Initialize();

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		MLOG_ERROR("MEngine initialization failed; SDL_Init Error: " + std::string(SDL_GetError()), LOG_CATEGORY_GENERAL);
		MUtilityLog::Shutdown();
		return false;
	}

	if (!MEngineGraphics::Initialize(appName, windowWidth, windowHeight))
	{
		MLOG_ERROR("Failed to initialize MEngineGraphics", LOG_CATEGORY_GENERAL);
		MUtilityLog::Shutdown();
		return false;
	}

	// TODODB: Make a separate file for all system initialization/shutdown so that we can avoid cluttering this file with them and their includes
	MEngineUtility::Initialize();
	MEngineEntityManager::Initialize();
	MEngineComponentManager::Initialize();
	MEngineInternalComponents::Initialize();
	MEngineInput::Initialize();
	MEngineText::Initialize();
	MEngineConfig::Initialize();
	MEngineSystemManager::Initialize();

	MLOG_INFO("MEngine initialized successfully", LOG_CATEGORY_GENERAL);

	m_Initialized = true;
	return true;
}

void MEngine::Shutdown()
{
	assert(IsInitialized() && "Calling SDLWrapper::Shutdown but it has not yet been initialized");

	MEngineSystemManager::Shutdown();
	MEngineConfig::Shutdown();
	MEngineText::Shutdown();
	MEngineInput::Shutdown();
	MEngineInternalComponents::Shutdown();
	MEngineComponentManager::Shutdown();
	MEngineEntityManager::Shutdown();
	MEngineGraphics::Shutdown();
	MEngineUtility::Shutdown();

	m_Initialized = false;
	SDL_Quit();

	MLOG_INFO("MEngine terminated gracefully", LOG_CATEGORY_GENERAL);
	MUtilityLog::Shutdown();
}

bool MEngine::IsInitialized()
{
	return m_Initialized;
}

bool MEngine::ShouldQuit()
{
	return m_QuitRequested;
}

void MEngine::Update()
{
	MEngineUtility::Update();
	MEngineInput::Update();

	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		if (event.type == SDL_QUIT)
		{
			m_QuitRequested = true;
			break;
		}

		MEngineInput::HandleEvent(event);
	};

	MEngineSystemManager::Update();
}

void MEngine::Render()
{
	MEngineGraphics::Render();
}