#include "Utility/Console.h"

#include <ctime>
#include <iostream>

ELogLevel Console::m_filter = ELogLevel::Debug | ELogLevel::Info | ELogLevel::Warning | ELogLevel::Error | ELogLevel::Exception;

void Console::ToggleFilter(const ELogLevel level)
{
	m_filter ^= level;
}

void Console::Debug(const string& message)
{
	Log(ELogLevel::Debug, message);
}

void Console::Info(const string& message)
{
	Log(ELogLevel::Info, message);
}

void Console::Warning(const string& message)
{
	Log(ELogLevel::Warning, message);
}

void Console::Error(const string& message)
{
	Log(ELogLevel::Error, message);
}

void Console::Exception(const string& message)
{
	Log(ELogLevel::Exception, message);
}

void Console::Exception(const exception& except)
{
	Log(ELogLevel::Exception, except.what());
}

void Console::Log(const ELogLevel level, const string& message)
{
	if ((level & m_filter) != level)
	{
		return;
	}

	time_t t = time(nullptr);
	tm timeInfo;
	(void)localtime_s(&timeInfo, &t);

	char timeBuffer[80];
	(void)strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &timeInfo);

	std::cout << LevelColor(level) << "[" << LevelString(level) << "]" <<
		"[" << timeBuffer << "]" << message << "\n\033[37m";
}

string Console::LevelString(const ELogLevel level)
{
	switch (level)
	{
		case ELogLevel::Debug:
		{
			return "DBG";
		}

		case ELogLevel::Info:
		{
			return "INF";
		}

		case ELogLevel::Warning:
		{
			return "WRN";
		}

		case ELogLevel::Error:
		{
			return "ERR";
		}

		case ELogLevel::Exception:
		{
			return "EXC";
		}

		default:
		{
			return "INV";
		}
	}
}

string Console::LevelColor(const ELogLevel level)
{
	switch (level)
	{
		case ELogLevel::Debug:
		{
			return "\033[37m";
		}

		case ELogLevel::Info:
		{
			return "\033[36m";
		}

		case ELogLevel::Warning:
		{
			return "\033[33m";
		}

		case ELogLevel::Error:
		{
			return "\033[91m";
		}

		case ELogLevel::Exception:
		{
			return "\033[95m";
		}

		default:
		{
			return "\033[37m";
		}
	}
}
