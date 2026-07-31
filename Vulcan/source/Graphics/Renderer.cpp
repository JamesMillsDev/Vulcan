#include "Graphics/Renderer.h"

#include <vulkan/vulkan.h>

#include "Application.h"
#include "Gameplay/GameInstance.h"
#include "Gameplay/Actors/World.h"
#include "Graphics/Rendering/Camera.h"
#include "Graphics/Rendering/Lighting.h"
#include "Graphics/Rendering/Material.h"
#include "Graphics/Rendering/Mesh.h"
#include "Graphics/Rendering/Texture.h"
#include "Graphics/Vulkan/GraphicsDevice.h"
#include "Graphics/Vulkan/MemoryBuffer.h"
#include "Graphics/Vulkan/Vulkan.h"

using namespace Vulcan;
using glm::mat3;

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
	vkDeviceWaitIdle(Vulkan::Device()->Logical());
}

Renderer::Renderer(Config* config, GLFWwindow* window)
	: m_frameCmdBuf{ VK_NULL_HANDLE }, m_globalsUniform{  }
{
	m_instance = this;
	InitVulkan(config, window);

	m_vulkan = Vulkan::Instance();

	m_globalUniformBuffer = new MemoryBuffer{ sizeof(GlobalsUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT };

	uint64 dynamicAlignment = m_vulkan->GetDevice()->DynamicAlignment<mat4>();
	uint64 bufferSize = MAX_VISIBLE_OBJECTS * dynamicAlignment;
	m_transforms = static_cast<mat4*>(alignedAlloc(bufferSize, dynamicAlignment));
	m_transformBuffer = new MemoryBuffer{ bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, m_transforms, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT };
	m_transformBuffer->SetRange(dynamicAlignment);

	dynamicAlignment = m_vulkan->GetDevice()->DynamicAlignment<MaterialUniform>();
	bufferSize = MAX_VISIBLE_OBJECTS * dynamicAlignment;
	m_materials = static_cast<MaterialUniform*>(alignedAlloc(bufferSize, dynamicAlignment));
	m_materialBuffer = new MemoryBuffer{ bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, m_materials, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT };
	m_materialBuffer->SetRange(dynamicAlignment);
}

Renderer::~Renderer()
{
	alignedFree(m_transforms);
	alignedFree(m_materials);

	delete m_transformBuffer;
	delete m_materialBuffer;

	delete m_globalUniformBuffer;

	DestroyVulkan();
}

void Renderer::UpdateBuffer(Material* material, const uint32 objectIndex) const
{
	MaterialBindInfo bindInfo = {};

	bindInfo.objectIndex = objectIndex;
	bindInfo.materialUniforms = m_materials;

	material->FillBuffer(bindInfo);
}

void Renderer::Render(const Mesh* mesh, const TList<Material*>& materials, const uint32 objectIndex, const Lighting* lighting) const
{
	const MaterialBindInfo bindInfo =
	{
		.objectIndex = objectIndex,
		.materialUniforms = m_materials,
		.materialBuffer = m_materialBuffer,
		.transformsBuffer = m_transformBuffer,
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
}

void Renderer::EndFrame()
{
	m_vulkan->EndFrame(m_frameCmdBuf);
	m_frameCmdBuf = VK_NULL_HANDLE;
}

void Renderer::UpdateBuffers()
{
	m_currentCamera->GetPvm(m_globalsUniform);
	m_globalsUniform.rotationView = mat4(mat3(m_globalsUniform.view));

	m_globalsUniform.exposure = 4.5f;
	m_globalsUniform.gamma = 2.2f;
	m_globalsUniform.prefilteredCubeMipLevels = 1.f;
	m_globalsUniform.scaleIBLAmbient = 1.f;

	m_globalUniformBuffer->Fill(&m_globalsUniform);

	// Get all transforms that have changed since the last frame
	const GameInstance* game = Application::GetGameInstance();
	if (const TList<DirtyTransform> dirty = game->GetWorld()->GetRootActor()->CollectDirtyTransforms();
		!dirty.IsEmpty())
	{
		// Update the buffer for the transforms
		for (auto& [index, value] : dirty)
		{
			m_transforms[index] = value;
		}

		m_transformBuffer->Fill(m_transforms);
	}

	m_materialBuffer->Fill(m_materials);
}
