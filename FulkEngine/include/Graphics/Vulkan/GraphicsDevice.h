#pragma once

#include "Graphics/Vulkan/Common.h"

namespace Fulk
{
	class GraphicsDevice
	{
		friend class Vulkan;

	private:
		VkPhysicalDeviceProperties m_physicalProperties;
		VkPhysicalDeviceMemoryProperties m_memoryProperties;

		VkPhysicalDevice m_physical;
		VkDevice m_logical;

		VkQueue m_queue;
		uint32 m_queueFamily;

	private:
		explicit GraphicsDevice(VkInstance instance);
		~GraphicsDevice();

	public:
		[[nodiscard]] const VkPhysicalDeviceProperties& PhysicalProperties() const;
		[[nodiscard]] const VkPhysicalDeviceMemoryProperties& MemoryProperties() const;

		[[nodiscard]] const VkPhysicalDevice& Physical() const;
		[[nodiscard]] const VkDevice& Logical() const;

		[[nodiscard]] const VkQueue& Queue() const;
		[[nodiscard]] const uint32& QueueFamily() const;

		template<typename T>
		uint64 DynamicAlignment() const;

	};

	template <typename T>
	uint64 GraphicsDevice::DynamicAlignment() const
	{
		const uint64 minUboAlignment = m_physicalProperties.limits.minUniformBufferOffsetAlignment;
		uint64 dynamicAlignment = sizeof(T);
		if (minUboAlignment > 0)
		{
			dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
		}

		return dynamicAlignment;
	}
}
