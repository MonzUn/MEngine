#include "interface/mengineConfig.h"
#include "mengineConfigInternal.h"
#include "mengineGlobals.h"
#include <MUtilityFile.h>
#include <MUtilityLog.h>
#include <MUtilityString.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <unordered_map>

#define MUTILITY_LOG_CATEGORY_CONFIG "MEngineConfig"

namespace MEngineConfig
{
	std::string ConfigFilePath		= "NOT_SET";
	std::string ConfigDirectoryPath = "NOT_SET";
	std::unordered_map<std::string, ConfigEntry*> Entries;
}

// ---------- INTERFACE ----------

int64_t MEngineConfig::GetInt(const std::string& key, int64_t defaultValue)
{
	std::string keyCopy = key;
	std::transform(keyCopy.begin(), keyCopy.end(), keyCopy.begin(), ::tolower);
	auto iterator = Entries.find(keyCopy);
	if (iterator == Entries.end())
	{
		iterator = Entries.emplace(key, new ConfigEntry(ValueType::INTEGER, defaultValue)).first;
	}
	return iterator->second->Value.IntegerValue;
}

double MEngineConfig::GetDouble(const std::string& key, double defaultValue)
{
	std::string keyCopy = key;
	std::transform(keyCopy.begin(), keyCopy.end(), keyCopy.begin(), ::tolower);
	auto iterator = Entries.find(keyCopy);
	if (iterator == Entries.end())
	{
		iterator = Entries.emplace(key, new ConfigEntry(ValueType::DECIMAL, defaultValue)).first;
	}
	return iterator->second->Value.DecimalValue;
}

bool MEngineConfig::GetBool(const std::string& key, bool defaultValue)
{
	std::string keyCopy = key;
	std::transform(keyCopy.begin(), keyCopy.end(), keyCopy.begin(), ::tolower);
	auto iterator = Entries.find(keyCopy);
	if (iterator == Entries.end())
	{
		iterator = Entries.emplace(key, new ConfigEntry(ValueType::BOOLEAN, defaultValue)).first;
	}
	return iterator->second->Value.BooleanValue;
}

std::string MEngineConfig::GetString(const std::string& key, const std::string& defaultValue)
{
	std::string keyCopy = key;
	std::transform(keyCopy.begin(), keyCopy.end(), keyCopy.begin(), ::tolower);
	auto iterator = Entries.find(keyCopy);
	if (iterator == Entries.end())
	{
		char* newString = static_cast<char*>(malloc(defaultValue.size() + 1)); // +1 for null terminator
		strcpy(newString, defaultValue.c_str());
		iterator = Entries.emplace(key, new ConfigEntry(ValueType::STRING, newString)).first;
	}
	return iterator->second->Value.StringValue;
}

void MEngineConfig::SetInt(std::string key, int64_t value)
{
	std::string keyCopy = key;
	std::transform(keyCopy.begin(), keyCopy.end(), keyCopy.begin(), ::tolower);
	auto iterator = Entries.find(keyCopy);
	if (iterator != Entries.end())
	{
		if (iterator->second->Type == ValueType::INTEGER)
			iterator->second->Value.IntegerValue = value;
		else
			MLOG_WARNING("Attempted to assign non integer value to integer config entry; key = " << key, MUTILITY_LOG_CATEGORY_CONFIG);
	}
	else
		iterator = Entries.emplace(key, new ConfigEntry(ValueType::INTEGER, value)).first;
}

void MEngineConfig::SetDecimal(std::string key, double value)
{
	std::string keyCopy = key;
	std::transform(keyCopy.begin(), keyCopy.end(), keyCopy.begin(), ::tolower);
	auto iterator = Entries.find(keyCopy);
	if (iterator != Entries.end())
	{
		if (iterator->second->Type == ValueType::DECIMAL)
			iterator->second->Value.DecimalValue = value;
		else
			MLOG_WARNING("Attempted to assign non decimal value to decimal config entry; key = " << key, MUTILITY_LOG_CATEGORY_CONFIG);
	}
	else
		iterator = Entries.emplace(key, new ConfigEntry(ValueType::DECIMAL, value)).first;
		
}

