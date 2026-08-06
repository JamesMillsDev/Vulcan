#include "Graphics/Vulkan/Vulkan.h"

#include <format>

#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_vulkan.h>

#include "Application.h"
#include "Window.h"

#include "Gameplay/Actors/Components/Rendering/LightComponent.h"

#include "Graphics/Rendering/Texture.h"
#include "Graphics/Vulkan/CommandManager.h"
#include "Graphics/Vulkan/GraphicsDevice.h"
#include "Graphics/Vulkan/Swapchain.h"
#include "Graphics/Vulkan/VulkanInstance.h"

#include "Utility/Config.h"
#include "Utility/Console.h"
#include "Utility/Collections/ResourceStack.h"

using std::exception;

using namespace Fulk;

void CheckSwapChain(const VkResult result, const string& errorMsg) // NOLINT(misc-use-anonymous-namespace, clang-diagnostic-microsoft-redeclare-static)
{
	if (result != VK_SUCCESS)
	{
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			Vulkan::Instance()->recreateSwapChain = true;
			return;
		}

		throw Vulkan::VulkanError(errorMsg, result);
	}
}

Vulkan* Vulkan::m_instance = nullptr;

Vulkan* Vulkan::Instance()
{
	return m_instance;
}

const GraphicsDevice* Vulkan::Device()
{
	return m_instance->GetDevice();
}

const GraphicsDevice* Vulkan::GetDevice() const
{
	return m_device;
}

const VmaAllocator& Vulkan::Allocator()
{
	return m_instance->GetAllocator();
}

const VmaAllocator& Vulkan::GetAllocator() const
{
	return m_vmaAllocator;
}

const CommandManager* Vulkan::CmdManager()
{
	return m_instance->GetCmdManager();
}

const CommandManager* Vulkan::GetCmdManager() const
{
	return m_commandManager;
}

bool Vulkan::IsLoaded()
{
	return m_instance != nullptr && m_instance->m_loaded;
}

runtime_error Vulkan::VulkanError(const string& message, const VkResult result)
{
	return runtime_error(std::format("{}. Error Code: {}", message, static_cast<int32>(result)));
}

void Vulkan::Create(Config* config, GLFWwindow* window)
{
	m_instance = new Vulkan{ config, window };
}

void Vulkan::Destroy()
{
	delete m_instance;
	m_instance = nullptr;
}

Vulkan::Vulkan(Config* config, GLFWwindow* window)
	: recreateSwapChain{ false }, m_resourceStack{ new ResourceStack }, m_loaded{ false },
	m_frameIndex{ 0 }, m_imageIndex{ 0 }
{
	m_clearColor = config->Get<Color>("Window.ClrColor");
	m_clearColor.ToGamma();

	Init(config, window);
}

Vulkan::~Vulkan()
{
	delete m_resourceStack;
}

VkFormat Vulkan::GetDepthFormat() const
{
	// Attempt to get the correct format for the swap chain images
	const vector depthFormatList = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	VkFormat depthFormat = VK_FORMAT_UNDEFINED;
	for (const VkFormat& format : depthFormatList)
	{
		// Get the device format properties
		VkFormatProperties2 formatProperties{};
		formatProperties.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
		vkGetPhysicalDeviceFormatProperties2(m_device->Physical(), format, &formatProperties);

		// If this format properties contains the tiling features we want, store and break
		if (formatProperties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			depthFormat = format;
			break;
		}
	}

	return depthFormat;
}

