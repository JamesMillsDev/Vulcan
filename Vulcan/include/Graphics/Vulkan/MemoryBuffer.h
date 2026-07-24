#pragma once

#include <vulkan/vulkan.h>

#include "Maths/Alias.h"

namespace Vulcan
{
	class Vulkan;

	class MemoryBuffer
	{
	private:
		static uint32 GetMemoryType(uint32 typeBits, VkMemoryPropertyFlags properties, const Vulkan* vulkan, VkBool32* memTypeFound = nullptr);

	private:
		VkDeviceSize m_size;
		VkBuffer m_buffer;

		VkBufferUsageFlags m_usage;
		VkMemoryPropertyFlags m_flags;
		VkSharingMode m_sharingMode;

		VkDeviceMemory m_memory;
		void* m_mapped;

		VkDeviceAddress m_deviceAddress;
		VkDescriptorBufferInfo m_bufferInfo;

	public:
		explicit MemoryBuffer(VkDeviceSize size, VkBufferUsageFlags usage, const void* data = nullptr, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, const Vulkan* vulkan = nullptr);
		~MemoryBuffer();

	public:
		VkResult Bind(VkDeviceSize offset = 0, const Vulkan* vulkan = nullptr) const;
		void Fill(const void* data, VkDeviceSize size = 0, VkDeviceSize offset = 0, bool shouldFlush = false);

		VkResult Map(VkDeviceSize size = 0, VkDeviceSize offset = 0, const Vulkan* vulkan = nullptr);
		void UnMap(const Vulkan* vulkan = nullptr);

		VkResult Flush(VkDeviceSize size = 0, VkDeviceSize offset = 0, const Vulkan* vulkan = nullptr) const;
		VkResult Invalidate(VkDeviceSize size = 0, VkDeviceSize offset = 0, const Vulkan* vulkan = nullptr) const;

		[[nodiscard]] const VkBuffer& Get() const;
		[[nodiscard]] const VkDescriptorBufferInfo& GetBufferInfo() const;
		[[nodiscard]] const VkDeviceAddress& GetAddress() const;
		[[nodiscard]] const VkDeviceSize& Size() const;

	private:
		void Create(const void* data, const Vulkan* vulkan = nullptr);
		void Destroy();

		
	};
}

