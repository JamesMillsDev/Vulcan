#include "Graphics/Rendering/Texture.h"

#include <format>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>

#include "Resources.h"
#include "Graphics/Vulkan/CommandManager.h"
#include "Graphics/Vulkan/GraphicsDevice.h"
#include "Graphics/Vulkan/MemoryBuffer.h"
#include "Graphics/Vulkan/Vulkan.h"
#include "Utility/Console.h"
#include "Utility/Collections/HashImpls.h"

using namespace Vulcan;

struct StbiTexture
{
	int32 w;
	int32 h;
	int32 channels;
	uint8* pixels;
};

const TArray TEXTURE_EXTENSIONS =
{
	".png",
	".tga",
	".jpg",
	".bmp",
	".hdr"
};

const TMap<uint8, VkFormat> FORMATS =
{
	{.key = static_cast<uint8>(Greyscale), .value = VK_FORMAT_R8_UNORM },
	{.key = static_cast<uint8>(Greyscale | SRgb), .value = VK_FORMAT_R8_SRGB },
	{.key = static_cast<uint8>(Greyscale | Alpha), .value = VK_FORMAT_R8G8_UNORM },
	{.key = static_cast<uint8>(Greyscale | SRgb | Alpha), .value = VK_FORMAT_R8G8_SRGB },
	{.key = static_cast<uint8>(Greyscale | Alpha), .value = VK_FORMAT_R8G8_UNORM },
	{.key = static_cast<uint8>(Greyscale | SRgb | Alpha), .value = VK_FORMAT_R8G8_SRGB },
	{.key = static_cast<uint8>(Rgb), .value = VK_FORMAT_R8G8B8A8_UNORM },
	{.key = static_cast<uint8>(Rgb | SRgb), .value = VK_FORMAT_R8G8B8_SRGB },
	{.key = static_cast<uint8>(Rgb | Alpha), .value = VK_FORMAT_R8G8B8A8_UNORM },
	{.key = static_cast<uint8>(Rgb | SRgb | Alpha), .value = VK_FORMAT_R8G8B8A8_SRGB },
	{.key = static_cast<uint8>(Hdr | Rgb), .value = VK_FORMAT_R16G16B16_SFLOAT },
	{.key = static_cast<uint8>(Hdr | Rgb | Alpha), .value = VK_FORMAT_R16G16B16A16_SFLOAT },
};

using std::runtime_error;

int32 Texture::m_nextId = 0;
queue<int32> Texture::m_freeIds;

Texture* Texture::LoadFromFile(const string& fileName, const TextureLoadInfo& loadInfo)
{
	bool found = false;
	ResourceData resourceData = {};

	for (const char* ext : TEXTURE_EXTENSIONS)
	{
		try
		{
			// Attempt to load the texture from memory
			const string file = fileName + ext;
			resourceData = Resources::Find(file);
			found = true;
			break;
		}
		catch ([[maybe_unused]] runtime_error& error)
		{
			continue;
		}
	}

	if (!found)
	{
		Console::Exception("Texture for filename: '" + fileName + "' not found!");
		return nullptr;
	}

	Texture* texture = new Texture;
	TList<uint8> data;
	data.SetData(resourceData.data, resourceData.length);

	TList<TList<uint8>> textureData;
	textureData.Add(data);

	texture->SetTextureInfo(loadInfo);

	texture->m_vulkanTexture = new VulkanTexture{ textureData, texture };

	return texture;
}

Texture* Texture::LoadCubeMapFromFile(const string& baseFileName, const TList<string>& fileNames, const TextureLoadInfo& loadInfo)
{
	TList<ResourceData> resourceData;

	for (string& fileName : fileNames)
	{
		for (const char* ext : TEXTURE_EXTENSIONS)
		{
			try
			{
				// Attempt to load the texture from memory
				const string file = baseFileName + fileName + ext;
				const ResourceData data = Resources::Find(file);

				resourceData.Add(data);
				break;
			}
			catch ([[maybe_unused]] runtime_error& error)
			{
				continue;
			}
		}
	}

	if (resourceData.IsEmpty())
	{
		Console::Exception("Texture for filename: '" + baseFileName + "' not found!");
		return nullptr;
	}

	if (resourceData.Count() != 6)
	{
		Console::Exception("CubeMap loading failed! Expected 6 images, but only loaded " + std::to_string(resourceData.Count()));
		return nullptr;
	}

	Texture* texture = new Texture;

	TList<TList<uint8>> textureData;
	for (int64 i = 0; i < resourceData.Count(); ++i)
	{
		textureData.Add(TList<uint8>{});
		textureData[i].SetData(resourceData[i].data, resourceData[i].length);
	}

	texture->SetIsCubeMap(true);
	texture->SetTextureInfo(loadInfo);

	texture->m_vulkanTexture = new VulkanTexture{ textureData, texture };

	return texture;
}

