#pragma once

#include <string>
#include <glm/mat4x4.hpp>
#include <vulkan/vulkan.h>

#include "Object.h"

#include "Graphics/Vulkan/GraphicsPipeline.h"

#include "Maths/Color.h"

#include "Utility/Collections/TList.h"

class MemoryBuffer;
struct ShaderConfig;
class GraphicsPipeline;
class Texture;

using glm::mat4;
using std::string;

struct MaterialUniform
{
	Color color;
	Color emissiveTint;
	Color specularColor;

	float roughness;
	float metallic;
	float specularStrength;
};

class Material : public Object
{
	friend class Renderer;
		
public:
	Color color;
	Color emissiveTint;
	float roughness;
	float metallic;

	Color specularColor;
	float specularStrength;

private:
	GraphicsPipelineConfig m_pipelineConfig;
	GraphicsPipeline* m_pipeline;
	bool m_shouldUpdateDescriptors;

	TList<Texture*> m_textures;

public:
	explicit Material(const string& shaderPath);
	explicit Material(const ShaderConfig& shaderConfig);
	~Material() override;

public:
	[[nodiscard]] uint64 GetHashCode() const override;

	void AddTexture(Texture* texture);

private:
	void Bind(VkCommandBuffer cmdBuffer, const mat4& transform);
	void UpdateDescriptorSets(TList<VkWriteDescriptorSet>& writes) const;

	void InsertTextureWrite(TList<VkWriteDescriptorSet>& writes, const Texture* texture, uint32 binding) const;
	void InsertUniformWrite(TList<VkWriteDescriptorSet>& writes, const MemoryBuffer* buffer, uint32 binding, uint32 arrayElem = 0) const;

};