void MEngineConfig::SetBool(std::string key, bool value)
{
	std::string keyCopy = key;
	std::transform(keyCopy.begin(), keyCopy.end(), keyCopy.begin(), ::tolower);
	auto iterator = Entries.find(keyCopy);
	if (iterator != Entries.end())
	{
		if (iterator->second->Type == ValueType::BOOLEAN)
			iterator->second->Value.BooleanValue = value;
		else
			MLOG_WARNING("Attempted to assign non boolean value to boolean config entry; key = " << key, MUTILITY_LOG_CATEGORY_CONFIG);
	}
	else
		iterator = Entries.emplace(key, new ConfigEntry(ValueType::BOOLEAN, value)).first;
}

void MEngineConfig::SetString(std::string key, const std::string& value)
{
	char* newString = static_cast<char*>(malloc(key.size() + 1)); // +1 for null terminator
	strcpy(newString, value.c_str());

	std::string keyCopy = key;
	std::transform(keyCopy.begin(), keyCopy.end(), keyCopy.begin(), ::tolower);
	auto iterator = Entries.find(keyCopy);
	if (iterator != Entries.end())
	{
		if (iterator->second->Type == ValueType::STRING)
		{
			free(iterator->second->Value.StringValue);
			iterator->second->Value.StringValue = newString;
		}
		else
			MLOG_WARNING("Attempted to assign non string value to string config entry; key = " << key, MUTILITY_LOG_CATEGORY_CONFIG);
	}
	else
		iterator = Entries.emplace(key, new ConfigEntry(ValueType::STRING, newString)).first;
}

void MEngineConfig::WriteConfigFile()
{
	std::stringstream stringStream;
	for (auto& keyAndEntry : Entries)
	{
		stringStream << keyAndEntry.first << " = ";

		const ConfigEntry::ValueContainer& value = keyAndEntry.second->Value;
		switch (keyAndEntry.second->Type)
		{
			case ValueType::INTEGER:
			{
				stringStream << value.IntegerValue;
			} break;

			case ValueType::DECIMAL:
			{
				stringStream << value.DecimalValue;
				if (value.DecimalValue == std::floor(value.DecimalValue)) // If the value is without decimal part; write a decimal so that the parser handles it as a decimal value and not an integer value
					stringStream << ".0";
			} break;

			case ValueType::BOOLEAN:
			{
				stringStream << value.BooleanValue ? "true" : "false";
			} break;

			case ValueType::STRING:
			{
				stringStream << "\"" << value.StringValue << "\"";
			} break;

			case ValueType::INVALID:
			{
				MLOG_WARNING("Config entries contained entry with invalid type", MUTILITY_LOG_CATEGORY_CONFIG);
			} break;
			
		default:
			MLOG_WARNING("Config entries contained entry with unknown type", MUTILITY_LOG_CATEGORY_CONFIG);
			break;
		}

		stringStream << std::endl;
	}
	
	if (!MUtilityFile::DirectoryExists(ConfigDirectoryPath.c_str()))
		MUtilityFile::CreateDir(ConfigDirectoryPath.c_str());

	std::ofstream outstream = std::ofstream(ConfigFilePath, std::ofstream::out | std::ofstream::trunc);
	if (outstream.is_open())
	{
		outstream << stringStream.str();
		outstream.close();
	}
}

