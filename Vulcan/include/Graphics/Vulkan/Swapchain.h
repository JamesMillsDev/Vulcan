#pragma once

#include <string>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Vulkan.h"
#include "Utility/Collections/TArray.h"
#include "Utility/Collections/TList.h"

using std::string;

namespace Vulcan
{
	class GraphicsDevice;
	class Window;

	class SwapChain
	{
		friend class Vulkan;

	private:
		VkSwapchainKHR m_swapChain;
		TList<VkImage> m_swapChainImages;
		TList<VkImageView> m_swapChainImageViews;

		VkImage m_depthImage;
		VkImageView m_depthImageView;
		VmaAllocation m_depthImageAllocation;

		TArray<VkFence, MAX_FRAMES_IN_FLIGHT> m_fences;
		TArray<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_imageAcquiredSemaphores;
		TList<VkSemaphore> m_renderCompleteSemaphores;

		GraphicsDevice* m_device;
		VkInstance m_vkInstance;
		VmaAllocator m_allocator;
		VkSurfaceKHR m_surface;

		VkFormat m_format;

	private:
		SwapChain(const Window* window, GraphicsDevice* device, const VkInstance& instance, const VmaAllocator& allocator, const VkFormat& initialDepthFormat);
		~SwapChain();

	private:
		void Create(const Window* window, const VkFormat& initialDepthFormat);
		void Recreate(const Window* window, const VkFormat& depthFormat);

		VkResult AcquireNextImage(uint32* imgIndex, uint32 frameIndex) const;
		VkImage GetImage(uint32 index) const;
		VkImageView GetImageView(uint32 index) const;

		VkFence* GetFenceForFrame(uint32 frame);
		VkSemaphore* GetSemaphoreForFrame(uint32 frame);
		VkSemaphore* GetRenderCompleteSemaphoreForFrame(uint32 frame);

		VkSwapchainKHR* GetSwapChain();
		VkImage GetDepthImage() const;
		VkImageView GetDepthImageView() const;

		void CreateDepthImage(const VkExtent3D& extent, const VkFormat& format);
		void TransitionFrameImages(VkCommandBuffer cmdBuffer, uint32 index) const;

	};
}