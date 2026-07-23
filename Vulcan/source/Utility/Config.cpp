#include "Utility/Config.h"

#include <filesystem>
#include <fstream>

using std::ifstream;
using std::filesystem::path;

Config::Config(string name)
	: m_name{ std::move(name) }
{
	Load();
}

void Config::Load()
{
	// Construct the config file path
	path cfgPath = ".\\Config\\";
	cfgPath.append(m_name + ".json");

	// Validate that the config file exists
	if (!std::filesystem::exists(cfgPath))
	{
		return;
	}

	string cfgContents;

	// Open the config file for reading
	ifstream file;
	file.open(cfgPath);

	// Append each line individually, maintaining newlines
	string line;
	while (std::getline(file, line))
	{
		cfgContents += line + "\n";
	}

	// Close the file and parse the json
	file.close();
	m_json = Json::parse(cfgContents);
}