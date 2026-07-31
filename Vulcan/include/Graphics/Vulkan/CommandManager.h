#pragma once

#include <functional>

#include "Graphics/Vulkan/Common.h"

using ImmediateSubmitFnc = std::function<void(VkCommandBuffer)>;

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
		void ImmediateSubmit(const ImmediateSubmitFnc& fnc) const;

	private:
		explicit CommandManager(const GraphicsDevice* device);
		~CommandManager();

	};
}
