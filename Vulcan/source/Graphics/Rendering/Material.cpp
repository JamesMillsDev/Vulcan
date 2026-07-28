#include "Graphics/Rendering/Material.h"

#include <format>
#include <ImGui/imgui.h>

#include "Gameplay/Actors/Components/Rendering/LightComponent.h"
#include "Graphics/Renderer.h"
#include "Graphics/Rendering/Lighting.h"
#include "Graphics/Rendering/Texture.h"
#include "Graphics/Vulkan/GraphicsDevice.h"
#include "Graphics/Vulkan/GraphicsPipeline.h"
#include "Graphics/Vulkan/MemoryBuffer.h"
#include "Graphics/Vulkan/Vulkan.h"
#include "Utility/Collections/HashImpls.h"

using namespace Vulcan;

Material::Material(const string& shaderPath)
	: Material{ ShaderConfig{.name = shaderPath } }
{}

Material::Material(const ShaderConfig& shaderConfig)
	: color{ 0xffffffff }, emissiveTint{ 0x00000000 }, ao{ 0.f }, roughness{ .5f }, metallic{ .5f },
	alphaMask{ 1.f }, alphaMaskCutoff{ 0.f }, m_pipelineConfig{ shaderConfig }, m_pipeline{ nullptr },
	m_shouldUpdateDescriptors{ true }
{
	AddTextureMaps();
}

Material::~Material()
{
	for (TMapEntry<string, Texture*>* texture : m_textures)
	{
		delete texture->Value();
	}
	m_textures.Clear();

	delete m_pipeline;
	m_pipeline = nullptr;
}

uint64 Material::GetHashCode() const
{
	return HashAll(color, emissiveTint, ao, roughness, metallic);
}

void Material::SetTexture(const string& id, Texture* texture)
{
	m_textures[id] = texture;
}

#if _DEBUG
void Material::Dbg_ShowGui()
{
	if (!showDebugWindow)
	{
		return;
	}

	ImGui::PushID("Material");
	ImGui::Begin("Material");

	float colors[3] = { color.r, color.g, color.b };
	if (ImGui::ColorEdit3("Color", colors))
	{
		color = Color{ colors[0], colors[1], colors[2], color.a };
	}

	colors[0] = emissiveTint.r;
	colors[1] = emissiveTint.g;
	colors[2] = emissiveTint.b;
	if (ImGui::ColorEdit3("Emissive Color", colors))
	{
		emissiveTint = Color{ colors[0], colors[1], colors[2], emissiveTint.a };
	}

	ImGui::DragFloat("AO", &ao, .01f, 0.f, 1.f, "%.2f");
	ImGui::DragFloat("Roughness", &roughness, .01f, 0.f, 1.f, "%.2f");
	ImGui::DragFloat("Metallic", &metallic, .01f, 0.f, 1.f, "%.2f");

	ImGui::End();
	ImGui::PopID();
}
#endif

void Material::FillBuffer(const MaterialBindInfo& bindInfo)
{
	// Update the material uniform with this material's data
	bindInfo.materialUniforms[bindInfo.objectIndex] =
	{
		.color = color,
		.emissiveTint = emissiveTint,
		.ao = ao,
		.roughness = 1.f - roughness,
		.metallic = metallic,
		.alphaMask = alphaMask,
		.alphaMaskCutoff = alphaMaskCutoff,
		.baseColorMap = m_textures[BASE_COLOR_MAP_NAME] != nullptr ? m_textures[BASE_COLOR_MAP_NAME]->GetId() : -1,
		.normalMap = m_textures[NORMAL_MAP_NAME] != nullptr ? m_textures[NORMAL_MAP_NAME]->GetId() : -1,
		.ormMap = m_textures[ORM_MAP_NAME] != nullptr ? m_textures[ORM_MAP_NAME]->GetId() : -1,
		.emissiveMap = m_textures[EMISSIVE_MAP_NAME] != nullptr ? m_textures[EMISSIVE_MAP_NAME]->GetId() : -1,
		.heightMap = m_textures[HEIGHT_MAP_NAME] != nullptr ? m_textures[HEIGHT_MAP_NAME]->GetId() : -1,
	};
}

