#pragma once

#include <stdexcept>
#include <string>

#include "Maths/Alias.h"

#include "Utility/EnumHelpers.h"

using std::exception;
using std::string;

enum class ELogLevel : uint8
{
	Debug = 0x01 << 0,
	Info = 0x01 << 1,
	Warning = 0x01 << 2,
	Error = 0x01 << 3,
	Exception = 0x01 << 4
};

ENUM_OPERATORS(ELogLevel, uint8)

class Console
{
private:
	static ELogLevel m_filter;

public:
	static void ToggleFilter(ELogLevel level);

	static void Debug(const string& message);
	static void Info(const string& message);
	static void Warning(const string& message);
	static void Error(const string& message);
	static void Exception(const string& message);
	static void Exception(const exception& except);

private:
	static void Log(ELogLevel level, const string& message);
	static string LevelString(ELogLevel level);
	static string LevelColor(ELogLevel level);

};