#include "Graphics/Renderer.h"

#include <vulkan/vulkan.h>

#include "Application.h"
#include "Gameplay/Actors/World.h"
#include "Graphics/Rendering/Camera.h"
#include "Graphics/Rendering/HdrTexture.h"
#include "Graphics/Rendering/Lighting.h"
#include "Graphics/Rendering/Material.h"
#include "Graphics/Rendering/Mesh.h"
#include "Graphics/Rendering/Texture.h"
#include "Graphics/Vulkan/GraphicsDevice.h"
#include "Graphics/Vulkan/MemoryBuffer.h"
#include "Graphics/Vulkan/Vulkan.h"

using namespace Tempest;
using glm::mat3;

Renderer* Renderer::m_instance = nullptr;
Camera* Renderer::m_currentCamera = nullptr;

Renderer* Renderer::Instance()
{
	return m_instance;
}

bool Renderer::IsValid()
{
	return m_instance != nullptr && Vulkan::IsLoaded();
}

VkCommandBuffer Renderer::CurrentCmdBuffer()
{
	return m_instance->m_frameCmdBuf;
}

Camera* Renderer::GetCurrentCamera()
{
	return m_currentCamera;
}

void Renderer::SetCurrent(Camera* newCurrent)
{
	if (m_currentCamera != nullptr)
	{
		m_currentCamera->m_isCurrent = false;
	}

	m_currentCamera = newCurrent;
	m_currentCamera->m_isCurrent = true;
}

void Renderer::Create(Config* config, GLFWwindow* window)
{
	m_instance = new Renderer{ config, window };
}

void Renderer::Destroy()
{
	delete m_instance;
	m_instance = nullptr;
}

void Renderer::InitVulkan(Config* config, GLFWwindow* window)
{
	Vulkan::Create(config, window);
}

void Renderer::DestroyVulkan()
{
	Vulkan::Destroy();
}

void Renderer::WaitIdle()
{
	vkDeviceWaitIdle(Vulkan::Device()->Logical());
}

Renderer::Renderer(Config* config, GLFWwindow* window)
	: globalsUniform{  }, m_frameCmdBuf{ VK_NULL_HANDLE }
{
	m_instance = this;
	InitVulkan(config, window);

	m_vulkan = Vulkan::Instance();

	m_globalUniformBuffer = new MemoryBuffer{ sizeof(GlobalsUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT };
}

Renderer::~Renderer()
{
	delete m_globalUniformBuffer;

	DestroyVulkan();
}

void Renderer::Render(const Mesh* mesh, const TList<Material*>& materials, const mat4& transform, const Lighting* lighting) const
{
	const MaterialBindInfo bindInfo =
	{
		.transform = transform,
		.globalsBuffer = m_globalUniformBuffer,
		.skyboxDescriptor = lighting->m_skyboxTexture->GetDescriptors(),
		.sceneLightBuffer = lighting->m_sceneLightingBuffer,
		.lightBuffers = lighting->m_lightBuffers
	};

	mesh->Render(m_frameCmdBuf, materials, bindInfo);
}

void Renderer::BeginFrame()
{
	if (!IsValid())
	{
		return;
	}

	m_frameCmdBuf = m_vulkan->BeginFrame();

	m_currentCamera->GetPvm(globalsUniform);
	globalsUniform.rotationView = mat4(mat3(globalsUniform.view));

	globalsUniform.exposure = 4.5f;
	globalsUniform.gamma = 2.2f;
	globalsUniform.prefilteredCubeMipLevels = 1.f;

	m_globalUniformBuffer->Fill(&globalsUniform);
}

void Renderer::EndFrame()
{
	m_vulkan->EndFrame(m_frameCmdBuf);
	m_frameCmdBuf = VK_NULL_HANDLE;
}