void Material::Bind(const VkCommandBuffer cmdBuffer, const MaterialBindInfo& bindInfo)
{
	ValidatePipeline();

	m_pipeline->Bind(cmdBuffer, bindInfo.objectIndex);

	// Update the descriptor sets if needed
	if (m_shouldUpdateDescriptors)
	{
		TList<VkWriteDescriptorSet> writes;

		InsertUniformWrite(writes, bindInfo.globalsBuffer, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		InsertUniformWrite(writes, bindInfo.sceneLightBuffer, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

		for (uint8 i = 0; i < MAX_LIGHT_COUNT; ++i)
		{
			InsertUniformWrite(writes, bindInfo.lightBuffers[i], 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, i);
		}

		InsertUniformWrite(writes, bindInfo.transformsBuffer, 3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
		InsertUniformWrite(writes, bindInfo.materialBuffer, 4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);

		UpdateDescriptorSets(writes);
		m_shouldUpdateDescriptors = false;
	}
}

void Material::UpdateDescriptorSets(TList<VkWriteDescriptorSet>& writes) const
{
	for (TMapEntry<string, Texture*>* texture : m_textures)
	{
		if (texture->Value() != nullptr)
		{
			InsertTextureWrite(writes, texture->Value(), 5);
		}
	}

	vkUpdateDescriptorSets(Vulkan::Device()->Logical(), static_cast<uint32>(writes.Count()), writes.Data(), 0, nullptr);
}

void Material::UpdateUniformDescriptor(const MemoryBuffer* buffer, uint32 binding) const
{
	const VkWriteDescriptorSet write =
	{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = nullptr,
		.dstSet = m_pipeline->GetDescriptorSet(),
		.dstBinding = binding,
		.dstArrayElement = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.pImageInfo = nullptr,
		.pBufferInfo = &buffer->GetBufferInfo(),
		.pTexelBufferView = nullptr
	};

	vkUpdateDescriptorSets(Vulkan::Device()->Logical(), 1, &write, 0, nullptr);
}

void Material::ValidatePipeline()
{
	if (m_pipeline == nullptr)
	{
		m_pipeline = new GraphicsPipeline{ m_pipelineConfig };
	}
}

void Material::InsertTextureWrite(TList<VkWriteDescriptorSet>& writes, const Texture* texture, const uint32 binding) const
{
	if (texture != nullptr)
	{
		writes.Add(
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = m_pipeline->GetDescriptorSet(),
				.dstBinding = binding,
				.dstArrayElement = static_cast<uint32>(texture->GetId()),
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = &texture->GetDescriptors(),
				.pBufferInfo = nullptr,
				.pTexelBufferView = nullptr
			}
		);
	}
}

void Material::InsertUniformWrite(TList<VkWriteDescriptorSet>& writes, const MemoryBuffer* buffer, const uint32 binding,
	const VkDescriptorType type, const uint32 arrayElem) const
{
	writes.Add(
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_pipeline->GetDescriptorSet(),
			.dstBinding = binding,
			.dstArrayElement = arrayElem,
			.descriptorCount = 1,
			.descriptorType = type,
			.pImageInfo = nullptr,
			.pBufferInfo = &buffer->GetBufferInfo(),
			.pTexelBufferView = nullptr
		});
}

void Material::AddTextureMaps()
{
	m_textures.Add(BASE_COLOR_MAP_NAME, nullptr);
	m_textures.Add(NORMAL_MAP_NAME, nullptr);
	m_textures.Add(ORM_MAP_NAME, nullptr);
	m_textures.Add(EMISSIVE_MAP_NAME, nullptr);
	m_textures.Add(HEIGHT_MAP_NAME, nullptr);
}
