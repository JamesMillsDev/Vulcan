#include "Graphics/Vulkan/MemoryBuffer.h"

#include "Graphics/Vulkan/Vulkan.h"

using namespace Vulcan;

uint32 MemoryBuffer::GetMemoryType(uint32 typeBits, const VkMemoryPropertyFlags properties, const Vulkan* vulkan, VkBool32* memTypeFound)
{
	const VkPhysicalDeviceMemoryProperties memoryProperties = vulkan->GetMemoryProperties();

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

MemoryBuffer::MemoryBuffer(const VkDeviceSize size, const VkBufferUsageFlags usage, const void* data, const VkSharingMode sharingMode, const VkMemoryPropertyFlags flags, const Vulkan* vulkan)
	: m_size{ size }, m_buffer{ VK_NULL_HANDLE }, m_usage{ usage }, m_flags{ flags },
	m_sharingMode{ sharingMode }, m_memory{ VK_NULL_HANDLE }, m_deviceAddress{ 0 }
{
	Create(data, vulkan);
}

MemoryBuffer::~MemoryBuffer()
{
	Destroy();
}

VkResult MemoryBuffer::Bind(const VkDeviceSize offset, const Vulkan* vulkan) const
{
	if (vulkan == nullptr)
	{
		vulkan = Vulkan::Instance();
	}

	return vkBindBufferMemory(vulkan->GetDevice(), m_buffer, m_memory, offset);
}

void MemoryBuffer::Fill(const void* data, VkDeviceSize size, const VkDeviceSize offset, bool shouldFlush)
{
	Map();

	size = size == 0 ? m_size : size;
	memcpy(static_cast<char*>(m_mapped) + offset, data, size);

	UnMap();
}

VkResult MemoryBuffer::Map(VkDeviceSize size, const VkDeviceSize offset, const Vulkan* vulkan)
{
	if (vulkan == nullptr)
	{
		vulkan = Vulkan::Instance();
	}

	size = size == 0 ? m_size : size;
	return vkMapMemory(vulkan->GetDevice(), m_memory, offset, size, 0, &m_mapped);
}

void MemoryBuffer::UnMap(const Vulkan* vulkan)
{
	if (vulkan == nullptr)
	{
		vulkan = Vulkan::Instance();
	}

	if (m_mapped != nullptr)
	{
		vkUnmapMemory(vulkan->GetDevice(), m_memory);
		m_mapped = nullptr;
	}
}

VkResult MemoryBuffer::Flush(VkDeviceSize size, const VkDeviceSize offset, const Vulkan* vulkan) const
{
	if (vulkan == nullptr)
	{
		vulkan = Vulkan::Instance();
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

	return vkFlushMappedMemoryRanges(vulkan->GetDevice(), 1, &mappedRange);
}

VkResult MemoryBuffer::Invalidate(VkDeviceSize size, const VkDeviceSize offset, const Vulkan* vulkan) const
{
	if (vulkan == nullptr)
	{
		vulkan = Vulkan::Instance();
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

	return vkInvalidateMappedMemoryRanges(vulkan->GetDevice(), 1, &mappedRange);
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

void MemoryBuffer::Create(const void* data, const Vulkan* vulkan)
{
	if (vulkan == nullptr)
	{
		vulkan = Vulkan::Instance();
	}

	VkResult result;
	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = m_size;
	createInfo.usage = m_usage;
	createInfo.sharingMode = m_sharingMode;

	// Attempt to allocate the memory
	if (result = vkCreateBuffer(vulkan->GetDevice(), &createInfo, nullptr, &m_buffer);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Memory Buffer!", result);
	}

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(vulkan->GetDevice(), m_buffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = GetMemoryType(memoryRequirements.memoryTypeBits, m_flags, vulkan);

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
		m_deviceAddress = vkGetBufferDeviceAddress(vulkan->GetDevice(), &deviceAddressInfo);

		allocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
		allocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		memoryAllocateInfo.pNext = &allocateFlagsInfo;
	}

	if (result = vkAllocateMemory(vulkan->GetDevice(), &memoryAllocateInfo, nullptr, &m_memory);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to allocate Memory Buffer!", result);
	}

	if (data != nullptr)
	{
		if (result = Map(0, 0, vulkan); result != VK_SUCCESS)
		{
			throw Vulkan::VulkanError("Failed to map Memory Buffer!", result);
		}

		memcpy(m_mapped, data, m_size);

		if ((m_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
		{
			(void)Flush(0, 0, vulkan);
		}

		UnMap(vulkan);
	}

	if (result = Bind(0, vulkan); result != VK_SUCCESS)
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
	UnMap();

	if (m_buffer != nullptr)
	{
		vkDestroyBuffer(Vulkan::Device(), m_buffer, nullptr);
		m_buffer = VK_NULL_HANDLE;
	}

	if (m_memory != nullptr)
	{
		vkFreeMemory(Vulkan::Device(), m_memory, nullptr);
		m_memory = VK_NULL_HANDLE;
	}
}