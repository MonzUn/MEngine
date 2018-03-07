#include "interface/mengineConsole.h"
#include "mengineConsoleInternal.h"
#include "interface/mengineColor.h"
#include "interface/mengineEntityManager.h"
#include "interface/mengineEntityFactory.h"
#include "interface/mengineGraphics.h"
#include "interface/mengineInput.h"
#include "interface/mengineText.h"
#include "interface/mengineTypes.h"
#include <MUtilityLog.h>
#include <algorithm>
#include <cctype>
#include <unordered_map>

#define LOG_CATEGROY_CONSOLE "MEngineConsole"

typedef std::unordered_map<std::string, MEngineConsoleCallback> CommandMap;

constexpr char DELIMITER = ' ';
constexpr int32_t INPUT_TEXTBOX_HEIGHT = 25;

namespace MEngineConsole
{
	CommandMap* m_Commands = nullptr;
	MEngine::EntityID m_BackgroundID	= INVALID_MENGINE_ENTITY_ID;
	MEngine::EntityID m_OutputTextboxID	= INVALID_MENGINE_ENTITY_ID;
	MEngine::EntityID m_InputTextboxID	= INVALID_MENGINE_ENTITY_ID;
	bool isActive = true;
	int32_t m_OutputTextBoxOriginalHeight = -1;
}

using namespace MEngine;
using namespace MEngineConsole;
using namespace PredefinedColors;

// ---------- INTERNAL ----------

bool MEngine::RegisterCommand(const std::string& commandName, MEngineConsoleCallback callback)
{
	std::string commandNameLower = commandName;
	std::transform(commandNameLower.begin(), commandNameLower.end(), commandNameLower.begin(), std::tolower);

	bool result = true;
	if (m_Commands->find(commandNameLower) != m_Commands->end())
	{
		MLOG_WARNING("Attempted to register multiple commands using the same name; name = " << commandNameLower, LOG_CATEGROY_CONSOLE);
		result = false;
	}

	if(result)
		m_Commands->emplace(commandNameLower, callback);

	return result;
}

bool MEngine::UnregisterCommand(std::string& commandName)
{
	std::string commandNameLower = commandName;
	std::transform(commandNameLower.begin(), commandNameLower.end(), commandNameLower.begin(), std::tolower);

	bool result = false;
	auto iterator = m_Commands->find(commandNameLower);
	if (iterator != m_Commands->end())
	{
		m_Commands->erase(iterator);
		result = true;
	}
	else
		MLOG_WARNING("Attempted to unregister command \"" << commandNameLower << "\" but no such command is registered", LOG_CATEGROY_CONSOLE);

	return result;
}

void MEngine::UnregisterAllCommands()
{
	m_Commands->clear();
}

bool MEngine::ExecuteCommand(const std::string& command, std::string* outResponse)
{
	if (command == "")
		return false;

	std::string commandLower = command;
	std::transform(commandLower.begin(), commandLower.end(), commandLower.begin(), std::tolower);

	// Find command name
	int32_t firstSpacePosition = static_cast<int32_t>(commandLower.find_first_of(DELIMITER));
	std::string commandName = ((firstSpacePosition == -1) ? commandLower : commandLower.substr(0, firstSpacePosition));

	bool result = false;
	auto iterator = m_Commands->find(commandName);
	if (iterator != m_Commands->end())
	{
		// Create parameter list
		std::string commandParameterString = commandLower.substr(firstSpacePosition + 1); // +1 so that the space doesn't get included in the string
		int32_t parameterCount = static_cast<int32_t>(std::count(commandLower.begin(), commandLower.end(), DELIMITER));

		std::string* parameters = ((parameterCount > 0) ? new std::string[parameterCount] : nullptr);
		int32_t offset = firstSpacePosition;
		for (int i = 0; i < parameterCount; ++i)
		{
			parameters[i] = commandLower.substr(offset + 1); // +1 so that the space doesn't get included in the string
			offset = static_cast<int32_t>(commandParameterString.find_first_of(DELIMITER, offset));
		}

		// Execute
		result = iterator->second(parameters, parameterCount, outResponse);
		delete[] parameters;
	}
	else if(outResponse != nullptr)
		*outResponse = '\"' + commandName + "\" is not a valid command";
		
	return result;
}

