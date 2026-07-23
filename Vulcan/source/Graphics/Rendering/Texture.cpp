#include "Graphics/Rendering/Texture.h"

#include <format>
#include <ktxvulkan.h>
#include <stdexcept>

#include "Resources.h"

#include "Graphics/Vulkan/MemoryBuffer.h"
#include "Graphics/Vulkan/Vulkan.h"
#include "Utility/Collections/HashImpls.h"

using std::runtime_error;

uint32 Texture::m_nextId = 0;
queue<uint32> Texture::m_freeIds;

Texture::Texture(const string& fileName)
	: m_file{ fileName }, m_image{ VK_NULL_HANDLE }, m_imageAllocation{ VK_NULL_HANDLE },
	m_imageView{ VK_NULL_HANDLE }, m_sampler{ VK_NULL_HANDLE }, m_imageExtent{ },
	m_imageFormat{ }, m_buffer{ VK_NULL_HANDLE }, m_texture{ nullptr }, m_textureDescriptors{ }
{
	// Get the next available ID (reusing old ones)
	if (m_freeIds.empty())
	{
		m_id = m_nextId++;
	}
	else
	{
		m_id = m_freeIds.front();
		m_freeIds.pop();
	}

	CreateBuffer();
}

Texture::~Texture()
{
	m_freeIds.push(m_id);

	DestroyBuffer();
}

uint64 Texture::GetHashCode() const
{
	return HashValue(m_id);
}

const VkDescriptorImageInfo& Texture::GetDescriptors() const
{
	return m_textureDescriptors;
}

uint32 Texture::GetId() const
{
	return m_id;
}

void Texture::CreateBuffer()
{
	// Attempt to load the texture from memory
	const string file = m_file + ".ktx2";
	ResourceData textureData = Resources::Find(file);

	if (ktx_error_code_e error = ktxTexture2_CreateFromMemory(textureData.data, textureData.length, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &m_texture);
		error != KTX_SUCCESS)
	{
		throw runtime_error(std::format("Failed to load texture from file! Error Code: {}", static_cast<int32>(error)));
	}

	ktxTexture2_TranscodeBasis(m_texture, KTX_TTF_BC7_RGBA, KTX_TF_HIGH_QUALITY);
	// Get the format and extent from the texture
	m_imageFormat = static_cast<VkFormat>(m_texture->vkFormat);
	m_imageExtent.width = m_texture->baseWidth;
	m_imageExtent.height = m_texture->baseHeight;
	m_imageExtent.depth = 1;

	// Generate the create info
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = m_imageFormat;
	imageCreateInfo.extent = m_imageExtent;
	imageCreateInfo.mipLevels = m_texture->numLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkResult result;

	// Attempt to create the image
	VmaAllocationCreateInfo texImageAlloc{};
	texImageAlloc.usage = VMA_MEMORY_USAGE_AUTO;
	if (result = vmaCreateImage(Vulkan::Allocator(), &imageCreateInfo, &texImageAlloc, &m_image, &m_imageAllocation, nullptr);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Image from texture!", result);
	}

	// Attempt to create the image view from the image
	VkImageViewCreateInfo viewCreateInfo{};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = m_image;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = m_imageFormat;

	viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCreateInfo.subresourceRange.levelCount = m_texture->numLevels;
	viewCreateInfo.subresourceRange.layerCount = 1;

	if (result = vkCreateImageView(Vulkan::Device(), &viewCreateInfo, nullptr, &m_imageView);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Image View from texture!", result);
	}

	// Generate the buffer and transition
	m_buffer = new MemoryBuffer{ m_texture->dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Vulkan::Instance() };
	m_buffer->Fill(m_texture->pData);

	TransitionImage();

	delete m_buffer;
	m_buffer = nullptr;

	// Generate the sampler
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 8.f;
	samplerInfo.maxLod = static_cast<float>(m_texture->numLevels);

	if (result = vkCreateSampler(Vulkan::Device(), &samplerInfo, nullptr, &m_sampler);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Sampler from image!", result);
	}

	// Destroy the texture and set up the descriptors
	ktxTexture2_Destroy(m_texture);
	m_texture = nullptr;

	m_textureDescriptors.sampler = m_sampler;
	m_textureDescriptors.imageView = m_imageView;
	m_textureDescriptors.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

}

void Texture::DestroyBuffer() const
{
	vkDestroyImageView(Vulkan::Device(), m_imageView, nullptr);
	vkDestroySampler(Vulkan::Device(), m_sampler, nullptr);
	vmaDestroyImage(Vulkan::Allocator(), m_image, m_imageAllocation);
}

void Texture::TransitionImage() const
{
	// Begin the one-time command
	Vulkan* vulkan = Vulkan::Instance();
	VkCommandBuffer commandBuffer;
	VkFence fence;
	vulkan->BeginOneTimeCommand(commandBuffer, fence);

	// Set up the memory barriers and dependency information
	VkImageMemoryBarrier2 barrierTexImage{};
	barrierTexImage.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrierTexImage.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
	barrierTexImage.srcAccessMask = VK_ACCESS_2_NONE;
	barrierTexImage.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
	barrierTexImage.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
	barrierTexImage.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrierTexImage.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrierTexImage.image = m_image;
	barrierTexImage.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrierTexImage.subresourceRange.levelCount = m_texture->numLevels;
	barrierTexImage.subresourceRange.layerCount = 1;

	VkDependencyInfo barrierTexInfo{};
	barrierTexInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	barrierTexInfo.imageMemoryBarrierCount = 1;
	barrierTexInfo.pImageMemoryBarriers = &barrierTexImage;

	// Run the transition
	vkCmdPipelineBarrier2(commandBuffer, &barrierTexInfo);

	// Get the regions to copy and then copy them
	TList<VkBufferImageCopy> copyRegions;
	copyRegions.Resize(m_texture->numLevels);

	for (uint32 i = 0; i < m_texture->numLevels; ++i)
	{
		ktx_size_t mipOffset = 0;
		ktxTexture2_GetImageOffset(m_texture, i, 0, 0, &mipOffset);

		VkBufferImageCopy& copy = copyRegions[i];

		// Assign the copy regions
		copy.bufferOffset = mipOffset;
		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.imageSubresource.mipLevel = i;
		copy.imageSubresource.layerCount = 1;
		copy.bufferOffset = mipOffset;
		copy.imageExtent = { .width = m_texture->baseWidth >> i, .height = m_texture->baseHeight >> i, .depth = 1 };
	}
	vkCmdCopyBufferToImage(
		commandBuffer, m_buffer->Get(), m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32>(copyRegions.size()), copyRegions.Data()
	);

	// Make the barrier readable
	VkImageMemoryBarrier2 barrierTexRead{};
	barrierTexRead.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrierTexRead.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
	barrierTexRead.srcAccessMask = VK_ACCESS_2_NONE;
	barrierTexRead.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
	barrierTexRead.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
	barrierTexRead.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrierTexRead.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
	barrierTexRead.image = m_image;
	barrierTexRead.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrierTexRead.subresourceRange.levelCount = m_texture->numLevels;
	barrierTexRead.subresourceRange.layerCount = 1;

	// Submit the pipeline command
	barrierTexInfo.pImageMemoryBarriers = &barrierTexRead;
	vkCmdPipelineBarrier2(commandBuffer, &barrierTexInfo);

	vulkan->EndOneTimeCommand(commandBuffer, fence);
}
