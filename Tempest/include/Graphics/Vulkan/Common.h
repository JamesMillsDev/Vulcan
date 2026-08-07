#pragma once

#include <vulkan/vulkan.h>

#include "Maths/Alias.h"

#include "Utility/Collections/TArray.h"
#include "Utility/Collections/TList.h"
#include "Utility/Collections/TMap.h"

namespace Tempest
{
	constexpr int32 MAX_FRAMES_IN_FLIGHT = 2;
	constexpr uint32 MAX_VISIBLE_OBJECTS = 512;
	constexpr uint64 VULKAN_TIMEOUT = 100000000;

	class CommandManager;
	class GraphicsDevice;
	class GraphicsPipeline;
	class MemoryBuffer;
	class SwapChain;
	class Vulkan;
	class VulkanInstance;
}
