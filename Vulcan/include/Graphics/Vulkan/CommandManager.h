#pragma once

#include "Graphics/Vulkan/Common.h"

namespace Vulcan
{
	class CommandManager
	{
		friend Vulkan;

	private:
		VkCommandPool m_commandPool;
		TArray<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> m_commandBuffers;

	public:
		VkCommandBuffer GetFrameCommandBuffer(uint32 frameIndex) const;

		void BeginOneTimeCommand(VkCommandBuffer& buffer, VkFence& fence) const;
		void EndOneTimeCommand(const VkCommandBuffer& buffer, const VkFence& fence) const;

	private:
		explicit CommandManager(const GraphicsDevice* device);
		~CommandManager();

	};
}