Texture::Texture()
	: m_vulkanTexture{ nullptr }, m_width{ 0 }, m_height{ 0 }, m_channels{ 4 }, m_isNormal{ false }, m_isSrgb{ false },
	m_isCubeMap{ false }, m_isGreyscale{ false }, m_isHdr{ false }, m_mipLevels{ 1 }, m_format{},
	m_bitsPerChannel{ 8 }, m_greenChannelFlipped{ false }
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
}

Texture::~Texture()
{
	m_freeIds.push(m_id);

	if (m_vulkanTexture != nullptr)
	{
		m_vulkanTexture->DestroyBuffer();
		delete m_vulkanTexture;
	}
}

uint64 Texture::GetHashCode() const
{
	return HashValue(m_id);
}

const VkDescriptorImageInfo& Texture::GetDescriptors() const
{
	return m_vulkanTexture->m_textureDescriptors;
}

int32 Texture::GetId() const
{
	return m_id;
}

void Texture::SetTextureInfo(const TextureLoadInfo& info)
{
	m_isSrgb = info.sRgb;
	m_isNormal = info.normalMap;
	m_greenChannelFlipped = info.invertGreen;
	m_channels = info.channels;
	m_isGreyscale = info.greyscale;
	m_mipLevels = info.mipLevels;
	m_isHdr = info.hdr;
}

VkFormat Texture::VulkanTexture::GetVulkanFormat(const Texture* texture)
{
	uint8 mask = texture->GetIsGreyscale() ? Greyscale : Rgb;

	if (texture->GetIsSrgb())
	{
		mask |= SRgb;
	}
	else if (texture->GetIsHdr())
	{
		mask |= Hdr;
	}

	if (texture->GetChannels() == 4)
	{
		mask |= Alpha;
	}

	return FORMATS[mask];
}

Texture::VulkanTexture::VulkanTexture(const TList<TList<uint8>>& textureBinary, Texture* texture)
	: m_image{ VK_NULL_HANDLE }, m_imageAllocation{ VK_NULL_HANDLE },
	m_imageView{ VK_NULL_HANDLE }, m_sampler{ VK_NULL_HANDLE }, m_imageExtent{ },
	m_imageFormat{  }, m_textureDescriptors{ }, m_buffer{ VK_NULL_HANDLE }
{
	CreateBuffer(textureBinary, texture);
}

