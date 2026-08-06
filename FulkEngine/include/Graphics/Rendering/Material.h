#pragma once

#include <string>
#include <glm/mat4x4.hpp>
#include <vulkan/vulkan.h>

#include "Debug.h"
#include "Object.h"

#include "Graphics/Vulkan/GraphicsPipeline.h"

#include "Maths/Color.h"

#include "Utility/Collections/TList.h"
#include "Utility/Collections/TMap.h"

using glm::mat4;
using std::string;

#define BASE_COLOR_MAP_NAME "Base Color Map"
#define NORMAL_MAP_NAME "Normal Map"
#define ORM_MAP_NAME "ORM Map"
#define EMISSIVE_MAP_NAME "Emissive Map"
#define HEIGHT_MAP_NAME "Height Map"

namespace Fulk
{
	class MemoryBuffer;
	struct ShaderConfig;
	class GraphicsPipeline;
	class Texture;

	struct MaterialUniform
	{
		vec4 color;
		vec4 emissiveTint;

		float ao;
		float roughness;
		float metallic;
		float alphaMask;
		float alphaMaskCutoff;

		int32 baseColorMap;
		int32 normalMap;
		int32 ormMap;
		int32 emissiveMap;
		int32 heightMap;
	};

	struct MaterialBindInfo
	{
		mat4 transform;
		MemoryBuffer* globalsBuffer;

		VkDescriptorImageInfo skyboxDescriptor;
		MemoryBuffer* sceneLightBuffer;
		TList<MemoryBuffer*> lightBuffers;
	};

	class Material : public Object
	{
		friend class Mesh;
		friend class Renderer;

	public:
	#if _DEBUG
		bool showDebugWindow = false;
	#endif

		Color color;
		Color emissiveTint;
		float ao;
		float roughness;
		float metallic;
		float alphaMask;
		float alphaMaskCutoff;

	private:
		GraphicsPipelineConfig m_pipelineConfig;
		GraphicsPipeline* m_pipeline;
		MemoryBuffer* m_materialBuffer;
		bool m_shouldUpdateDescriptors;

		TMap<string, Texture*> m_textures;

	public:
		explicit Material(const string& shaderPath);
		explicit Material(const ShaderConfig& shaderConfig);
		explicit Material(const GraphicsPipelineConfig& pipelineConfig);
		~Material() override;

	public:
		[[nodiscard]] uint64 GetHashCode() const override;

		void SetTexture(const string& id, Texture* texture);

		DEFINE_DEBUG_FUNCTION(ShowGui)

	private:
		void Bind(VkCommandBuffer cmdBuffer, const MaterialBindInfo& bindInfo);
		void UpdateDescriptorSets(TList<VkWriteDescriptorSet>& writes) const;
		void UpdateUniformDescriptor(const MemoryBuffer* buffer, uint32 binding) const;

		void ValidatePipeline();

		void InsertTextureWrite(TList<VkWriteDescriptorSet>& writes, const VkDescriptorImageInfo& descriptor, uint32 binding, uint32 id) const;
		void InsertUniformWrite(TList<VkWriteDescriptorSet>& writes, const MemoryBuffer* buffer, uint32 binding, VkDescriptorType type, uint32 arrayElem = 0) const;

		void AddTextureMaps();

	};
}