void Vulkan::Init(Config* config, GLFWwindow* window)
{
	try
	{
		// VK Instance
		InitAndPushResource(
			[this, config]
			{
				m_vkInstance = new VulkanInstance{ config };
			},
			[this]
			{
				delete m_vkInstance;
			}
		);

		// Logical / Physical Device
		InitAndPushResource(
			[this]
			{
				m_device = new GraphicsDevice{ m_vkInstance->Get() };
			},
			[this]
			{
				delete m_device;
			}
		);

		// VMA allocator
		InitAndPushResource(
			[this]
			{
				VmaVulkanFunctions vkFunctions{};
				vkFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
				vkFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
				vkFunctions.vkCreateImage = vkCreateImage;

				VmaAllocatorCreateInfo allocatorCI{};
				allocatorCI.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
				allocatorCI.physicalDevice = m_device->Physical();
				allocatorCI.device = m_device->Logical();
				allocatorCI.pVulkanFunctions = &vkFunctions;
				allocatorCI.instance = m_vkInstance->Get();

				Try(
					vmaCreateAllocator(&allocatorCI, &m_vmaAllocator),
					"Failed to create VMA Allocator"
				);
			},
			[this]
			{
				vmaDestroyAllocator(m_vmaAllocator);
			}
		);

		// Swap chain / swap chain images
		InitAndPushResource(
			[this]
			{
				const Window* win = Application::GetWindow();
				m_swapChain = new SwapChain{ win, m_device, m_vkInstance->Get(), m_vmaAllocator, GetDepthFormat() };
			},
			[this]
			{
				delete m_swapChain;
			}
		);

		// Fences and semaphores
		InitAndPushResource(
			[this]
			{
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
				m_renderCompleteSemaphores.Resize(m_swapChain->m_swapChainImages.Count());
				for (VkSemaphore& semaphore : m_renderCompleteSemaphores)
				{
					Try(
						vkCreateSemaphore(m_device->Logical(), &semaphoreCreateInfo, nullptr, &semaphore),
						"Failed to create Render Complete Semaphore!"
					);
				}
			},
			[this]
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
			}
		);

		// Command buffers
		InitAndPushResource(
			[this]
			{
				// Attempt to create the command pool
				m_commandManager = new CommandManager{ m_device };
			},
			[this]
			{
				delete m_commandManager;
			}
		);

	#if _DEBUG
		// ImGui
		InitAndPushResource(
			[this, window]
			{
				TArray poolSizes
				{
					VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
					VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
					VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
					VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
					VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
					VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
					VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
					VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
					VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
					VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
					VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
				};

				VkDescriptorPoolCreateInfo poolInfo
				{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
					.pNext = nullptr,
					.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
					.maxSets = 1000,
					.poolSizeCount = poolSizes.Count(),
					.pPoolSizes = poolSizes.Data()
				};

				Try(
					vkCreateDescriptorPool(m_device->Logical(), &poolInfo, nullptr, &m_imguiPool),
					"Failed to create ImGui Descriptor Pool!"
				);

				ImGui::CreateContext();
				ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

				ImGui_ImplGlfw_InitForVulkan(window, true);

				ImGui_ImplVulkan_InitInfo initInfo{};
				initInfo.Instance = m_vkInstance->Get();
				initInfo.PhysicalDevice = m_device->Physical();
				initInfo.Device = m_device->Logical();
				initInfo.Queue = m_device->Queue();
				initInfo.DescriptorPool = m_imguiPool;
				initInfo.MinImageCount = 3;
				initInfo.ImageCount = 3;
				initInfo.UseDynamicRendering = true;

				ImGui_ImplVulkan_PipelineInfo pipelineInfo{};
				pipelineInfo.PipelineRenderingCreateInfo =
				{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
					.pNext = nullptr,
					.viewMask = VK_FORMAT_UNDEFINED,
					.colorAttachmentCount = 1,
					.pColorAttachmentFormats = &m_swapChain->m_format,
					.depthAttachmentFormat = GetDepthFormat(),
					.stencilAttachmentFormat = VK_FORMAT_UNDEFINED
				};
				pipelineInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

				initInfo.PipelineInfoMain = pipelineInfo;

				ImGui_ImplVulkan_Init(&initInfo);
			},
			[this]
			{
				ImGui_ImplVulkan_Shutdown();
				vkDestroyDescriptorPool(m_device->Logical(), m_imguiPool, nullptr);
				ImGui_ImplGlfw_Shutdown();

				ImGui::DestroyContext();
			}
		);
	#endif

		// Set the resize callback
		glfwSetWindowSizeCallback(window, [](GLFWwindow* _, const int w, const int h)
			{
				Application::GetWindow()->SetWidth(w);
				Application::GetWindow()->SetHeight(h);

				Instance()->recreateSwapChain = true;
			});

		// All functions ran safely, so we loaded correctly. 
		m_loaded = true;
	}
	catch (exception& e)
	{
		Console::Exception(e);
	}
}