void Texture::VulkanTexture::CreateBuffer(const TList<TList<uint8>>& textureBinary, Texture* texture)
{
	TList<StbiTexture> stbiTextures;
	uint32 maxW = 0, maxH = 0;

	for (TList<uint8>& textureData : textureBinary)
	{
		StbiTexture stbiTexture;
		stbiTexture.pixels = stbi_load_from_memory(
			textureData.Data(), static_cast<int32>(textureData.Count()), &stbiTexture.w, &stbiTexture.h,
			&stbiTexture.channels, static_cast<int32>(texture->GetChannels())
		);

		if (stbiTexture.pixels == nullptr)
		{
			throw runtime_error("Failed to load texture!");
		}

		// Flip the green channel if necessary
		if (texture->GetIsNormal() && texture->GetGreenChannelFlipped())
		{
			for (int32 i = 0; i < stbiTexture.w * stbiTexture.h; ++i)
			{
				const int32 index = i * static_cast<int32>(texture->GetChannels());
				stbiTexture.pixels[index + 1] = 255 - stbiTexture.pixels[index + 1];
			}
		}

		maxW = std::max(static_cast<uint32>(stbiTexture.w), maxW);
		maxH = std::max(static_cast<uint32>(stbiTexture.h), maxH);
		stbiTextures.Add(stbiTexture);
	}

	m_imageFormat = GetVulkanFormat(texture);
	m_imageExtent = { .width = maxW, .height = maxH, .depth = 1 };

	texture->SetWidth(m_imageExtent.width);
	texture->SetHeight(m_imageExtent.height);
	texture->SetFormat(m_imageFormat);

	// Generate the creation info
	VkImageCreateInfo imageCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = texture->GetIsCubeMap() ? static_cast<VkImageCreateFlags>(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) : 0,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = m_imageFormat,
		.extent = m_imageExtent,
		.mipLevels = texture->GetMipLevels(),
		.arrayLayers = static_cast<uint32>(stbiTextures.Count()),
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

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
	viewCreateInfo.viewType = texture->GetIsCubeMap() ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = m_imageFormat;

	viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCreateInfo.subresourceRange.levelCount = texture->GetMipLevels();
	viewCreateInfo.subresourceRange.layerCount = static_cast<uint32>(stbiTextures.Count());

	const GraphicsDevice* device = Vulkan::Device();

	if (result = vkCreateImageView(device->Logical(), &viewCreateInfo, nullptr, &m_imageView);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Image View from texture!", result);
	}

	uint64 singleTextureLength = static_cast<uint64>(maxW * maxH) * texture->GetChannels();
	// Generate the buffer and transition
	m_buffer = new MemoryBuffer
	{
		singleTextureLength * stbiTextures.Count(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT
	};

	for (int64 i = 0; i < stbiTextures.Count(); ++i)
	{
		m_buffer->Fill(stbiTextures[i].pixels, singleTextureLength, i * singleTextureLength);
	}

	// Transition the image
	const CommandManager* cmdManager = Vulkan::CmdManager();
	cmdManager->ImmediateSubmit([&](const VkCommandBuffer buffer)
		{
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
			barrierTexImage.subresourceRange.levelCount = texture->GetMipLevels();
			barrierTexImage.subresourceRange.layerCount = static_cast<uint32>(stbiTextures.Count());

			VkDependencyInfo barrierTexInfo{};
			barrierTexInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
			barrierTexInfo.imageMemoryBarrierCount = 1;
			barrierTexInfo.pImageMemoryBarriers = &barrierTexImage;

			// Run the transition
			vkCmdPipelineBarrier2(buffer, &barrierTexInfo);

			// Get the regions to copy and then copy them
			TList<VkBufferImageCopy> copyRegions;
			copyRegions.Resize(stbiTextures.Count());

			for (int64 i = 0; i < copyRegions.Count(); ++i)
			{
				VkBufferImageCopy& copy = copyRegions[i];

				// Assign the copy regions
				copy = VkBufferImageCopy{};
				copy.bufferOffset = static_cast<VkDeviceSize>(i) *
					static_cast<VkDeviceSize>(maxW) *
					static_cast<VkDeviceSize>(maxH) *
					static_cast<VkDeviceSize>(texture->GetChannels());
				copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copy.imageSubresource.baseArrayLayer = static_cast<uint32>(i);
				copy.imageSubresource.layerCount = 1;
				copy.imageExtent = m_imageExtent;
			}
			vkCmdCopyBufferToImage(
				buffer, m_buffer->Get(), m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				static_cast<uint32>(copyRegions.Count()), copyRegions.Data()
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
			barrierTexRead.subresourceRange.levelCount = texture->GetMipLevels();
			barrierTexRead.subresourceRange.layerCount = static_cast<uint32>(stbiTextures.Count());

			// Submit the pipeline command
			barrierTexInfo.pImageMemoryBarriers = &barrierTexRead;
			vkCmdPipelineBarrier2(buffer, &barrierTexInfo);
		});

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
	samplerInfo.maxLod = 1.f;

	if (result = vkCreateSampler(device->Logical(), &samplerInfo, nullptr, &m_sampler);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Sampler from image!", result);
	}

	// Destroy the texture and set up the descriptors
	for (StbiTexture& tex : stbiTextures)
	{
		stbi_image_free(tex.pixels);
	}

	m_textureDescriptors.sampler = m_sampler;
	m_textureDescriptors.imageView = m_imageView;
	m_textureDescriptors.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
}

void Texture::VulkanTexture::DestroyBuffer() const
{
	const GraphicsDevice* device = Vulkan::Device();

	vkDestroyImageView(device->Logical(), m_imageView, nullptr);
	vkDestroySampler(device->Logical(), m_sampler, nullptr);
	vmaDestroyImage(Vulkan::Allocator(), m_image, m_imageAllocation);
}