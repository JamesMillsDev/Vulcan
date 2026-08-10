#include "EditorApplication.h"

#include <imgui_internal.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <glm/vec3.hpp>

#include "EditorGameInstance.h"
#include "FlyCamera.h"
#include "GameTime.h"

#include "Editor/EditorWindow.h"
#include "Editor/Menu.h"
#include "Editor/Windows/ConsoleWindow.h"
#include "Editor/Windows/InspectorWindow.h"
#include "Editor/Windows/ProjectWindow.h"
#include "Editor/Windows/WorldWindow.h"

#include "Gameplay/Actors/Components/Rendering/LightComponent.h"

#include "Graphics/Renderer.h"
#include "Graphics/Vulkan/GraphicsDevice.h"
#include "Graphics/Vulkan/Swapchain.h"
#include "Graphics/Vulkan/Vulkan.h"
#include "Graphics/Vulkan/VulkanInstance.h"

using glm::vec3;

using namespace Tempest;
using namespace Tempest::Editor;

EExitCode EditorApplication::Open()
{
	// Validate that the open function has not already been called
	assert(m_instance == nullptr && "Cannot create a second instance of application!");

	// If we are in a debug build, enable memory leak detection
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Create an instance of the game and application
	EditorApplication* app = new EditorApplication;
	m_instance = app;
	app->m_game = new EditorGameInstance;

	// Run the application, gathering the exit code
	const EExitCode exitCode = app->Run();

	// Clean up the application instance and return the exit code
	delete app;
	m_instance = nullptr;
	return exitCode;
}

EditorApplication::EditorApplication()
	: m_camera{ nullptr }, m_mainMenu{ nullptr }, m_imguiPool{ VK_NULL_HANDLE }, m_dockSpaceId{ 0 }
{}

void EditorApplication::Init(Vulkan* vulkan)
{
	InitialiseImGui(vulkan);

	m_camera = new FlyCamera{ 45.f, .1f, 100.f };
	m_camera->location = vec3{ 0.f, 2.f, 10.f };
	m_camera->yaw = 180.f;

	MenuBuilder builder;
	builder
		.SubMenu("File")
		.SubMenu("New")
		.Item("Project", []
			{})
		.Item("Scene", []
			{})
		.End()
		.Item("Open", []
			{})
		.Separator()
		.Item("Save", []
			{})
		.Item("Save As...", []
			{})
		.Separator()
		.Item("Close", []
			{
				Quit();
			})
		.End()
		.SubMenu("Edit")
		.Item("Undo", []
			{})
		.Item("Redo", []
			{})
		.End()
		.SubMenu("Help")
		.Item("About...", []
			{})
		.End();

	m_mainMenu = builder.Build();

	m_windows.Add(new InspectorWindow);
	m_windows.Add(new WorldWindow);
	m_windows.Add(new ProjectWindow);
	m_windows.Add(new ConsoleWindow);
}

void EditorApplication::Tick()
{
	m_camera->Tick();
}

void EditorApplication::PreRender()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
	m_dockSpaceId = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	m_mainMenu->Render();

	for (EditorWindow* window : m_windows)
	{
		window->Render();
	}
}

void EditorApplication::Render()
{
	if (const ImGuiDockNode* centralNode = ImGui::DockBuilderGetCentralNode(m_dockSpaceId))
	{
		ImVec2 centralNodeLoc = centralNode->Pos;
		ImVec2 centralNodeSize = centralNode->Size;
		const VkRect2D scissor =
		{
			.offset = { static_cast<int32>(centralNodeLoc.x), static_cast<int32>(centralNodeLoc.y) },
			.extent = { static_cast<uint32>(centralNodeSize.x), static_cast<uint32>(centralNodeSize.y) }
		};
		Vulkan::Instance()->GetSwapChain()->UpdateScissor(scissor);
	}
}

void EditorApplication::PostRender()
{
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Renderer::CurrentCmdBuffer());
}

void EditorApplication::Shutdown()
{
	delete m_mainMenu;
	delete m_camera;

	ImGui_ImplVulkan_Shutdown();
	vkDestroyDescriptorPool(Vulkan::Device()->Logical(), m_imguiPool, nullptr);
	ImGui_ImplGlfw_Shutdown();

	ImGui::DestroyContext();
}

void EditorApplication::InitialiseImGui(Vulkan* vulkan)
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
		vkCreateDescriptorPool(vulkan->GetDevice()->Logical(), &poolInfo, nullptr, &m_imguiPool),
		"Failed to create ImGui Descriptor Pool!"
	);

	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplGlfw_InitForVulkan(m_window->GlfwHandle(), true);

	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = vulkan->GetInstance()->Get();
	initInfo.PhysicalDevice = vulkan->GetDevice()->Physical();
	initInfo.Device = vulkan->GetDevice()->Logical();
	initInfo.Queue = vulkan->GetDevice()->Queue();
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
		.pColorAttachmentFormats = &vulkan->GetSwapChain()->GetFormat(),
		.depthAttachmentFormat = vulkan->GetDepthFormat(),
		.stencilAttachmentFormat = VK_FORMAT_UNDEFINED
	};
	pipelineInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	initInfo.PipelineInfoMain = pipelineInfo;

	ImGui_ImplVulkan_Init(&initInfo);
}
