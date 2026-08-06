#pragma once

#include <queue>
#include <string>

#include "Maths/Alias.h"

#include "Utility/Collections/TMap.h"

using std::queue;
using std::string;

namespace Fulk
{
	class Config;

	struct ResourceData
	{
		uint8* data;
		int32 length;
	};

	class ResourceIdQueue
	{
	private:
		int32 m_nextId;
		queue<int32> m_freeIds;

	public:
		ResourceIdQueue();

	public:
		int32 Request();
		void Return(int32 id);

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
		static ResourceIdQueue m_textureIdQueue; /**< the queue of texture resource ids. */

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

		static int32 RequestNewTextureId();
		static void ReturnTextureId(int32 id);

	private:
		static void Init(Config* config);
		static void Shutdown();

	};
}