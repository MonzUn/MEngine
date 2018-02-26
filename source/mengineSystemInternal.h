#pragma once
#include "interface/mengineSystem.h"

namespace MEngineSystem
{
	constexpr float DEFAULT_TIME_STEP			= MEngine::MENGINE_TIME_STEP_FPS_15;
	constexpr float DEFAULT_SIMULATION_SPEED	= MEngine::MENGINE_TIME_STEP_FPS_15;

	void Initialize();
	void Shutdown();
	void Update();
}