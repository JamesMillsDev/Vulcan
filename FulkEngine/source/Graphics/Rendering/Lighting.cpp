#include "Graphics/Rendering/Lighting.h"

#include <format>
#include <ImGui/imgui.h>

#include "Gameplay/Actors/Actor.h"
#include "Gameplay/Actors/Transform.h"
#include "Gameplay/Actors/World.h"
#include "Gameplay/Actors/Components/Rendering/LightComponent.h"
#include "Gameplay/Actors/Components/Rendering/MeshComponent.h"
#include "Graphics/Renderer.h"
#include "Graphics/Rendering/HdrTexture.h"
#include "Graphics/Rendering/Material.h"
#include "Graphics/Rendering/Mesh.h"
#include "Graphics/Rendering/Texture.h"
#include "Graphics/Vulkan/GraphicsPipeline.h"
#include "Graphics/Vulkan/MemoryBuffer.h"

using namespace Fulk;

const TArray LIGHT_NAMES =
{
	"Directional",
	"Point",
	"Spot"
};

Lighting::Lighting(World* world) :
	m_sceneLighting{ .ambientColor = Color{.313f, .313f, .313f, 1.f}, .ambientStrength = .01f },
	m_sceneLightingBuffer{ nullptr }
{
	m_lightBuffers.Resize(MAX_LIGHT_COUNT);

	GraphicsPipelineConfig skyboxConfig = GraphicsPipelineConfig{ ShaderConfig{.name = "Shaders/skybox"} };
	skyboxConfig.rasterizer.cullMode = VK_CULL_MODE_NONE;
	m_skyboxMesh = Mesh::MakeCube();
	m_skyboxMaterial = new Material{ skyboxConfig };
	m_skyboxTexture = HdrTexture::LoadFromFile("Textures\\T_DefaultSkybox");

	m_skyboxActor = world->MakeActor<Actor>();
	m_skyboxActor->MakeComponent<MeshComponent>(m_skyboxMesh, TList{ m_skyboxMaterial });
}

Lighting::~Lighting()
{
	delete m_skyboxMesh;
	delete m_skyboxTexture;
	delete m_skyboxMaterial;

	delete m_sceneLightingBuffer;
	for (MemoryBuffer*& lightBuffer : m_lightBuffers)
	{
		delete lightBuffer;
	}
}

void Lighting::UpdateBuffers()
{
	if (m_sceneLightingBuffer == nullptr)
	{
		m_sceneLightingBuffer = new MemoryBuffer{ sizeof(SceneLightingUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT };
		for (MemoryBuffer*& lightBuffer : m_lightBuffers)
		{
			lightBuffer = new MemoryBuffer{ sizeof(LightUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT };
		}
	}

	m_sceneLightingBuffer->Fill(&m_sceneLighting);

	for (uint8 i = 0; i < MAX_LIGHT_COUNT; ++i)
	{
		LightUniform lightUniform
		{
			.location = vec4{ 0.f },
			.direction = vec4{ 0.f },
			.color = Color::BLACK,
			.intensity = 0.f,
			.constant = 0.f,
			.linear = 0.f,
			.quadratic = 0.f,
			.cutOff = 0.f,
			.outerCutOff = 0.f,
			.type = static_cast<uint8>(LightComponent::EType::Directional),
			.enabled = 0
		};

		if (i < m_lights.Count())
		{
			LightComponent* light = m_lights[i];
			const Transform* transform = light->Owner()->GetTransform();

			lightUniform =
			{
				.location = vec4{ transform->Location(), 1.f },
				.direction = vec4{ transform->Forward(), 0.f },
				.color = light->color,
				.intensity = light->intensity,
				.constant = light->constant,
				.linear = light->linear,
				.quadratic = light->quadratic,
				.cutOff = light->cutOff,
				.outerCutOff = light->outerCutOff,
				.type = static_cast<uint8>(light->type),
				.enabled = 1
			};
		}

		m_lightBuffers[i]->Fill(&lightUniform);
	}
}

#if _DEBUG
void Lighting::Dbg_ShowGui()
{
	ImGui::Begin("Lighting");

	ImGui::PushID("Ambient_Lighting");
	if (ImGui::CollapsingHeader("Ambient"))
	{
		float colors[3] =
		{
			m_sceneLighting.ambientColor.r,
			m_sceneLighting.ambientColor.g,
			m_sceneLighting.ambientColor.b
		};

		if (ImGui::ColorEdit3("Color", colors))
		{
			m_sceneLighting.ambientColor = vec3{ colors[0], colors[1], colors[2] };
		}

		ImGui::SliderFloat("Strength", &m_sceneLighting.ambientStrength, 0.f, 1.f, "%.2f");
		ImGui::SliderFloat("IBL Lighting Scale", &Renderer::Instance()->globalsUniform.scaleIblAmbient, .01f, 10.f, "%.2f");
	}
	ImGui::PopID();

	ImGui::PushID("Scene_Lights");
	if (ImGui::CollapsingHeader("Lights"))
	{
		for (uint8 i = 0; i < MAX_LIGHT_COUNT; ++i)
		{
			if (i < m_lights.Count())
			{
				LightComponent* light = m_lights[i];

				string id = std::format("Light: {}", i + 1);
				ImGui::PushID(id.c_str());
				if (ImGui::CollapsingHeader(id.c_str()))
				{
					int itemIndex = static_cast<int>(light->type);
					if (ImGui::Combo("Type", &itemIndex, LIGHT_NAMES.Data(), static_cast<int>(LIGHT_NAMES.Count())))
					{
						light->type = static_cast<LightComponent::EType>(itemIndex);
					}

					ImGui::DragFloat("Intensity", &light->intensity, 1.f, 0.f, FLT_MAX, "%.2f");

					float colors[3] =
					{
						light->color.r,
						light->color.g,
						light->color.b
					};

					if (ImGui::ColorEdit3("Color", colors))
					{
						light->color = vec3{ colors[0], colors[1], colors[2] };
					}
				}
				ImGui::PopID();
			}
		}
	}
	ImGui::PopID();

	ImGui::End();
}
#endif // _DEBUG

void Lighting::AddLight(LightComponent* light)
{
	m_lights.Add(light);
}

void Lighting::RemoveLight(LightComponent* light)
{
	m_lights.Remove(light);
}