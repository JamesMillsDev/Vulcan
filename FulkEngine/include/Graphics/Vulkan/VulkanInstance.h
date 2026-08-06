#pragma once

#include <string>
#include <vk_mem_alloc.h>

#include "Graphics/Vulkan/Common.h"

using std::string;

namespace Fulk
{
	class Config;
	class Version;

#ifdef _DEBUG
	constexpr bool ENABLE_VALIDATION_LAYERS = true;
#else
	constexpr bool ENABLE_VALIDATION_LAYERS = false;
#endif

	const TList VALIDATION_LAYERS =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	class VulkanInstance
	{
		friend Vulkan;

	private:
		static bool CheckValidationLayerSupport();
		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	private:
		Version* m_appVersion;
		string m_appName;
		Version* m_engineVersion;
		string m_engineName;

		VmaAllocator m_vmaAllocator;
		VkInstance m_vkInstance;
		VkDebugUtilsMessengerEXT m_debugMessenger;

	private:
		explicit VulkanInstance(Config* config);
		~VulkanInstance();

	public:
		const VkInstance& Get() const;

	private:
		void CreateVkInstance();
		void CreateDebugMessenger();

	};
}