#pragma once
#include "MengineTypes.h"
#include "MengineColor.h"
#include <string>

namespace MEngine // TODODB: Create a way to draw text without using the component system (for easy access to text rendering)
{
	constexpr int32_t DEFAULT_TEXT_SIZE = 20;
	const MEngine::ColorData DEFAULT_TEXT_COLOR = PredefinedColors::Colors[PredefinedColors::BLACK];

	MEngineFontID CreateFont(const std::string& relativeFontPath, int32_t fontSize = DEFAULT_TEXT_SIZE, const ColorData& textColor = DEFAULT_TEXT_COLOR);
	bool DestroyFont(MEngineFontID ID);

	// Size is returned as uint16_t, int32_t is used so that -1 can be returned in case of an error
	int32_t GetTextWidth(MEngineFontID ID, const char* text);
	int32_t GetTextHeight(MEngineFontID ID, const char* text);

	bool IsFontIDValid(MEngineFontID ID); // Create a collecttion of these functions in MEngineTypes.h so that host applications can easiliy check the validity of their IDs
}