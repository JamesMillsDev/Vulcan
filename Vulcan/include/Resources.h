#pragma once

#include <string>

#include "Maths/Alias.h"

#include "Utility/Collections/TMap.h"

using std::string;

class Config;

struct ResourceData
{
	uint8* data;
	int32 length;
};

/** 
 * @brief A manager for the resource files of the game. 
 * 
 * @details 
 * Handles the loading and unloading of raw binary data from the 
 * Resource files using a mapping file. The resource data is 
 * generated when the application is built using a Python script.
 */
class Resources
{
	friend class Application;

private:
	static TMap<string, uint32> m_fileMappings; /**< The mappings of all resource paths to the correct resource files. */
	static TMap<string, ResourceData> m_resources; /**< The loaded resource data. This prevents having to re-read the resource files. */
	static string m_resourceDir; /**< The directory that any resource files are stored in. */
	static string m_resourceFileName; /**< The name of any resource / resource mapping files. */

public:
	/**
	 * @brief Attempts to load raw binary data of a resource.
	 * 
	 * @details
	 * Attempts to return binary data of a resource from either a 
	 * previously loaded version, or will read the raw resource
	 * files.
	 * 
	 * @param id The id of the resource.
	 * @return The raw binary data of the resource.
	 */
	static ResourceData& Find(string id);

private:
	static void Init(Config* config);
	static void Shutdown();

};