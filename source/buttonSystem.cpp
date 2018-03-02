#include "buttonSystem.h"
#include "interface/mengineComponentManager.h"
#include "interface/mengineInternalComponents.h"
#include "interface/mengineInput.h"
#include "interface/mengineText.h"

using namespace MEngine;

void ButtonSystem::UpdatePresentationLayer(float deltaTime)
{
	int32_t cursorPosX = GetCursorPosX();
	int32_t cursorPosY = GetCursorPosY();
	int32_t componentCount = -1;
	ButtonComponent* buttons = reinterpret_cast<ButtonComponent*>(GetComponentBuffer(ButtonComponent::GetComponentMask(), &componentCount));
	for (int i = 0; i < componentCount; ++i)
	{
		const ButtonComponent& button = buttons[i];
		if (cursorPosX >= button.PosX && cursorPosX < button.PosX + button.Width && cursorPosY >= button.PosY && cursorPosY < button.PosY + button.Height)
		{
			// TODODB: Tint the button when it is hovered
			if (KeyReleased(MKEY_MOUSE_LEFT))
				button.Callback();
		}

		MEngine::DrawText(button.PosX + (button.Width / 2) - (GetTextWidth(button.text.c_str()) / 2), button.PosY + (button.Height / 2) - (GetTextHeight(button.text.c_str()) / 2), button.text); // Centered
	}
}