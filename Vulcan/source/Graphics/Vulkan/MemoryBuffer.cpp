#include "Graphics/Vulkan/MemoryBuffer.h"

#include "Graphics/Vulkan/GraphicsDevice.h"
#include "Graphics/Vulkan/Vulkan.h"

using namespace Vulcan;

uint32 MemoryBuffer::GetMemoryType(uint32 typeBits, const VkMemoryPropertyFlags properties, const GraphicsDevice* device, VkBool32* memTypeFound)
{
	const VkPhysicalDeviceMemoryProperties memoryProperties = device->MemoryProperties();

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				if (memTypeFound)
				{
					*memTypeFound = true;
				}
				return i;
			}
		}
		typeBits >>= 1;
	}

	if (memTypeFound)
	{
		*memTypeFound = false;
		return 0;
	}

	throw std::runtime_error("Could not find a matching memory type");
}

MemoryBuffer::MemoryBuffer(const VkDeviceSize size, const VkBufferUsageFlags usage, const void* data, const VkSharingMode sharingMode, const VkMemoryPropertyFlags flags, const GraphicsDevice* device)
	: m_size{ size }, m_buffer{ VK_NULL_HANDLE }, m_usage{ usage }, m_flags{ flags },
	m_sharingMode{ sharingMode }, m_memory{ VK_NULL_HANDLE }, m_deviceAddress{ 0 }
{
	Create(data, device);
}

MemoryBuffer::~MemoryBuffer()
{
	Destroy();
}

VkResult MemoryBuffer::Bind(const VkDeviceSize offset, const GraphicsDevice* device) const
{
	if (device == nullptr)
	{
		device = Vulkan::Device();
	}

	return vkBindBufferMemory(device->Logical(), m_buffer, m_memory, offset);
}

void MemoryBuffer::Fill(const void* data, VkDeviceSize size, const VkDeviceSize offset, bool shouldFlush)
{
	Map();

	size = size == 0 ? m_size : size;
	memcpy(static_cast<char*>(m_mapped) + offset, data, size);

	UnMap();
}

VkResult MemoryBuffer::Map(VkDeviceSize size, const VkDeviceSize offset, const GraphicsDevice* device)
{
	if (device == nullptr)
	{
		device = Vulkan::Device();
	}

	size = size == 0 ? m_size : size;
	return vkMapMemory(device->Logical(), m_memory, offset, size, 0, &m_mapped);
}

void MemoryBuffer::UnMap(const GraphicsDevice* device)
{
	if (device == nullptr)
	{
		device = Vulkan::Device();
	}

	if (m_mapped != nullptr)
	{
		vkUnmapMemory(device->Logical(), m_memory);
		m_mapped = nullptr;
	}
}

VkResult MemoryBuffer::Flush(VkDeviceSize size, const VkDeviceSize offset, const GraphicsDevice* device) const
{
	if (device == nullptr)
	{
		device = Vulkan::Device();
	}

	size = size == 0 ? m_size : size;

	const VkMappedMemoryRange mappedRange =
	{
			.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
			.pNext = nullptr,
			.memory = m_memory,
			.offset = offset,
			.size = size
	};

	return vkFlushMappedMemoryRanges(device->Logical(), 1, &mappedRange);
}

VkResult MemoryBuffer::Invalidate(VkDeviceSize size, const VkDeviceSize offset, const GraphicsDevice* device) const
{
	if (device == nullptr)
	{
		device = Vulkan::Device();
	}

	size = size == 0 ? m_size : size;

	const VkMappedMemoryRange mappedRange =
	{
			.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
			.pNext = nullptr,
			.memory = m_memory,
			.offset = offset,
			.size = size
	};

	return vkInvalidateMappedMemoryRanges(device->Logical(), 1, &mappedRange);
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

void MemoryBuffer::Create(const void* data, const GraphicsDevice* device)
{
	if (device == nullptr)
	{
		device = Vulkan::Device();
	}

	VkResult result;
	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = m_size;
	createInfo.usage = m_usage;
	createInfo.sharingMode = m_sharingMode;

	// Attempt to allocate the memory
	if (result = vkCreateBuffer(device->Logical(), &createInfo, nullptr, &m_buffer);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Memory Buffer!", result);
	}

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device->Logical(), m_buffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = GetMemoryType(memoryRequirements.memoryTypeBits, m_flags, device);

	VkMemoryAllocateFlagsInfoKHR allocateFlagsInfo{};

	if (m_usage == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
	{
		// Attempt to get the device address
		const VkBufferDeviceAddressInfo deviceAddressInfo
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.pNext = nullptr,
			.buffer = m_buffer
		};
		m_deviceAddress = vkGetBufferDeviceAddress(device->Logical(), &deviceAddressInfo);

		allocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
		allocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		memoryAllocateInfo.pNext = &allocateFlagsInfo;
	}

	if (result = vkAllocateMemory(device->Logical(), &memoryAllocateInfo, nullptr, &m_memory);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to allocate Memory Buffer!", result);
	}

	if (data != nullptr)
	{
		if (result = Map(0, 0, device); result != VK_SUCCESS)
		{
			throw Vulkan::VulkanError("Failed to map Memory Buffer!", result);
		}

		memcpy(m_mapped, data, m_size);

		if ((m_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
		{
			(void)Flush(0, 0, device);
		}

		UnMap(device);
	}

	if (result = Bind(0, device); result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to bind buffer memory!", result);
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
	const GraphicsDevice* device = Vulkan::Device();

	UnMap();

	if (m_buffer != nullptr)
	{
		vkDestroyBuffer(device->Logical(), m_buffer, nullptr);
		m_buffer = VK_NULL_HANDLE;
	}

	if (m_memory != nullptr)
	{
		vkFreeMemory(device->Logical(), m_memory, nullptr);
		m_memory = VK_NULL_HANDLE;
	}
}