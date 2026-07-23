#include "Graphics/Vulkan/SwapChain.h"

#include <format>

#include "Window.h"

#include "Graphics/Vulkan/Vulkan.h"

static void Try(const VkResult result, const string& errorMsg)  // NOLINT(misc-use-anonymous-namespace)
{
	if (result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError(errorMsg, result);
	}
}

SwapChain::SwapChain(const Window* window, VkPhysicalDevice gpu, VkDevice device, VkSurfaceKHR surface)
	: m_swapChain{ VK_NULL_HANDLE }, m_gpu{ gpu }, m_device{ device },
	m_surface{ surface }, m_format{ VK_FORMAT_B8G8R8A8_SRGB }
{
	Create(window);
}

SwapChain::~SwapChain()
{
	for (VkImageView& scImageView : m_swapChainImageViews)
	{
		vkDestroyImageView(m_device, scImageView, nullptr);
	}
	m_swapChainImageViews.Clear();

	vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
}

void SwapChain::Create(const Window* window)
{
	VkSurfaceCapabilitiesKHR surfaceCaps{};
	Try(
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpu, m_surface, &surfaceCaps),
		"Failed to retrieve surface capabilities"
	);

	// Verify the window size
	VkExtent2D swapChainExtent = surfaceCaps.currentExtent;
	if (surfaceCaps.currentExtent.width == 0xffffffff)
	{
		// Use the glfw window size as the swap chain size
		swapChainExtent =
		{
			.width = static_cast<uint32>(window->Width()),
			.height = static_cast<uint32>(window->Height())
		};
	}

	// Generate the Swap Chain Create Information
	VkSwapchainCreateInfoKHR swapChainCI{};
	swapChainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCI.surface = m_surface;
	swapChainCI.minImageCount = surfaceCaps.minImageCount;
	swapChainCI.imageFormat = m_format;
	swapChainCI.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapChainCI.imageExtent = { .width = swapChainExtent.width, .height = swapChainExtent.height };
	swapChainCI.imageArrayLayers = 1;
	swapChainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCI.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapChainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCI.presentMode = VK_PRESENT_MODE_FIFO_KHR;

	// Attempt to create the swap chain
	Try(
		vkCreateSwapchainKHR(m_device, &swapChainCI, nullptr, &m_swapChain),
		"Failed to create Swap Chain!"
	);

	// Attempt to acquire the swap chain images from the swap chain
	uint32 scImageCount = 0;
	Try(
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &scImageCount, nullptr),
		"Failed to count Swap Chain Images!"
	);

	m_swapChainImages.Resize(scImageCount);
	Try(
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &scImageCount, m_swapChainImages.Data()),
		"Failed to retrieve Swap Chain Images!"
	);

	// Resize the image view vector to match the image one
	m_swapChainImageViews.Resize(scImageCount);

	// Create the new Swap Chain image views
	for (uint32 i = 0; i < scImageCount; ++i)
	{
		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = m_swapChainImages[i];
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = m_format;
		viewCreateInfo.subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 };  // NOLINT(clang-diagnostic-missing-designated-field-initializers)

		Try(
			vkCreateImageView(m_device, &viewCreateInfo, nullptr, &m_swapChainImageViews[i]),
			std::format("Failed to create Swap Chain Image View for index: {}", i)
		);
	}
}

void SwapChain::Recreate(const Window* window, TList<VkSemaphore>& renderCompleteSemaphores)
{
	// Try to get the device capabilities
	VkSurfaceCapabilitiesKHR surfaceCaps;
	Try(
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpu, m_surface, &surfaceCaps),
		"Failed to get the device capabilities!"
	);

	// Verify the window size
	VkExtent2D swapChainExtent = surfaceCaps.currentExtent;
	if (surfaceCaps.currentExtent.width == 0xffffffff)
	{
		// Use the glfw window size as the swap chain size
		swapChainExtent =
		{
			.width = static_cast<uint32>(window->Width()),
			.height = static_cast<uint32>(window->Height())
		};
	}

	// Generate the Swap Chain Create Information
	constexpr VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
	VkSwapchainCreateInfoKHR swapChainCI{};
	swapChainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCI.surface = m_surface;
	swapChainCI.minImageCount = surfaceCaps.minImageCount;
	swapChainCI.imageFormat = imageFormat;
	swapChainCI.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swapChainCI.imageExtent = { .width = swapChainExtent.width, .height = swapChainExtent.height };
	swapChainCI.imageArrayLayers = 1;
	swapChainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCI.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapChainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCI.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapChainCI.oldSwapchain = m_swapChain;

	// Create the new swap chain
	Try(
		vkCreateSwapchainKHR(m_device, &swapChainCI, nullptr, &m_swapChain),
		"Failed to Recreate Swap Chain!"
	);

	// Destroy old swap chain images
	uint32 imageCount = static_cast<uint32>(m_swapChainImages.size());
	for (uint32 i = 0; i < imageCount; ++i)
	{
		vkDestroyImageView(m_device, m_swapChainImageViews[i], nullptr);
	}

	// Get the new Swap Chain Images
	imageCount = 0;
	Try(
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr),
		"Failed to get Swap Chain Image Count!"
	);
	m_swapChainImages.Resize(imageCount);
	Try(
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.Data()),
		"Failed to get Swap Chain Images!"
	);
	m_swapChainImageViews.Resize(imageCount);

	// Create the new Swap Chain image views
	for (uint32 i = 0; i < imageCount; ++i)
	{
		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = m_swapChainImages[i];
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = imageFormat;
		viewCreateInfo.subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 };  // NOLINT(clang-diagnostic-missing-designated-field-initializers)

		Try(
			vkCreateImageView(m_device, &viewCreateInfo, nullptr, &m_swapChainImageViews[i]),
			std::format("Failed to create Swap Chain Image View for index: {}", i)
		);
	}

	// Destroy old semaphores
	for (VkSemaphore& semaphore : renderCompleteSemaphores)
	{
		vkDestroySemaphore(m_device, semaphore, nullptr);
	}

	// Recreate semaphores
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	renderCompleteSemaphores.Resize(imageCount);
	for (VkSemaphore& semaphore : renderCompleteSemaphores)
	{
		Try(
			vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &semaphore),
			"Failed to recreate semaphore!"
		);
	}

	// Destroy the old swap chain and depth image / image view
	vkDestroySwapchainKHR(m_device, swapChainCI.oldSwapchain, nullptr);
}

VkResult SwapChain::AcquireNextImage(uint32* imgIndex, const VkSemaphore imgAcquiredSemaphore) const
{
	return vkAcquireNextImageKHR(
		m_device, m_swapChain, UINT64_MAX, imgAcquiredSemaphore, VK_NULL_HANDLE, imgIndex
	);
}

VkImage SwapChain::GetImage(uint32 index) const
{
	return m_swapChainImages[index];
}

VkImageView SwapChain::GetImageView(const uint32 index) const
{
	return m_swapChainImageViews[index];
}

VkSwapchainKHR* SwapChain::GetSwapChain()
{
	return &m_swapChain;
}