void MEngine::SetConsoleActive(bool active)
{
	if (isActive == active)
		return;

	RectangleRenderingComponent* mainBackground = static_cast<RectangleRenderingComponent*>(GetComponentForEntity(RectangleRenderingComponent::GetComponentMask(), m_BackgroundID));
	TextComponent* outputText = static_cast<TextComponent*>(GetComponentForEntity(TextComponent::GetComponentMask(), m_OutputTextboxID));
	RectangleRenderingComponent* outputTextBackground = static_cast<RectangleRenderingComponent*>(GetComponentForEntity(RectangleRenderingComponent::GetComponentMask(), m_OutputTextboxID));
	TextComponent* inputText = static_cast<TextComponent*>(GetComponentForEntity(TextComponent::GetComponentMask(), m_InputTextboxID));
	RectangleRenderingComponent* inputTextbackground = static_cast<RectangleRenderingComponent*>(GetComponentForEntity(RectangleRenderingComponent::GetComponentMask(), m_InputTextboxID));
	if (active)
	{
		mainBackground->RenderIgnore		= false;
		outputText->RenderIgnore			= false;
		outputTextBackground->RenderIgnore	= false;
		inputText->RenderIgnore				= false;
		inputTextbackground->RenderIgnore	= false;
		inputText->StartEditing();
	}
	else
	{
		mainBackground->RenderIgnore		= true;
		outputText->RenderIgnore			= true;
		outputTextBackground->RenderIgnore	= true;
		inputText->RenderIgnore				= true;
		inputTextbackground->RenderIgnore	= true;
		inputText->StopEditing();
	}
	isActive = active;
}

// ---------- INTERNAL ----------

void MEngineConsole::Initialize()
{
	m_Commands = new CommandMap();

	m_OutputTextBoxOriginalHeight = GetWindowHeight() / 3;
	int32_t fullWidth = GetWindowWidth();

	m_BackgroundID = CreateEntity();
	AddComponentsToEntity( PosSizeComponent::GetComponentMask() | RectangleRenderingComponent::GetComponentMask(), m_BackgroundID);

	PosSizeComponent* posSize = static_cast<PosSizeComponent*>(GetComponentForEntity(PosSizeComponent::GetComponentMask(), m_BackgroundID));
	posSize->PosX	= 0;
	posSize->PosY	= 0;
	posSize->PosZ	= 1U;
	posSize->Width	= fullWidth;
	posSize->Height = m_OutputTextBoxOriginalHeight;

	RectangleRenderingComponent* background = static_cast<RectangleRenderingComponent*>(GetComponentForEntity(RectangleRenderingComponent::GetComponentMask(), m_BackgroundID));
	background->FillColor	= ColorData(0, 128, 0, 128);

	m_OutputTextboxID	= CreateTextBox(0, 0, fullWidth, m_OutputTextBoxOriginalHeight - INPUT_TEXTBOX_HEIGHT, 0U);
	m_InputTextboxID	= CreateTextBox(0, m_OutputTextBoxOriginalHeight - INPUT_TEXTBOX_HEIGHT, fullWidth, INPUT_TEXTBOX_HEIGHT, 0U, true, "", Colors[TRANSPARENT], Colors[BLUE]);

	SetConsoleActive(false);
}

void MEngineConsole::shutdown()
{
	delete m_Commands;
	DestroyEntity(m_BackgroundID);
	DestroyEntity(m_OutputTextboxID);
	DestroyEntity(m_InputTextboxID);
}

void MEngineConsole::Update()
{
	if (isActive)
	{
		const TextComponent* inputText = static_cast<const TextComponent*>(GetComponentForEntity(TextComponent::GetComponentMask(), m_InputTextboxID));
		const TextComponent* outputText = static_cast<const TextComponent*>(GetComponentForEntity(TextComponent::GetComponentMask(), m_OutputTextboxID));
		if (*inputText->Text != "" && IsInputString(inputText->Text) && KeyReleased(MKEY_MAIN_ENTER) || KeyReleased(MKEY_NUMPAD_ENTER))
		{
			std::string response;
			ExecuteCommand(*inputText->Text, &response);
			*outputText->Text += ">" + *inputText->Text + '\n' + " - " + response + '\n';

			*inputText->Text = "";
		}

		// Write new log messages
		std::string newMessages;
		if (MUtilityLog::FetchUnreadMessages(newMessages))
			*outputText->Text += newMessages;

		// Keep the output text within the console
		PosSizeComponent* outputTextPos = static_cast<PosSizeComponent*>(GetComponentForEntity(PosSizeComponent::GetComponentMask(), m_OutputTextboxID));
		uint16_t fullTextHeight = GetTextHeight(outputText->Text->c_str());
		int32_t diff = fullTextHeight + outputTextPos->PosY - m_OutputTextBoxOriginalHeight;
		if (diff > 0)
		{
			outputTextPos->PosY		-= diff;
			outputTextPos->Height	+= diff;
		}
	}

	// TODODB: This comma gets placed in the input string if the textbox is taking input; fix that
	if (KeyReleased(MKEY_NUMPAD_COMMA)) // TODODB: Check against some action here when such a system has been built 
		SetConsoleActive(!isActive);
}