void Vulkan::RecreateSwapChain()
{
	vkDeviceWaitIdle(m_device->Logical());

	const Window* window = Application::GetWindow();
	m_swapChain->Recreate(window, GetDepthFormat());

	// Destroy old semaphores
	for (VkSemaphore& semaphore : m_renderCompleteSemaphores)
	{
		vkDestroySemaphore(m_device->Logical(), semaphore, nullptr);
	}

	// Recreate semaphores
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	m_renderCompleteSemaphores.Resize(m_swapChain->GetImageCount());
	for (VkSemaphore& semaphore : m_renderCompleteSemaphores)
	{
		Try(
			vkCreateSemaphore(m_device->Logical(), &semaphoreCreateInfo, nullptr, &semaphore),
			"Failed to recreate semaphore!"
		);
	}
}

VkCommandBuffer Vulkan::BeginFrame()
{
	// Recreate the Swap Chain if needed
	if (recreateSwapChain)
	{
		recreateSwapChain = false;
		RecreateSwapChain();
	}

	// Wait on and reset fences
	Try(
		vkWaitForFences(m_device->Logical(), 1, &m_fences[m_frameIndex], true, VULKAN_TIMEOUT),
		std::format("Failed to wait for fence on frame: {}!", m_frameIndex)
	);
	Try(
		vkResetFences(m_device->Logical(), 1, &m_fences[m_frameIndex]),
		std::format("Failed to reset fence on frame: {}!", m_frameIndex)
	);

	// Try to get the swap chain image index for this frame
	CheckSwapChain(
		m_swapChain->AcquireNextImage(&m_imageIndex, m_imageAcquiredSemaphores[m_frameIndex]),
		std::format("Failed to acquire Swap Chain Image index for frame: {}!", m_frameIndex)
	);

	// Try to reset and retrieve the command buffer
	const VkCommandBuffer cmdBuf = m_commandManager->GetFrameCommandBuffer(m_frameIndex);

	// Transition swap chain and depth images / begin rendering
	m_swapChain->TransitionFrameImages(cmdBuf, m_imageIndex);
	m_swapChain->BeginFrameRender(cmdBuf, m_imageIndex, m_clearColor);

#if _DEBUG
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	ImGui::BeginMainMenuBar();

	ImGui::MenuItem("File");

	ImGui::EndMainMenuBar();
#endif

	return cmdBuf;
}

void Vulkan::EndFrame(const VkCommandBuffer cmdBuffer)
{
#if _DEBUG
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuffer);
#endif // _DEBUG

	// End the rendering and transition the swap chain image
	Try(
		m_swapChain->EndFrameRender(cmdBuffer, m_imageIndex),
		std::format("Failed to end Command Buffer for frame: {}!", m_frameIndex)
	);

	// Try to submit the queue
	CheckSwapChain(
		m_swapChain->Present(
			m_imageAcquiredSemaphores[m_frameIndex], m_renderCompleteSemaphores[m_frameIndex],
			m_fences[m_frameIndex], cmdBuffer, m_imageIndex, m_frameIndex
		),
		std::format("Failed to present queue for frame: {}!", m_frameIndex)
	);

	// Try to present the queue
	m_frameIndex = (m_frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Vulkan::InitAndPushResource(const InitFunction& init, const CleanupFunction& cleanup) const
{
	init();
	m_resourceStack->Push(cleanup);
}