#include "Graphics/Rendering/Material.h"

#include "Gameplay/Actors/Components/Rendering/LightComponent.h"

#include "Graphics/Renderer.h"
#include "Graphics/Uniforms.h"
#include "Graphics/Rendering/Camera.h"
#include "Graphics/Rendering/SceneLightingData.h"
#include "Graphics/Rendering/Texture.h"
#include "Graphics/Vulkan/GraphicsPipeline.h"
#include "Graphics/Vulkan/MemoryBuffer.h"
#include "Graphics/Vulkan/Vulkan.h"

#include "Utility/Collections/HashImpls.h"

Material::Material(const string& shaderPath) :
	color{ 0xffffffff }, emissiveTint{ 0x00000000 }, roughness{ 0 }, metallic{ 0 },
	specularColor{ Color::WHITE }, specularStrength{ .5f }, m_pipelineConfig{ shaderPath },
	m_pipeline{ nullptr }, m_shouldUpdateDescriptors{ true }
{}

Material::Material(const ShaderConfig& shaderConfig) :
	color{ 0xffffffff }, emissiveTint{ 0x00000000 }, roughness{ 0 }, metallic{ 0 },
	specularColor{ Color::WHITE }, specularStrength{ .5f }, m_pipelineConfig{ shaderConfig },
	m_pipeline{ nullptr }, m_shouldUpdateDescriptors{ true }
{}

Material::~Material()
{
	for (Texture*& texture : m_textures)
	{
		delete texture;
	}
	m_textures.Clear();

	delete m_pipeline;
	m_pipeline = nullptr;
}

uint64 Material::GetHashCode() const
{
	return HashAll(
		color, emissiveTint, roughness, metallic,
		specularColor, specularStrength
	);
}

void Material::AddTexture(Texture* texture)
{
	m_textures.Add(texture);
}

void Material::Bind(const VkCommandBuffer cmdBuffer, const mat4& transform)
{
	if (m_pipeline == nullptr)
	{
		for (int64 i = 0; i < m_textures.Count(); ++i)
		{
			m_pipelineConfig.shaderConfig.descriptors.Add(
				{
					.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.count = 1,
					.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
					.binding = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
				}
			);
		}

		m_pipeline = new GraphicsPipeline{ m_pipelineConfig };
	}

	// Update the material uniform with this material's data
	const MemoryBuffer* materialBuffer = Vulkan::Instance()->GetUniformBuffer(EUniformBufferIds::Material);
	const MaterialUniform materialUniform
	{
		.color = color,
		.emissiveTint = emissiveTint,
		.specularColor = specularColor,
		.roughness = roughness,
		.metallic = metallic,
		.specularStrength = specularStrength
	};
	materialBuffer->Fill(&materialUniform);

	// Update the transform buffer with our object's transform
	const MemoryBuffer* uboBuffer = Vulkan::Instance()->GetUniformBuffer(EUniformBufferIds::ProjectionView);
	ProjectionViewModelUniform pvm;
	Renderer::GetCurrentCamera()->GetPvm(pvm);

	pvm.model = transform;
	uboBuffer->Fill(&pvm);

	// TODO: Use more dynamic lighting. This is a test
	const MemoryBuffer* light0Buffer = Vulkan::Instance()->GetUniformBuffer(EUniformBufferIds::Lights, 0);
	LightUniform light0
	{
		.location = { 0.f, 0.f, 0.f },
		.direction = { .32f, -.77f, -.56f }, // this is the unity default light direction
		.color = Color::WHITE,
		.type = 0,
	};
	light0Buffer->Fill(&light0);

	const MemoryBuffer* sceneLightBuffer = Vulkan::Instance()->GetUniformBuffer(EUniformBufferIds::SceneLighting);
	SceneLightingData sceneLighting
	{
		.ambientColor = Color::WHITE,
		.ambientStrength = .1f
	};
	sceneLightBuffer->Fill(&sceneLighting);

	// Bind the pipeline and push the push constants to the command buffer
	m_pipeline->Bind(cmdBuffer, uboBuffer->GetAddress());

	// Update the descriptor sets if needed
	if (m_shouldUpdateDescriptors)
	{
		TList<VkWriteDescriptorSet> writes;

		InsertUniformWrite(writes, sceneLightBuffer, 0);
		InsertUniformWrite(writes, light0Buffer, 1);
		InsertUniformWrite(writes, materialBuffer, 2);

		UpdateDescriptorSets(writes);
		m_shouldUpdateDescriptors = false;
	}
}

void Material::UpdateDescriptorSets(TList<VkWriteDescriptorSet>& writes) const
{
	TList<int32> textureBindings;
	if (m_pipeline->TryGetTextureBinding(textureBindings))
	{
		for (int64 i = 0; i < textureBindings.Count(); ++i)
		{
			InsertTextureWrite(writes, m_textures[i], textureBindings[i]);
		}
	}

	vkUpdateDescriptorSets(Vulkan::Device(), static_cast<uint32>(writes.Count()), writes.Data(), 0, nullptr);
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
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = &texture->GetDescriptors(),
				.pBufferInfo = nullptr,
				.pTexelBufferView = nullptr
			}
		);
	}
}

void Material::InsertUniformWrite(TList<VkWriteDescriptorSet>& writes, const MemoryBuffer* buffer, const uint32 binding, const uint32 arrayElem) const
{
	writes.Add(
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_pipeline->GetDescriptorSet(),
			.dstBinding = binding,
			.dstArrayElement = arrayElem,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pImageInfo = nullptr,
			.pBufferInfo = &buffer->GetBufferInfo(),
			.pTexelBufferView = nullptr
		});
}