void MEngineConfig::ReadConfigFile()
{
	if (!MUtilityFile::DirectoryExists(ConfigDirectoryPath.c_str()))
	{
		MLOG_WARNING("Config file directory does not exist; Path = " << ConfigDirectoryPath, MUTILITY_LOG_CATEGORY_CONFIG);
		return;
	}

	if (!MUtilityFile::FileExists(ConfigFilePath.c_str()))
	{
		MLOG_WARNING("Config file does not exist; Path = " << ConfigFilePath, MUTILITY_LOG_CATEGORY_CONFIG);
		return;
	}

	std::stringstream stringStream;
	stringStream << MUtilityFile::GetFileContentAsString(ConfigFilePath);

	while (stringStream.good())
	{
		std::string line, key, value = "";
		getline(stringStream, line);
		if (line == "")
			continue;

		std::transform(line.begin(), line.end(), line.begin(), ::tolower);
		line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end()); // Strip whitespaces
		
		size_t dividerPos = line.find("=");
		if (dividerPos == 0)
		{
			MLOG_WARNING("Found config line with missing key; line = " << line, MUTILITY_LOG_CATEGORY_CONFIG);
			continue;
		}
		if (dividerPos == std::string::npos)
		{
			MLOG_WARNING("Found config line without divider; line = " << line, MUTILITY_LOG_CATEGORY_CONFIG);
			continue;
		}
		if (dividerPos == line.length() - 1)
		{
			MLOG_WARNING("Found config line without key; line = " << line, MUTILITY_LOG_CATEGORY_CONFIG);
			continue;
		}

		key = line.substr(0, dividerPos);
		value = line.substr(dividerPos + 1, line.length() - dividerPos);
		
		ValueType::ValueType type = ValueType::INVALID;
		if (value[0] == '\"' || value[value.size() - 1] == '\"')
		{
			if (value[0] != '\"')
			{
				MLOG_WARNING("Found config string missing initial \" character; line = " << line, MUTILITY_LOG_CATEGORY_CONFIG);
				continue;
			}
			else if (value[value.size() - 1] != '\"')
			{
				MLOG_WARNING("Found config string missing ending \" character; line = " << line, MUTILITY_LOG_CATEGORY_CONFIG);
				continue;
			}

			value.erase(std::remove(value.begin(), value.end(), '\"'), value.end()); // Remove the "" from the string
			char* stringValue = static_cast<char*>(malloc(value.size() + 1)); // +1 for the null terminator
			strcpy(stringValue, value.c_str());
			Entries.emplace(key, new ConfigEntry(ValueType::STRING, stringValue));
		}
		else if (MUtilityString::IsStringNumber(value))
		{
			int64_t intValue = strtoll(value.c_str(), nullptr, 0);
			Entries.emplace(key, new ConfigEntry(ValueType::INTEGER, intValue));
		}
		else if (value == "true" || value == "false")
		{
			bool boolValue = (value == "true");
			Entries.emplace(key, new ConfigEntry(ValueType::BOOLEAN, boolValue));
		}
		else if (MUtilityString::IsStringNumberExcept(value, '.', 1) || MUtilityString::IsStringNumberExcept(value, ',', 1))
		{
			double doubleValue = std::stod(value);
			Entries.emplace(key, new ConfigEntry(ValueType::DECIMAL, doubleValue));
		}
		else
			MLOG_WARNING("Unable to determine value type of config line; line = " << line, MUTILITY_LOG_CATEGORY_CONFIG);
	}
}

void MEngineConfig::SetConfigFilePath(const std::string& relativeFilePathAndName)
{
	ConfigFilePath		= Globals::EXECUTABLE_PATH + '/' + relativeFilePathAndName + CONFIG_EXTENSION;
	ConfigDirectoryPath = MUtilityFile::GetDirectoryPathFromFilePath(ConfigFilePath);
}

// ---------- INTERNAL ----------

void MEngineConfig::Initialize()
{
	SetConfigFilePath(DEFAULT_CONFIG_FILE_RELATIVE_PATH);
	ReadConfigFile();
}

void MEngineConfig::Shutdown()
{
	WriteConfigFile();
	for (auto& keyAndValue : Entries)
	{
		delete keyAndValue.second;
	}
	Entries.clear();
}