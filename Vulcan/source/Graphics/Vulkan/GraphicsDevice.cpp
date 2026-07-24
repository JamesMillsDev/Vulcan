#include "Graphics/Vulkan/GraphicsDevice.h"

#include <stdexcept>
#include <string>
#include <vector>
#include <GLFW/glfw3.h>

#include "Graphics/Vulkan/Vulkan.h"

using std::runtime_error;
using std::string;
using std::vector;

using namespace Vulcan;

GraphicsDevice::GraphicsDevice(VkInstance instance)
	: m_physicalProperties{}, m_memoryProperties{}, m_physical{ VK_NULL_HANDLE },
	m_logical{ VK_NULL_HANDLE }, m_queue{ VK_NULL_HANDLE }, m_queueFamily{ 0 }
{
	// Get a list and number of all GPU's attached to the computer
	uint32 deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	// Get the properties of the device
	m_physical = devices[0];
	VkPhysicalDeviceProperties2 deviceProperties
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
		.pNext = nullptr,
		.properties = {}
	};
	vkGetPhysicalDeviceProperties2(m_physical, &deviceProperties);
	m_physicalProperties = deviceProperties.properties;

	vkGetPhysicalDeviceMemoryProperties(m_physical, &m_memoryProperties);

	// Get all the device's queue families
	uint32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physical, &queueFamilyCount, nullptr);
	vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physical, &queueFamilyCount, queueFamilies.data());

	// Get the graphics queue family index
	m_queueFamily = 0;
	for (uint32 i = 0; i < queueFamilyCount; ++i)
	{
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			m_queueFamily = i;
			break;
		}
	}

	// Validate the queue family support
	if (glfwGetPhysicalDevicePresentationSupport(instance, m_physical, m_queueFamily) == GLFW_FALSE)
	{
		throw runtime_error("GLFW does not support presentation on this queue family!");
	}

	// Generate the queue create info with a 100% priority
	constexpr float qfPriorities = 1.f;
	VkDeviceQueueCreateInfo queueCI
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.queueFamilyIndex = m_queueFamily,
		.queueCount = 1,
		.pQueuePriorities = &qfPriorities,
	};

	// Generate the feature and extension supports we need
	const vector deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VkPhysicalDeviceVulkan11Features enabledVk11Features{};
	enabledVk11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	enabledVk11Features.variablePointers = true;
	enabledVk11Features.variablePointersStorageBuffer = true;

	VkPhysicalDeviceVulkan12Features enabledVk12Features{};
	enabledVk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	enabledVk12Features.pNext = &enabledVk11Features;
	enabledVk12Features.descriptorIndexing = true;
	enabledVk12Features.shaderSampledImageArrayNonUniformIndexing = true;
	enabledVk12Features.descriptorBindingUniformBufferUpdateAfterBind = true;
	enabledVk12Features.descriptorBindingVariableDescriptorCount = true;
	enabledVk12Features.runtimeDescriptorArray = true;
	enabledVk12Features.bufferDeviceAddress = true;
	enabledVk12Features.descriptorBindingSampledImageUpdateAfterBind = true;
	enabledVk12Features.descriptorBindingPartiallyBound = true;

	VkPhysicalDeviceVulkan13Features enabledVk13Features{};
	enabledVk13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	enabledVk13Features.pNext = &enabledVk12Features;
	enabledVk13Features.synchronization2 = true;
	enabledVk13Features.dynamicRendering = true;

	VkPhysicalDeviceFeatures enabledVk10Features{};
	enabledVk10Features.samplerAnisotropy = true;

	// Generate the device create info with the above parameters
	const VkDeviceCreateInfo deviceCI
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &enabledVk13Features,
		.flags = 0,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueCI,
		.enabledLayerCount = 0, // DEPRECATED
		.ppEnabledLayerNames = nullptr, // DEPRECATED
		.enabledExtensionCount = static_cast<uint32>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data(),
		.pEnabledFeatures = &enabledVk10Features
	};

	// Attempt to create the device and get the graphics queue
	Try(
		vkCreateDevice(m_physical, &deviceCI, nullptr, &m_logical),
		"Failed to create Logical Device!"
	);
	vkGetDeviceQueue(m_logical, m_queueFamily, 0, &m_queue);
}

GraphicsDevice::~GraphicsDevice()
{
	vkDestroyDevice(m_logical, nullptr);
}

const VkPhysicalDeviceProperties& GraphicsDevice::PhysicalProperties() const
{
	return m_physicalProperties;
}

const VkPhysicalDeviceMemoryProperties& GraphicsDevice::MemoryProperties() const
{
	return m_memoryProperties;
}

const VkPhysicalDevice& GraphicsDevice::Physical() const
{
	return m_physical;
}

const VkDevice& GraphicsDevice::Logical() const
{
	return m_logical;
}

const VkQueue& GraphicsDevice::Queue() const
{
	return m_queue;
}

const uint32& GraphicsDevice::QueueFamily() const
{
	return m_queueFamily;
}