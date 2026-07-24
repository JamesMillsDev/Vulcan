#include "Graphics/Vulkan/SwapChain.h"

#include <format>

#include "Window.h"
#include "Graphics/Vulkan/GraphicsDevice.h"

using namespace Vulcan;

SwapChain::SwapChain(const Window* window, GraphicsDevice* device, const VkInstance& instance, const VmaAllocator& allocator, const VkFormat& initialDepthFormat)
	: m_swapChain{ VK_NULL_HANDLE }, m_device{ device }, m_vkInstance{ instance }, m_allocator{ allocator },
	m_surface{ VK_NULL_HANDLE }, m_format{ VK_FORMAT_B8G8R8A8_SRGB }
{
	Create(window, initialDepthFormat);
}

SwapChain::~SwapChain()
{
	for (const VkSemaphore& semaphore : m_renderCompleteSemaphores)
	{
		vkDestroySemaphore(m_device->Logical(), semaphore, nullptr);
	}

	m_renderCompleteSemaphores.Clear();

	for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(m_device->Logical(), m_imageAcquiredSemaphores[i], nullptr);
		vkDestroyFence(m_device->Logical(), m_fences[i], nullptr);
	}

	vmaDestroyImage(m_allocator, m_depthImage, m_depthImageAllocation);
	vkDestroyImageView(m_device->Logical(), m_depthImageView, nullptr);

	for (VkImageView& scImageView : m_swapChainImageViews)
	{
		vkDestroyImageView(m_device->Logical(), scImageView, nullptr);
	}
	m_swapChainImageViews.Clear();

	vkDestroySwapchainKHR(m_device->Logical(), m_swapChain, nullptr);

	vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr);
}

void SwapChain::Create(const Window* window, const VkFormat& initialDepthFormat)
{
	Try(
		glfwCreateWindowSurface(m_vkInstance, window->GlfwHandle(), nullptr, &m_surface),
		"Failed to create window surface!"
	);

	VkSurfaceCapabilitiesKHR surfaceCaps{};
	Try(
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->Physical(), m_surface, &surfaceCaps),
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
		vkCreateSwapchainKHR(m_device->Logical(), &swapChainCI, nullptr, &m_swapChain),
		"Failed to create Swap Chain!"
	);

	// Attempt to acquire the swap chain images from the swap chain
	uint32 scImageCount = 0;
	Try(
		vkGetSwapchainImagesKHR(m_device->Logical(), m_swapChain, &scImageCount, nullptr),
		"Failed to count Swap Chain Images!"
	);

	m_swapChainImages.Resize(scImageCount);
	Try(
		vkGetSwapchainImagesKHR(m_device->Logical(), m_swapChain, &scImageCount, m_swapChainImages.Data()),
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
			vkCreateImageView(m_device->Logical(), &viewCreateInfo, nullptr, &m_swapChainImageViews[i]),
			std::format("Failed to create Swap Chain Image View for index: {}", i)
		);
	}

	CreateDepthImage(
		{ .width = static_cast<uint32_t>(window->Width()), .height = static_cast<uint32_t>(window->Height()), .depth = 1 },
		initialDepthFormat
	);

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Make sure the fence will be signaled for the first frame
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	// Create a fence and an image semaphore for each frame in flight
	for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		Try(
			vkCreateFence(m_device->Logical(), &fenceCreateInfo, nullptr, &m_fences[i]),
			std::format("Failed to create Fence for frame: {}!", i)
		);

		Try(
			vkCreateSemaphore(m_device->Logical(), &semaphoreCreateInfo, nullptr, &m_imageAcquiredSemaphores[i]),
			std::format("Failed to create Image Acquired Semaphore for frame: {}!", i)
		);
	}

	// Match the size of the render complete semaphores to the swap chain images
	m_renderCompleteSemaphores.Resize(m_swapChainImages.Count());
	for (VkSemaphore& semaphore : m_renderCompleteSemaphores)
	{
		Try(
			vkCreateSemaphore(m_device->Logical(), &semaphoreCreateInfo, nullptr, &semaphore),
			"Failed to create Render Complete Semaphore!"
		);
	}
}

