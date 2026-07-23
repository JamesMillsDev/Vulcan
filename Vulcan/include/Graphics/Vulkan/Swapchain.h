#pragma once

#include <string>
#include <vulkan/vulkan.h>

#include "Utility/Collections/TList.h"

using std::string;

class Window;

class SwapChain
{
	friend class Vulkan;

private:
	VkSwapchainKHR m_swapChain;
	TList<VkImage> m_swapChainImages;
	TList<VkImageView> m_swapChainImageViews;

	VkPhysicalDevice m_gpu;
	VkDevice m_device;
	VkSurfaceKHR m_surface;

	VkFormat m_format;

private:
	SwapChain(const Window* window, VkPhysicalDevice gpu, VkDevice device, VkSurfaceKHR surface);
	~SwapChain();

private:
	void Create(const Window* window);
	void Recreate(const Window* window, TList<VkSemaphore>& renderCompleteSemaphores);

	VkResult AcquireNextImage(uint32* imgIndex, VkSemaphore imgAcquiredSemaphore) const;
	VkImage GetImage(uint32 index) const;
	VkImageView GetImageView(uint32 index) const;

	VkSwapchainKHR* GetSwapChain();

};