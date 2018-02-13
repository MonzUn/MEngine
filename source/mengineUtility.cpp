#include "interface/mengineUtility.h"
#include "mengineUtilityInternal.h"
#include "mengineGraphicsInternal.h"
#include <MUtilitySystem.h>
#include <SDL.h>
#include <atomic>

const std::string*	ExecutablePath	= nullptr;
std::atomic<bool>	HasFocus		= false;
std::atomic<bool>	IsHovered		= false;

// ---------- INTERFACE ----------

const std::string& MEngineUtility::GetExecutablePath()
{
	return *ExecutablePath;
}

bool MEngineUtility::WindowHasFocus()
{
	return HasFocus;
}

bool MEngineUtility::WindowIsHovered()
{
	return IsHovered;
}

// ---------- INTERNAL ----------

void MEngineUtility::Initialize()
{
	ExecutablePath = new std::string(MUtility::GetExecutableDirectoryPath());
	HasFocus = SDL_GetWindowFlags(MEngineGraphics::GetWindow()) & SDL_WINDOW_INPUT_FOCUS;
	IsHovered = SDL_GetWindowFlags(MEngineGraphics::GetWindow()) & SDL_WINDOW_MOUSE_FOCUS;
}

void MEngineUtility::Shutdown()
{
	delete ExecutablePath;
}

void MEngineUtility::Update()
{
	HasFocus = SDL_GetWindowFlags(MEngineGraphics::GetWindow()) & SDL_WINDOW_INPUT_FOCUS;
	IsHovered = SDL_GetWindowFlags(MEngineGraphics::GetWindow()) & SDL_WINDOW_MOUSE_FOCUS;
}