void SwapChain::Recreate(const Window* window, const VkFormat& depthFormat)
{
	// Try to get the device capabilities
	VkSurfaceCapabilitiesKHR surfaceCaps;
	Try(
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->Physical(), m_surface, &surfaceCaps),
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
		vkCreateSwapchainKHR(m_device->Logical(), &swapChainCI, nullptr, &m_swapChain),
		"Failed to Recreate Swap Chain!"
	);

	// Destroy old swap chain images
	uint32 imageCount = static_cast<uint32>(m_swapChainImages.size());
	for (uint32 i = 0; i < imageCount; ++i)
	{
		vkDestroyImageView(m_device->Logical(), m_swapChainImageViews[i], nullptr);
	}

	// Get the new Swap Chain Images
	imageCount = 0;
	Try(
		vkGetSwapchainImagesKHR(m_device->Logical(), m_swapChain, &imageCount, nullptr),
		"Failed to get Swap Chain Image Count!"
	);
	m_swapChainImages.Resize(imageCount);
	Try(
		vkGetSwapchainImagesKHR(m_device->Logical(), m_swapChain, &imageCount, m_swapChainImages.Data()),
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
			vkCreateImageView(m_device->Logical(), &viewCreateInfo, nullptr, &m_swapChainImageViews[i]),
			std::format("Failed to create Swap Chain Image View for index: {}", i)
		);
	}

	// Destroy old semaphores
	for (VkSemaphore& semaphore : m_renderCompleteSemaphores)
	{
		vkDestroySemaphore(m_device->Logical(), semaphore, nullptr);
	}

	// Recreate semaphores
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	m_renderCompleteSemaphores.Resize(imageCount);
	for (VkSemaphore& semaphore : m_renderCompleteSemaphores)
	{
		Try(
			vkCreateSemaphore(m_device->Logical(), &semaphoreCreateInfo, nullptr, &semaphore),
			"Failed to recreate semaphore!"
		);
	}

	// Destroy the old swap chain and depth image / image view
	vkDestroySwapchainKHR(m_device->Logical(), swapChainCI.oldSwapchain, nullptr);

	vmaDestroyImage(m_allocator, m_depthImage, m_depthImageAllocation);
	vkDestroyImageView(m_device->Logical(), m_depthImageView, nullptr);

	CreateDepthImage(
		{ .width = static_cast<uint32_t>(window->Width()), .height = static_cast<uint32_t>(window->Height()), .depth = 1 },
		depthFormat
	);
}

VkResult SwapChain::AcquireNextImage(uint32* imgIndex, const uint32 frameIndex) const
{
	return vkAcquireNextImageKHR(
		m_device->Logical(), m_swapChain, UINT64_MAX, m_imageAcquiredSemaphores[frameIndex], VK_NULL_HANDLE, imgIndex
	);
}

VkImage SwapChain::GetImage(const uint32 index) const
{
	return m_swapChainImages[index];
}

VkImageView SwapChain::GetImageView(const uint32 index) const
{
	return m_swapChainImageViews[index];
}

VkFence* SwapChain::GetFenceForFrame(const uint32 frame)
{
	return &m_fences[frame];
}

VkSemaphore* SwapChain::GetSemaphoreForFrame(const uint32 frame)
{
	return &m_imageAcquiredSemaphores[frame];
}

VkSemaphore* SwapChain::GetRenderCompleteSemaphoreForFrame(const uint32 frame)
{
	return &m_renderCompleteSemaphores[frame];
}

VkSwapchainKHR* SwapChain::GetSwapChain()
{
	return &m_swapChain;
}

VkImage SwapChain::GetDepthImage() const
{
	return m_depthImage;
}

VkImageView SwapChain::GetDepthImageView() const
{
	return m_depthImageView;
}

void SwapChain::CreateDepthImage(const VkExtent3D& extent, const VkFormat& format)
{
	VkImageCreateInfo depthImageCI{};
	depthImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	depthImageCI.imageType = VK_IMAGE_TYPE_2D;
	depthImageCI.format = format;
	depthImageCI.extent = extent;
	depthImageCI.mipLevels = 1;
	depthImageCI.arrayLayers = 1;
	depthImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	depthImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depthImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo allocCI{};
	allocCI.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocCI.usage = VMA_MEMORY_USAGE_AUTO;

	// Attempt to create the depth image
	Try(
		vmaCreateImage(m_allocator, &depthImageCI, &allocCI, &m_depthImage, &m_depthImageAllocation, nullptr),
		"Failed to create Depth Image!"
	);

	VkImageViewCreateInfo depthViewCI
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.image = m_depthImage,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY },
		.subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 }
	};

	// Attempt to create the depth image view
	Try(
		vkCreateImageView(m_device->Logical(), &depthViewCI, nullptr, &m_depthImageView),
		"Failed to create Depth Image View!"
	);
}

void SwapChain::TransitionFrameImages(const VkCommandBuffer cmdBuffer, const uint32 index) const
{
	const TArray outputBarriers
	{
		VkImageMemoryBarrier2
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0,
			.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = 0,
			.dstQueueFamilyIndex = 0,
			.image = m_swapChainImages[index],
			.subresourceRange =
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		},
		VkImageMemoryBarrier2
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
			.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
			.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = 0,
			.dstQueueFamilyIndex = 0,
			.image = m_depthImage,
			.subresourceRange =
			{
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		}
	};
	VkDependencyInfo barrierDependencyInfo{};
	barrierDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	barrierDependencyInfo.imageMemoryBarrierCount = static_cast<uint32>(outputBarriers.size());
	barrierDependencyInfo.pImageMemoryBarriers = outputBarriers.Data();
	vkCmdPipelineBarrier2(cmdBuffer, &barrierDependencyInfo);
}
