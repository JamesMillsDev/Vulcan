#include "Graphics/Vulkan/CommandManager.h"

#include <format>

#include "Graphics/Vulkan/GraphicsDevice.h"
#include "Graphics/Vulkan/Vulkan.h"

using namespace Vulcan;

VkCommandBuffer CommandManager::GetFrameCommandBuffer(const uint32 frameIndex) const
{
	const VkCommandBuffer cmdBuf = m_commandBuffers[frameIndex];

	Try(
		vkResetCommandBuffer(cmdBuf, 0),
		std::format("Failed to reset Command Buffer for frame: {}!", frameIndex)
	);

	// Begin using the command buffer
	VkCommandBufferBeginInfo cbBeginInfo{};
	cbBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	Try(
		vkBeginCommandBuffer(cmdBuf, &cbBeginInfo),
		std::format("Failed to begin Command Buffer for frame: {}!", frameIndex)
	);

	return m_commandBuffers[frameIndex];
}

void CommandManager::BeginOneTimeCommand(VkCommandBuffer& buffer, VkFence& fence) const
{
	const GraphicsDevice* device = Vulkan::Device();

	// Attempt to create the one-time fence
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	Try(
		vkCreateFence(device->Logical(), &fenceCreateInfo, nullptr, &fence),
		"Failed to create One-Time Fence!"
	);

	// Attempt to allocate one-time command buffer
	VkCommandBufferAllocateInfo cbAllocateInfo{};
	cbAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocateInfo.commandPool = m_commandPool;
	cbAllocateInfo.commandBufferCount = 1;

	Try(
		vkAllocateCommandBuffers(device->Logical(), &cbAllocateInfo, &buffer),
		"Failed to create One-Time Command Buffer!"
	);

	// Attempt to begin the command buffer
	VkCommandBufferBeginInfo cbBeginInfo{};
	cbBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	Try(
		vkBeginCommandBuffer(buffer, &cbBeginInfo),
		"Failed to begin One-Time Command Buffer!"
	);
}

void CommandManager::EndOneTimeCommand(const VkCommandBuffer& buffer, const VkFence& fence) const
{
	const GraphicsDevice* device = Vulkan::Device();

	// Attempt to end the command buffer
	Try(
		vkEndCommandBuffer(buffer),
		"Failed to end One-Time Command Buffer!"
	);

	// Attempt to submit the queue
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &buffer;

	Try(
		vkQueueSubmit(device->Queue(), 1, &submitInfo, fence),
		"Failed to submit One-Time Command!"
	);

	// Wait for the fences to finish
	Try(
		vkWaitForFences(device->Logical(), 1, &fence, VK_TRUE, UINT64_MAX),
		"Fence timed out!"
	);

	vkDestroyFence(device->Logical(), fence, nullptr);
}

CommandManager::CommandManager(const GraphicsDevice* device)
{
	// Attempt to create the command pool
	VkCommandPoolCreateInfo cpCreateInfo{};
	cpCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cpCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cpCreateInfo.queueFamilyIndex = device->QueueFamily();

	Try(
		vkCreateCommandPool(device->Logical(), &cpCreateInfo, nullptr, &m_commandPool),
		"Failed to create Command Pool!"
	);

	// Attempt to create command buffers for each frame in flight
	VkCommandBufferAllocateInfo cbAllocateInfo{};
	cbAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
	cbAllocateInfo.commandPool = m_commandPool;

	Try(
		vkAllocateCommandBuffers(device->Logical(), &cbAllocateInfo, m_commandBuffers.Data()),
		"Failed to create Command Buffers!"
	);
}

CommandManager::~CommandManager()
{
	vkDestroyCommandPool(Vulkan::Device()->Logical(), m_commandPool, nullptr);
}