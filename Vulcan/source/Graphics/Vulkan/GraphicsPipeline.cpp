#include "Graphics/Vulkan/GraphicsPipeline.h"

#include "Gameplay/Actors/Components/Rendering/LightComponent.h"
#include "Graphics/Rendering/Lighting.h"
#include "Graphics/Rendering/Material.h"
#include "Graphics/Rendering/Mesh.h"
#include "Graphics/Rendering/Shader.h"
#include "Graphics/Vulkan/GraphicsDevice.h"
#include "Graphics/Vulkan/Vulkan.h"

using namespace Vulcan;

bool ShaderConfig::StageComp::operator()(const VkShaderStageFlagBits& lhs, const VkShaderStageFlagBits& rhs) const
{
	return lhs < rhs;
}

GraphicsPipelineConfig::GraphicsPipelineConfig(ShaderConfig shader)
	: shaderConfig{ std::move(shader) }
{

}

GraphicsPipelineConfig::GraphicsPipelineConfig(const string& shaderName)
	: GraphicsPipelineConfig{ ShaderConfig{.name = shaderName } }
{

}

uint32 GraphicsPipelineConfig::Size() const
{
	return shaderConfig.stages.Count();
}

bool GraphicsPipelineConfig::ContainsStage(const VkShaderStageFlagBits stage) const
{
	return shaderConfig.stages.Contains(stage);
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipelineConfig config) :
	m_config{ std::move(config) }, m_bindPoint{ VK_PIPELINE_BIND_POINT_GRAPHICS }
{
	Init(Vulkan::Device()->Logical());
}

GraphicsPipeline::~GraphicsPipeline()
{
	Destroy();
}

void GraphicsPipeline::Bind(const VkCommandBuffer cmdBuffer, const uint32 objectIndex) const
{
	TArray dynamicOffsets = 
	{
		objectIndex * static_cast<uint32>(Vulkan::Device()->DynamicAlignment<mat4>()),
		objectIndex * static_cast<uint32>(Vulkan::Device()->DynamicAlignment<MaterialUniform>()),
	}; 

	vkCmdBindDescriptorSets(
		cmdBuffer, m_bindPoint, m_pipelineLayout, 0, 1, &m_descriptorSets,
		dynamicOffsets.Count(), dynamicOffsets.Data()
	);

	vkCmdBindPipeline(cmdBuffer, m_bindPoint, m_pipeline);
}

void GraphicsPipeline::SetBindPoint(const VkPipelineBindPoint bindPoint)
{
	m_bindPoint = bindPoint;
}

VkDescriptorSet GraphicsPipeline::GetDescriptorSet() const
{
	return m_descriptorSets;
}

void GraphicsPipeline::Init(const VkDevice& device)
{
	InitDescriptors(device);
	InitPipeline(device);
}

void GraphicsPipeline::Destroy()
{
	const GraphicsDevice* device = Vulkan::Device();

	vkDestroyDescriptorPool(device->Logical(), m_descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device->Logical(), m_descriptorSetLayout, nullptr);

	vkDestroyPipeline(device->Logical(), m_pipeline, nullptr);
	vkDestroyPipelineLayout(device->Logical(), m_pipelineLayout, nullptr);

	m_pipelineLayout = VK_NULL_HANDLE;
	m_pipeline = VK_NULL_HANDLE;
}

void GraphicsPipeline::InitDescriptors(const VkDevice& device)
{
	VkResult result;

	TArray dslBindings =
	{
		VkDescriptorSetLayoutBinding
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = 2,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = MAX_LIGHT_COUNT,
			.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = 3,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = 4,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = 5,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = UINT16_MAX,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = nullptr
		}
	};
	TArray flags =
	{
		VkDescriptorBindingFlags{ VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT },
		VkDescriptorBindingFlags{ VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT },
		VkDescriptorBindingFlags{ VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT },
		VkDescriptorBindingFlags{ VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT },
		VkDescriptorBindingFlags{ VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT },
		VkDescriptorBindingFlags{ VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT }
	};
	TArray poolSizes =
	{
		VkDescriptorPoolSize
		{
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1
		},
		VkDescriptorPoolSize
		{
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			.descriptorCount = 1
		},
		VkDescriptorPoolSize
		{
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			.descriptorCount = 1
		},
		VkDescriptorPoolSize
		{
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1
		},
		VkDescriptorPoolSize
		{
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = MAX_LIGHT_COUNT
		},
		VkDescriptorPoolSize
		{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = UINT16_MAX
		}
	};

	const VkDescriptorSetLayoutBindingFlagsCreateInfo dslFlagsCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		.pNext = nullptr,
		.bindingCount = flags.Count(),
		.pBindingFlags = flags.Data()
	};

	const VkDescriptorSetLayoutCreateInfo dslCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = &dslFlagsCreateInfo,
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
		.bindingCount = dslBindings.Count(),
		.pBindings = dslBindings.Data()
	};

	if (result = vkCreateDescriptorSetLayout(device, &dslCreateInfo, nullptr, &m_descriptorSetLayout);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Descriptor Set Layout!", result);
	}

	const VkDescriptorPoolCreateInfo dpCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
		.maxSets = 1,
		.poolSizeCount = poolSizes.Count(),
		.pPoolSizes = poolSizes.Data()
	};

	if (result = vkCreateDescriptorPool(device, &dpCreateInfo, nullptr, &m_descriptorPool);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Descriptor Pool!", result);
	}

	uint32 dynamicDescriptorCount = { UINT16_MAX };

	VkDescriptorSetVariableDescriptorCountAllocateInfo vdsAllocateInfo =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
		.pNext = nullptr,
		.descriptorSetCount = 1,
		.pDescriptorCounts = &dynamicDescriptorCount
	};

	const VkDescriptorSetAllocateInfo dsAllocateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = &vdsAllocateInfo,
		.descriptorPool = m_descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &m_descriptorSetLayout
	};

	if (result = vkAllocateDescriptorSets(device, &dsAllocateInfo, &m_descriptorSets);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Descriptor Pool!", result);
	}
}

