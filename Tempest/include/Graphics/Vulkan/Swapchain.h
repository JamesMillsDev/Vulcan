#pragma once

#include <string>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Vulkan.h"
#include "Utility/Collections/TList.h"

using std::string;

namespace Tempest
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

		GraphicsDevice* m_device;
		VkInstance m_vkInstance;
		VmaAllocator m_allocator;
		VkSurfaceKHR m_surface;

		VkFormat m_format;

	private:
		SwapChain(const Window* window, GraphicsDevice* device, const VkInstance& instance, const VmaAllocator& allocator, const VkFormat& initialDepthFormat);
		~SwapChain();

	public:
		const VkFormat& GetFormat() const;

	private:
		void Create(const Window* window, const VkFormat& initialDepthFormat);
		void Recreate(const Window* window, const VkFormat& depthFormat);

		VkResult AcquireNextImage(uint32* imgIndex, VkSemaphore imgAcquiredSemaphore) const;
		VkImage GetImage(uint32 index) const;
		VkImageView GetImageView(uint32 index) const;

		VkImage GetDepthImage() const;
		VkImageView GetDepthImageView() const;
		uint32 GetImageCount() const;

		void CreateDepthImage(const VkExtent3D& extent, const VkFormat& format);

		void TransitionFrameImages(VkCommandBuffer cmdBuffer, uint32 index) const;
		void BeginFrameRender(VkCommandBuffer cmdBuffer, uint32 imgIndex, const Color& clrColor);
		VkResult EndFrameRender(VkCommandBuffer cmdBuffer, uint32 imgIndex);

		VkResult Present(const VkSemaphore& waitSemaphore, const VkSemaphore& signalSemaphore,
			const VkFence& fence, const VkCommandBuffer& cmdBuffer, uint32 imgIndex,
			uint32 frameIndex) const;

	};
}