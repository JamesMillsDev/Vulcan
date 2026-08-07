#pragma once

#include <functional>
#include <vk_mem_alloc.h>

#include "Graphics/Vulkan/Common.h"

#include "Maths/Color.h"

struct GLFWwindow;

using InitFunction = std::function<void()>;
using CleanupFunction = std::function<void()>;

namespace Tempest
{
	class Config;
	class ResourceStack;

	class Vulkan  // NOLINT(cppcoreguidelines-special-member-functions)
	{
		friend class Renderer;

	private:
		static Vulkan* m_instance;

	public:
		[[nodiscard]] static Vulkan* Instance();

		[[nodiscard]] static bool IsLoaded();
		[[nodiscard]] static runtime_error VulkanError(const string& message, VkResult result);

		[[nodiscard]] static const GraphicsDevice* Device();
		[[nodiscard]] static const CommandManager* CmdManager();
		[[nodiscard]] static const VmaAllocator& Allocator();

	private:
		static void Create(Config* config, GLFWwindow* window);
		static void Destroy();

	public:
		bool recreateSwapChain;

	private:
		Color m_clearColor;
		VmaAllocator m_vmaAllocator;

		ResourceStack* m_resourceStack;
		bool m_loaded;

		VulkanInstance* m_vkInstance;
		GraphicsDevice* m_device;
		SwapChain* m_swapChain;
		CommandManager* m_commandManager;

		TArray<VkFence, MAX_FRAMES_IN_FLIGHT> m_fences;
		TArray<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_imageAcquiredSemaphores;
		TList<VkSemaphore> m_renderCompleteSemaphores;

		uint32 m_frameIndex;
		uint32 m_imageIndex;

	private:
		explicit Vulkan(Config* config, GLFWwindow* window);
		~Vulkan();

	public:
		VkFormat GetDepthFormat() const;

		[[nodiscard]] const GraphicsDevice* GetDevice() const;
		[[nodiscard]] const CommandManager* GetCmdManager() const;
		[[nodiscard]] const VmaAllocator& GetAllocator() const;
		[[nodiscard]] const VulkanInstance* GetInstance() const;
		[[nodiscard]] const SwapChain* GetSwapChain() const;

	private:
		void Init(Config* config, GLFWwindow* window);
		void RecreateSwapChain();

		VkCommandBuffer BeginFrame();
		void EndFrame(VkCommandBuffer cmdBuffer);
		
		void InitAndPushResource(const InitFunction& init, const CleanupFunction& cleanup) const;

	};

	inline void Try(const VkResult result, const string& errorMsg)
	{
		if (result != VK_SUCCESS)
		{
			throw Vulkan::VulkanError(errorMsg, result);
		}
	}
}
