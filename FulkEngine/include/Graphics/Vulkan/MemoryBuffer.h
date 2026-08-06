#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Maths/Alias.h"

namespace Fulk
{
	class GraphicsDevice;
	class Vulkan;

	class MemoryBuffer
	{
	private:
		static uint32 GetMemoryType(uint32 typeBits, VkMemoryPropertyFlags properties, const GraphicsDevice* device, VkBool32* memTypeFound = nullptr);

	private:
		VkDeviceSize m_size;
		VkBuffer m_buffer;
		VmaAllocation m_allocation;
		VmaAllocationInfo m_allocationInfo;
		VkBufferUsageFlags m_usage;
		VkMemoryPropertyFlags m_flags;
		VkDeviceAddress m_deviceAddress;

		VkDescriptorBufferInfo m_bufferInfo;

	public:
		explicit MemoryBuffer(
			VkDeviceSize size, VkBufferUsageFlags usage, const void* data = nullptr, 
			VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			const GraphicsDevice* device = nullptr, const Vulkan* vulkan = nullptr
		);
		~MemoryBuffer();

	public:
		void Fill(const void* data, VkDeviceSize size = 0, size_t offset = 0) const;
		[[nodiscard]] const VkBuffer& Get() const;
		[[nodiscard]] const VkDescriptorBufferInfo& GetBufferInfo() const;
		[[nodiscard]] const VkDeviceAddress& GetAddress() const;
		[[nodiscard]] const VkDeviceSize& Size() const;

		void SetRange(uint64 range);

	private:
		void Create(const void* data, const GraphicsDevice* device = nullptr, const Vulkan* vulkan = nullptr);
		void Destroy();

		
	};
}

