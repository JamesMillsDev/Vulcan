#include "Graphics/Renderer.h"

#include <vulkan/vulkan.h>

#include "Graphics/Rendering/Camera.h"
#include "Graphics/Rendering/Material.h"
#include "Graphics/Rendering/Mesh.h"
#include "Graphics/Vulkan/MemoryBuffer.h"
#include "Graphics/Vulkan/Vulkan.h"

using namespace Vulcan;

Renderer* Renderer::m_instance = nullptr;
Camera* Renderer::m_currentCamera = nullptr;

// Courtesy of https://github.com/SaschaWillems/Vulkan/blob/master/examples/dynamicuniformbuffer/dynamicuniformbuffer.cpp#L31C1-L42C2
void* alignedAlloc(size_t size, size_t alignment)
{
	void* data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
	data = _aligned_malloc(size, alignment);
#else
	int res = posix_memalign(&data, alignment, size);
	if (res != 0)
		data = nullptr;
#endif
	return data;
}

void alignedFree(void* data)
{
#if	defined(_MSC_VER) || defined(__MINGW32__)
	_aligned_free(data);
#else
	free(data);
#endif
}

Renderer* Renderer::Instance()
{
	return m_instance;
}

bool Renderer::IsValid()
{
	return m_instance != nullptr && Vulkan::IsLoaded();
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
	vkDeviceWaitIdle(Vulkan::Device());
}

Renderer::Renderer(Config* config, GLFWwindow* window)
	: m_frameCmdBuf{ VK_NULL_HANDLE }, m_globalsUniform{  }
{
	m_instance = this;
	InitVulkan(config, window);

	m_vulkan = Vulkan::Instance();

	const uint64 bufferSize = MAX_VISIBLE_OBJECTS * m_vulkan->GetDynamicAlignment();
	m_transforms.values = static_cast<mat4*>(alignedAlloc(bufferSize, m_vulkan->GetDynamicAlignment()));
	m_transformBuffer = new MemoryBuffer{ bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, m_vulkan, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT };

	m_transformBuffer->Fill(&m_transforms);
}

Renderer::~Renderer()
{
	alignedFree(m_transforms.values);

	DestroyVulkan();
}

void Renderer::Render(const Mesh* mesh, Material* material, const mat4& transform, const uint32 objectIndex) const
{
	m_transforms.values[objectIndex] = transform;
	m_transformBuffer->Fill(&m_transforms);

	material->Bind(m_frameCmdBuf, objectIndex);
	mesh->Render(m_frameCmdBuf);
}

void Renderer::BeginFrame()
{
	if (!IsValid())
	{
		return;
	}

	m_frameCmdBuf = m_vulkan->BeginFrame();

	m_currentCamera->GetPvm(m_globalsUniform);

	m_globalsUniform.exposure = 4.5f;
	m_globalsUniform.gamma = 2.2f;
	m_globalsUniform.prefilteredCubeMipLevels = 1.f;
	m_globalsUniform.scaleIBLAmbient = 1.f;

	const MemoryBuffer* globalsBuff = m_vulkan->GetUniformBuffer(EUniformBufferIds::Globals);
	globalsBuff->Fill(&m_globalsUniform);

	
}

void Renderer::EndFrame()
{
	m_vulkan->EndFrame(m_frameCmdBuf);
	m_frameCmdBuf = VK_NULL_HANDLE;
}