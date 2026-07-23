#pragma once

#include <sstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "Maths/Color.h"

using std::string;
using std::stringstream;
using std::vector;

using Json = nlohmann::json;

/**
 * @brief Loads and provides typed access to a named JSON configuration file.
 *
 * Config reads a JSON file by name on construction and exposes its values
 * through a typed Get() accessor. The file is loaded once via Load() and
 * cached for the lifetime of the object.
 */
class Config
{
public:
	/**
	 * @brief Constructs a Config and loads the associated JSON file.
	 * @param name The name of the configuration file to load, without extension.
	 */
	explicit Config(string name);

public:
	/**
	 * @brief Retrieves a value from the configuration by key, cast to the requested type.
	 *
	 * @tparam T   The type to retrieve the value as.
	 * @param  id  The key identifying the value within the JSON file.
	 * @return     The value associated with @p id, interpreted as type T.
	 */
	template<typename T>
	T Get(const string& id);

private:
	/** @brief The name of the configuration file, without extension. */
	string m_name;

	/** @brief The parsed JSON data loaded from the configuration file. */
	Json m_json;

private:
	/**
	 * @brief Reads and parses the JSON configuration file into m_json.
	 *
	 * Called once during construction. The filename is derived from m_name.
	 */
	void Load();

};

template <typename T>
T Config::Get(const string& id)
{
	// Simple helper lambda function to split a string into its parts by '.'
	auto splitStr = [](const string& str) -> vector<string>
		{
			vector<string> result;
			stringstream stream(str);
			string item;

			// Iterate over each element to the next '.'
			while (std::getline(stream, item, '.'))
			{
				result.emplace_back(item);
			}

			return result;
		};

	// Start at the root of the json file
	Json json = m_json;

	// Split string into its parts and iterate over them
	for (string part : splitStr(id))
	{
		// Try to get the child component
		if (json.contains(part))
		{
			json = json[part];
			continue;
		}

		// The part of the id we are looking for doesn't exist, so return default.
		return T{};
	}

	// Return the node data if it was found, otherwise the default value
	return json.is_null() ? T{} : json.get<T>();
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Color, r, g, b, a)