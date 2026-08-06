#pragma once

#include <string>

using std::string;

namespace Fulk
{
	class Config;

	class Version
	{
	public:
		int major;
		int minor;
		int patch;

	public:
		Version(const string& name, Config* config);

	};
}