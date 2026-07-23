#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

class Vulkan;

class MemoryBuffer
{
private:
	VkDeviceSize m_size;
	VkBuffer m_buffer;
	VmaAllocation m_allocation;
	VmaAllocationInfo m_allocationInfo;
	VkBufferUsageFlags m_usage;
	VkDeviceAddress m_deviceAddress;

	VkDescriptorBufferInfo m_bufferInfo;

public:
	explicit MemoryBuffer(VkDeviceSize size, VkBufferUsageFlags usage, Vulkan* vulkan);
	~MemoryBuffer();

public:
	void Fill(const void* data, VkDeviceSize size = 0, size_t offset = 0) const;
	[[nodiscard]] const VkBuffer& Get() const;
	[[nodiscard]] const VkDescriptorBufferInfo& GetBufferInfo() const;
	[[nodiscard]] const VkDeviceAddress& GetAddress() const;
	[[nodiscard]] const VkDeviceSize& Size() const;

private:
	void Create(const Vulkan* vulkan);
	void Destroy();

};

