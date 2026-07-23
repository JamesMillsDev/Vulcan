#pragma once

#include <string>

#include <vulkan/vulkan.h>

#include "Graphics/Uniforms.h"

#include "Utility/Collections/TList.h"
#include "Utility/Collections/TSet.h"

class Vulkan;

using std::string;

struct DescriptorConfig
{
	VkDescriptorType type;
	uint32 count;
	VkShaderStageFlags stage;
	VkDescriptorBindingFlags binding = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
};

struct ShaderConfig
{
	struct StageComp
	{
		bool operator()(const VkShaderStageFlagBits& lhs, const VkShaderStageFlagBits& rhs) const;
	};

public:
	TSet<VkShaderStageFlagBits, StageComp> stages = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
	TList<DescriptorConfig> descriptors = {};
	string name;
	string entryPoint = "main";
	bool lit = true;

};

struct RasterizerConfig
{
	VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
	VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
	bool depthBiasEnabled = VK_FALSE;
	bool depthClampEnabled = VK_FALSE;
	bool rasterizerDiscardEnabled = VK_FALSE;
	float lineWidth = 1.f;
};

struct ColorAttachmentConfig
{
	VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	bool blendEnabled = VK_FALSE;
};

struct ColorBlendStateConfig
{
	static constexpr int BLEND_CONSTANT_COUNT = 4;

	bool logicOpEnabled = VK_FALSE;
	VkLogicOp logicOp = VK_LOGIC_OP_COPY;
	float blendConstants[BLEND_CONSTANT_COUNT] = { 0.f, 0.f, 0.f, 0.f };
};

struct PrimitiveConfig
{
	VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	bool primitiveRestartEnabled = VK_FALSE;
};

struct MultisamplerConfig
{
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	bool sampleShadingEnabled = VK_FALSE;
};

struct GraphicsPipelineConfig
{
	friend class GraphicsPipeline;

public:
	ShaderConfig shaderConfig;
	RasterizerConfig rasterizer;
	ColorAttachmentConfig colorAttachment;
	ColorBlendStateConfig blendState;
	PrimitiveConfig primitive;
	MultisamplerConfig multisampler;
	TList<VkPushConstantRange> pushConstantRanges =
	{
		{
			.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
			.offset = 0,
			.size = sizeof(ProjectionViewModelUniform)
		}
	};

public:
	explicit GraphicsPipelineConfig(ShaderConfig shader);
	explicit GraphicsPipelineConfig(const string& shaderName);

public:
	[[nodiscard]] uint32 Size() const;
	[[nodiscard]] bool ContainsStage(VkShaderStageFlagBits stage) const;

};

class GraphicsPipeline
{
private:
	GraphicsPipelineConfig m_config;

	VkDescriptorPool m_descriptorPool;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorSet m_descriptorSets;
	TList<int32> m_samplerBindings;

	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_pipeline;
	VkPipelineBindPoint m_bindPoint;
	VkShaderStageFlagBits m_pushConstantStage;

public:
	explicit GraphicsPipeline(GraphicsPipelineConfig config);
	~GraphicsPipeline();

public:
	void Bind(VkCommandBuffer cmdBuffer, VkDeviceAddress pushConstantAddress) const;
	void SetBindPoint(VkPipelineBindPoint bindPoint);
	void SetPushConstantStage(VkShaderStageFlagBits stage);

	VkDescriptorSet GetDescriptorSet() const;
	bool IsLit() const;

	bool TryGetTextureBinding(TList<int32>& binding) const;

private:
	void Init(Vulkan* vulkan);
	void Destroy();

	void InitDescriptors(const Vulkan* vulkan);
	void InitPipeline(Vulkan* vulkan);

};