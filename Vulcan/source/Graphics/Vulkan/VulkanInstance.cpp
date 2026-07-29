#include "Graphics/Vulkan/VulkanInstance.h"

#include <GLFW/glfw3.h>

#include "Graphics/Vulkan/Vulkan.h"

#include "Utility/Config.h"
#include "Utility/Console.h"
#include "Utility/Version.h"

using namespace Vulcan;

namespace
{
	VkDebugUtilsMessageSeverityFlagBitsEXT messageLevel = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		if (messageSeverity >= messageLevel)
		{
			switch (messageSeverity)  // NOLINT(clang-diagnostic-switch-enum)
			{
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				{
					Console::Debug(pCallbackData->pMessage);
					break;
				}
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				{
					Console::Info(pCallbackData->pMessage);
					break;
				}
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				{
					Console::Warning(pCallbackData->pMessage);
					break;
				}
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				{
					Console::Error(pCallbackData->pMessage);
					break;
				}
				default: break;
			}
		}

		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(const VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		PFN_vkCreateDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(  // NOLINT(clang-diagnostic-cast-function-type-strict)
			vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
			);
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}

		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void DestroyDebugUtilsMessengerEXT(const VkInstance instance, const VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		PFN_vkDestroyDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(  // NOLINT(clang-diagnostic-cast-function-type-strict)
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
			);
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}
}

bool VulkanInstance::CheckValidationLayerSupport()
{
	// Count the number of available layers
	uint32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	// Get the layers from the driver, storing them in a vector
	TList<VkLayerProperties> availableLayers;
	availableLayers.Resize(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.Data());

	// Iterate over the layers we want to use
	for (const char* layerName : VALIDATION_LAYERS)
	{
		bool layerFound = false;

		// Iterate over the layers that the driver supports
		for (const VkLayerProperties& layerProperties : availableLayers)
		{
			// Validate the names match for this property, if they do, mark it as found and break
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		// If the layer still hasn't been found, we are trying to enable a layer that is
		// unsupported
		if (!layerFound)
		{
			return false;
		}
	}

	// All requested layers are supported
	return true;
}

void VulkanInstance::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
}

VulkanInstance::VulkanInstance(Config* config)
{
	m_appName = config->Get<string>("Application.Title");
	m_appVersion = new Version{ "Application.Version", config };
	m_engineName = config->Get<string>("Engine.Title");
	m_engineVersion = new Version{ "Engine.Version", config };

	CreateVkInstance();

	// Debug Messenger
	if constexpr (ENABLE_VALIDATION_LAYERS)
	{
		CreateDebugMessenger();
	}
}

VulkanInstance::~VulkanInstance()
{
	if constexpr (ENABLE_VALIDATION_LAYERS)
	{
		DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
	}

	vkDestroyInstance(m_vkInstance, nullptr);

	delete m_appVersion;
	delete m_engineVersion;
}

const VkInstance& VulkanInstance::Get() const
{
	return m_vkInstance;
}

void VulkanInstance::CreateVkInstance()
{
	const VkApplicationInfo appInfo
	{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = nullptr,
		.pApplicationName = m_appName.c_str(),
		.applicationVersion = VK_MAKE_VERSION(m_appVersion->major, m_appVersion->minor, m_appVersion->patch),
		.pEngineName = m_engineName.c_str(),
		.engineVersion = VK_MAKE_VERSION(m_engineVersion->major, m_engineVersion->minor, m_engineVersion->patch),
		.apiVersion = VK_API_VERSION_1_3
	};

	uint32 instanceExtensionsCount = 0;
	const char** instanceExtensions = glfwGetRequiredInstanceExtensions(&instanceExtensionsCount);

	vector<const char*> extensions;
	for (uint32 i = 0; i < instanceExtensionsCount; ++i)
	{
		extensions.emplace_back(instanceExtensions[i]);
	}

	if constexpr (ENABLE_VALIDATION_LAYERS)
	{
		// Add in the extra required extensions
		extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	VkInstanceCreateInfo instanceInfo{};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledExtensionCount = static_cast<uint32>(extensions.size());
	instanceInfo.ppEnabledExtensionNames = extensions.data();

	if constexpr (ENABLE_VALIDATION_LAYERS)
	{
		// Add the layers into the creation info if we requested it (Debug only)
		instanceInfo.enabledLayerCount = static_cast<uint32>(VALIDATION_LAYERS.size());
		instanceInfo.ppEnabledLayerNames = VALIDATION_LAYERS.Data();

		// Set the next create info to be the debug messenger
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		instanceInfo.pNext = &debugCreateInfo;
	}

	Try(
		vkCreateInstance(&instanceInfo, nullptr, &m_vkInstance),
		"Failed to create Vulkan Instance!"
	);
}

void VulkanInstance::CreateDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);

	Try(
		CreateDebugUtilsMessengerEXT(m_vkInstance, &createInfo, nullptr, &m_debugMessenger),
		"Failed to create Debug Messenger!"
	);
}