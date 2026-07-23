#include "Graphics/Vulkan/MemoryBuffer.h"

#include "Graphics/Vulkan/Vulkan.h"

MemoryBuffer::MemoryBuffer(const VkDeviceSize size, const VkBufferUsageFlags usage, Vulkan* vulkan)
	: m_size{ size }, m_buffer{ VK_NULL_HANDLE }, m_allocation{ VK_NULL_HANDLE },
	m_usage{ usage }, m_deviceAddress{ 0 }
{
	Create(vulkan);
}

MemoryBuffer::~MemoryBuffer()
{
	Destroy();
}

void MemoryBuffer::Fill(const void* data, VkDeviceSize size, const size_t offset) const
{
	size = size == 0 ? m_size : size;
	memcpy(static_cast<char*>(m_allocationInfo.pMappedData) + offset, data, size);
}

const VkBuffer& MemoryBuffer::Get() const
{
	return m_buffer;
}

const VkDescriptorBufferInfo& MemoryBuffer::GetBufferInfo() const
{
	return m_bufferInfo;
}

const VkDeviceAddress& MemoryBuffer::GetAddress() const
{
	return m_deviceAddress;
}

const VkDeviceSize& MemoryBuffer::Size() const
{
	return m_size;
}

void MemoryBuffer::Create(const Vulkan* vulkan)
{
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = m_size;
	bufferCreateInfo.usage = m_usage;

	VmaAllocationCreateInfo bufferAllocCreateInfo{};
	bufferAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
		VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
		VMA_ALLOCATION_CREATE_MAPPED_BIT;
	bufferAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

	// Attempt to allocate the memory
	if (const VkResult result = vmaCreateBuffer(vulkan->GetAllocator(), &bufferCreateInfo, &bufferAllocCreateInfo, &m_buffer, &m_allocation, &m_allocationInfo);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to allocate Mesh Buffer!", result);
	}

	if (m_usage == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
	{
		// Attempt to get the device address
		const VkBufferDeviceAddressInfo deviceAddressInfo
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.pNext = nullptr,
			.buffer = m_buffer
		};
		m_deviceAddress = vkGetBufferDeviceAddress(vulkan->GetDevice(), &deviceAddressInfo);
	}

	m_bufferInfo =
	{
		.buffer = m_buffer,
		.offset = 0,
		.range = m_size
	};
}

void MemoryBuffer::Destroy()
{
	vmaDestroyBuffer(Vulkan::Allocator(), m_buffer, m_allocation);

	m_buffer = VK_NULL_HANDLE;
	m_allocation = VK_NULL_HANDLE;

	m_allocationInfo.pMappedData = nullptr;
}