void GraphicsPipeline::InitPipeline(const VkDevice& device)
{
	VkResult result;
	// Attempt to create the pipeline layout
	const VkPipelineLayoutCreateInfo plCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.setLayoutCount = 1,
		.pSetLayouts = &m_descriptorSetLayout,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = nullptr
	};

	if (result = vkCreatePipelineLayout(device, &plCreateInfo, nullptr, &m_pipelineLayout);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Pipeline Layout!", result);
	}

	const ShaderConfig& shaderConfig = m_config.shaderConfig;
	Shader* shader = new Shader{ shaderConfig.name };
	TList<VkPipelineShaderStageCreateInfo> ssCreateInfos;
	for (int32 i = VK_SHADER_STAGE_VERTEX_BIT; i < VK_SHADER_STAGE_ALL_GRAPHICS; i <<= 1)
	{
		if (!m_config.ContainsStage(static_cast<VkShaderStageFlagBits>(i)))
		{
			continue;
		}

		// Generate the stage's create info
		VkPipelineShaderStageCreateInfo ssCreateInfo{};
		ssCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ssCreateInfo.stage = static_cast<VkShaderStageFlagBits>(i);
		ssCreateInfo.module = shader->GetShaderModule();
		ssCreateInfo.pName = shaderConfig.entryPoint.c_str();

		ssCreateInfos.Add(ssCreateInfo);
	}

	auto bindingDescription = Vertex::GetBindingDescription();
	auto attributeDescription = Vertex::GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.Data();

	const auto& [topology, primitiveRestartEnabled] = m_config.primitive;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = topology;
	inputAssembly.primitiveRestartEnable = primitiveRestartEnabled;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	const auto& [frontFace, cullMode, polygonMode, depthBiasEnabled,
		depthClampEnabled, rasterizerDiscardEnabled, lineWidth] = m_config.rasterizer;
	VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
	rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerInfo.depthClampEnable = depthClampEnabled;
	rasterizerInfo.rasterizerDiscardEnable = rasterizerDiscardEnabled;
	rasterizerInfo.polygonMode = polygonMode;
	rasterizerInfo.lineWidth = lineWidth;
	rasterizerInfo.cullMode = cullMode;
	rasterizerInfo.frontFace = frontFace;
	rasterizerInfo.depthBiasEnable = depthBiasEnabled;

	const auto& [samples, sampleShadingEnabled] = m_config.multisampler;
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = sampleShadingEnabled;
	multisampling.rasterizationSamples = samples;

	const auto& [colorWriteMask, blendEnabled] = m_config.colorAttachment;
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = colorWriteMask;
	colorBlendAttachment.blendEnable = blendEnabled;

	const auto& [logicOpEnabled, logicOp, blendConstants] = m_config.blendState;
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = logicOpEnabled;
	colorBlending.logicOp = logicOp;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	for (uint32 i = 0; i < ColorBlendStateConfig::BLEND_CONSTANT_COUNT; ++i)
	{
		colorBlending.blendConstants[i] = blendConstants[i];
	}

	TList dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.Data();

	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

	constexpr VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
	VkPipelineRenderingCreateInfo renderingCreateInfo{};
	renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingCreateInfo.colorAttachmentCount = 1;
	renderingCreateInfo.pColorAttachmentFormats = &imageFormat;
	renderingCreateInfo.depthAttachmentFormat = Vulkan::Instance()->GetDepthFormat();

	VkGraphicsPipelineCreateInfo pCreateInfo{};
	pCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pCreateInfo.pNext = &renderingCreateInfo;
	pCreateInfo.stageCount = static_cast<uint32_t>(ssCreateInfos.size());
	pCreateInfo.pStages = ssCreateInfos.Data();
	pCreateInfo.pVertexInputState = &vertexInputInfo;
	pCreateInfo.pInputAssemblyState = &inputAssembly;
	pCreateInfo.pViewportState = &viewportState;
	pCreateInfo.pRasterizationState = &rasterizerInfo;
	pCreateInfo.pMultisampleState = &multisampling;
	pCreateInfo.pColorBlendState = &colorBlending;
	pCreateInfo.pDepthStencilState = &depthStencilState;
	pCreateInfo.pDynamicState = &dynamicState;
	pCreateInfo.layout = m_pipelineLayout;

	// Attempt to create the pipeline
	if (result = vkCreateGraphicsPipelines(device, nullptr, 1, &pCreateInfo, nullptr, &m_pipeline);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Graphics Pipeline!", result);
	}

	delete shader;
}
