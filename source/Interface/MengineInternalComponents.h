#pragma once
#include "MEngineColor.h"
#include "MEngineComponent.h"
#include "MEngineInput.h"
#include <MUtilityLog.h>
#include <MUtilityTypes.h>
#include <MUtilityMacros.h>
#include <functional>
#include <string>

namespace MEngine
{
	enum class TextBoxFlags : MUtility::BitSet
	{
		None = 0,

		Editable					= 1 << 0,
		Scrollable					= 1 << 1,
		OverwriteOnDefaultTextMatch = 1 << 2,
		ResetToDefaultWhenEmpty		= 1 << 3,
	};
	CREATE_BITFLAG_OPERATOR_SIGNATURES(TextBoxFlags);

	class PosSizeComponent : public ComponentBase<PosSizeComponent>
	{
	public:
		int32_t		PosX	= 0;
		int32_t		PosY	= 0;
		uint32_t	PosZ	= ~0U;
		int32_t		Width	= 0;
		int32_t		Height	= 0;

		bool IsMouseOver() const
		{
			return GetCursorPosX() >= PosX && GetCursorPosX() < PosX + Width && GetCursorPosY() >= PosY && GetCursorPosY() < PosY + Height;
		}
	};

	class RectangleRenderingComponent : public ComponentBase<RectangleRenderingComponent>
	{
	public:
		bool IsFullyTransparent() const { return BorderColor.IsFullyTransparent() && FillColor.IsFullyTransparent(); }

		ColorData BorderColor	= ColorData(PredefinedColors::TRANSPARENT_);
		ColorData FillColor		= ColorData(PredefinedColors::TRANSPARENT_);
		bool RenderIgnore		= false;
	};

	class TextureRenderingComponent : public ComponentBase<TextureRenderingComponent>
	{
	public:
		bool RenderIgnore	= false;
		TextureID TextureID;
	};

	class ButtonComponent : public ComponentBase<ButtonComponent>
	{
	public:
		void Destroy() override;

		bool IsActive		= true;
		bool IsTriggered	= false;
		bool IsMouseOver	= false;
		std::function<void()>* Callback = nullptr; // TODODB: Attempt to make it possible to use any parameters and return type
	};

	class TextComponent : public ComponentBase<TextComponent>
	{
	public:
		void Destroy() override;

		FontID FontID;
		std::string* Text				= nullptr;
		const std::string* DefaultText	= nullptr;
		TextAlignment Alignment			= TextAlignment::BottomLeft;
		bool RenderIgnore				= false;
		TextBoxFlags EditFlags			= TextBoxFlags::None;
		uint32_t ScrolledLinesCount		= 0;

		void StartEditing() const // TODODB: When we can use any parameter for button callbacks; move this to the relevant system instead
		{
			if (Text == nullptr)
			{
				MLOG_WARNING("Attempted to edit textcomponent text while it was null", "TextComponent"); // TODODB: Set proper log category when this code has been moved into a system
				return;
			}

			if ((EditFlags & TextBoxFlags::Editable) != 0)
			{
				// Removed the text if it is the default text
				if (((EditFlags & TextBoxFlags::OverwriteOnDefaultTextMatch) != 0) && DefaultText != nullptr && *Text == *DefaultText)
					*Text = "";

				MEngine::StartTextInput(Text);
			}
		};

		void StopEditing() const // TODODB: See if there are any additional places we should call this (direct calls to MEngineInput was used before this was created)
		{
			if ((EditFlags & TextBoxFlags::Editable) != 0 && MEngine::IsInputString(Text))
			{
				// Return the text to the default if it is left empty
				if ((EditFlags & TextBoxFlags::ResetToDefaultWhenEmpty) != 0 && *Text == "")
					*Text = *DefaultText;

				MEngine::StopTextInput();
			}
		}
	};
}