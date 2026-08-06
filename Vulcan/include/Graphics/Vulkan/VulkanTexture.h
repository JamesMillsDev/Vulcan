#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <stb_image.h>

#include "CommandManager.h"
#include "GraphicsDevice.h"
#include "MemoryBuffer.h"
#include "Vulkan.h"
#include "Maths/Alias.h"

#define DEFINE_GETTER_SETTER_VARIABLE(NAME, TYPE, VAR_NAME) \
	private: \
		TYPE m_##VAR_NAME; \
	public: \
	void Set##NAME(TYPE VAR_NAME) { m_##VAR_NAME = VAR_NAME; } \
	TYPE Get##NAME() const { return m_##VAR_NAME; } \

#define DEFINE_GETTER_SETTER(NAME, TYPE, VAR_NAME, VAR) \
	void Set##NAME(TYPE VAR_NAME) { VAR = VAR_NAME; } \
	TYPE Get##NAME() const { return VAR; } \

namespace Vulcan
{
	class HdrTexture;
	template<typename T>
	class ITexture;
	class MemoryBuffer;
	class Texture;

	enum EVulkanFormatMask : uint8
	{
		Greyscale = 1 << 0,
		Rgb = 1 << 1,
		Alpha = 1 << 2,
		SRgb = 1 << 3,
		Hdr = 1 << 4,
	};

	struct TextureLoadInfo
	{
		bool sRgb = true;
		bool normalMap = false;
		bool invertGreen = false;
		bool greyscale = false;
		uint32 channels = 4;
		uint32 mipLevels = 1;
	};

	template<typename T>
	class VulkanTexture
	{
		friend HdrTexture;
		friend Texture;
		friend ITexture;

	private:
		static VkFormat GetVulkanFormat(const ITexture<T>* texture, bool hdr);

	private:
		VkImage m_image;
		VmaAllocation m_imageAllocation;
		VkImageView m_imageView;
		VkSampler m_sampler;

		VkExtent3D m_imageExtent;
		VkFormat m_imageFormat;

		VkDescriptorImageInfo m_textureDescriptors;

		MemoryBuffer* m_buffer;

	private:
		VulkanTexture(const TList<TList<uint8>>& textureBinary, ITexture<T>* texture, bool hdr, bool flipV = false);

	private:
		void CreateBuffer(const TList<TList<uint8>>& textureBinary, ITexture<T>* texture, bool hdr, bool flipV);
		void DestroyBuffer() const;

	};

	struct StbiTexture
	{
		int32 w;
		int32 h;
		int32 channels;
		bool hdr = false;
		uint8* pixels = nullptr;
		float* hdrPixels = nullptr;
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
		{.key = static_cast<uint8>(Hdr | Rgb), .value = VK_FORMAT_R32G32B32_SFLOAT },
		{.key = static_cast<uint8>(Hdr | Rgb | Alpha), .value = VK_FORMAT_R32G32B32A32_SFLOAT },
	};

	template<typename T>
	VkFormat VulkanTexture<T>::GetVulkanFormat(const ITexture<T>* texture, bool hdr)
	{
		uint8 mask = texture->GetIsGreyscale() ? Greyscale : Rgb;

		if (texture->GetIsSrgb())
		{
			mask |= SRgb;
		}
		else if (hdr)
		{
			mask |= Hdr;
		}

		if (texture->GetChannels() == 4)
		{
			mask |= Alpha;
		}

		return FORMATS[mask];
	}

	template<typename T>
	VulkanTexture<T>::VulkanTexture(const TList<TList<uint8>>& textureBinary, ITexture<T>* texture, bool hdr, bool flipV)
		: m_image{ VK_NULL_HANDLE }, m_imageAllocation{ VK_NULL_HANDLE },
		m_imageView{ VK_NULL_HANDLE }, m_sampler{ VK_NULL_HANDLE }, m_imageExtent{ },
		m_imageFormat{  }, m_textureDescriptors{ }, m_buffer{ VK_NULL_HANDLE }
	{
		CreateBuffer(textureBinary, texture, hdr, flipV);
	}

	template<typename T>
	void VulkanTexture<T>::CreateBuffer(const TList<TList<uint8>>& textureBinary, ITexture<T>* texture, bool hdr, bool flipV)
	{
		TList<StbiTexture> stbiTextures;
		uint32 maxW = 0, maxH = 0;

		if (flipV)
		{
			stbi_set_flip_vertically_on_load(true);
		}

		for (TList<uint8>& textureData : textureBinary)
		{
			StbiTexture stbiTexture;
			if (!hdr)
			{
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
			}
			else
			{
				stbiTexture.hdr = true;
				stbiTexture.hdrPixels = stbi_loadf_from_memory(
					textureData.Data(), static_cast<int32>(textureData.Count()), &stbiTexture.w, &stbiTexture.h,
					&stbiTexture.channels, static_cast<int32>(texture->GetChannels())
				);

				if (stbiTexture.hdrPixels == nullptr)
				{
					throw runtime_error("Failed to load HDR texture!");
				}
			}

			maxW = std::max(static_cast<uint32>(stbiTexture.w), maxW);
			maxH = std::max(static_cast<uint32>(stbiTexture.h), maxH);
			stbiTextures.Add(stbiTexture);
		}

		m_imageFormat = GetVulkanFormat(texture, hdr);
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

		uint64 singleTextureLength = static_cast<uint64>(maxW * maxH) * texture->GetChannels() * (hdr ? sizeof(float) : sizeof(uint8));
		// Generate the buffer and transition
		m_buffer = new MemoryBuffer
		{
			singleTextureLength * stbiTextures.Count(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT
		};

		for (int64 i = 0; i < stbiTextures.Count(); ++i)
		{
			m_buffer->Fill(hdr ? reinterpret_cast<uint8*>(stbiTextures[i].hdrPixels) : stbiTextures[i].pixels, singleTextureLength, i * singleTextureLength);
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
			stbi_image_free(!hdr ? static_cast<void*>(tex.pixels) : static_cast<void*>(tex.hdrPixels));
		}

		m_textureDescriptors.sampler = m_sampler;
		m_textureDescriptors.imageView = m_imageView;
		m_textureDescriptors.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

		stbi_set_flip_vertically_on_load(false);
	}

	template<typename T>
	void VulkanTexture<T>::DestroyBuffer() const
	{
		const GraphicsDevice* device = Vulkan::Device();

		vkDestroyImageView(device->Logical(), m_imageView, nullptr);
		vkDestroySampler(device->Logical(), m_sampler, nullptr);
		vmaDestroyImage(Vulkan::Allocator(), m_image, m_imageAllocation);
	}
}