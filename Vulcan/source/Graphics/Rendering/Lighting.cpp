#include "Graphics/Rendering/Lighting.h"

#include <format>
#include <ImGui/imgui.h>

#include "Gameplay/Actors/Actor.h"
#include "Gameplay/Actors/Transform.h"
#include "Gameplay/Actors/Components/Rendering/LightComponent.h"
#include "Graphics/Vulkan/MemoryBuffer.h"
#include "Graphics/Vulkan/Vulkan.h"

using namespace Vulcan;

const TArray LIGHT_NAMES =
{
	"Directional",
	"Point",
	"Spot"
};

Lighting::Lighting()
	: m_sceneLighting{ .ambientColor = Color::WHITE, .ambientStrength = .2f }, m_sceneLightingBuffer{ nullptr }
{
	m_lightBuffers.Resize(MAX_LIGHT_COUNT);
}

Lighting::~Lighting()
{
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
				// multiply the direction by 50000 to make sure it normalizes to 1
				.direction = vec4{ glm::normalize(transform->Forward() * 50000.f), 0.f },
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