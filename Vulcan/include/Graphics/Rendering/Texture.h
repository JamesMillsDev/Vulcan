#pragma once

#include <queue>
#include <string>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Object.h"
#include "Maths/Alias.h"

#include "Utility/Collections/TList.h"

using std::queue;
using std::string;

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
	class MemoryBuffer;

	struct TextureLoadInfo
	{
		bool sRgb = true;
		bool normalMap = false;
		bool invertGreen = false;
		bool greyscale = false;
		bool hdr = false;
		uint32 channels = 4;
		uint32 mipLevels = 1;
	};

	enum EVulkanFormatMask : uint8
	{
		Greyscale = 1 << 0,
		Rgb = 1 << 1,
		Alpha = 1 << 2,
		SRgb = 1 << 3,
		Hdr = 1 << 4,
	};

	class Texture : public Object
	{
		friend class Material;
		friend class Renderer;

	public:
		class VulkanTexture
		{
			friend class Texture;

		private:
			static VkFormat GetVulkanFormat(const Texture* texture);

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
			VulkanTexture(const TList<TList<uint8>>& textureBinary, Texture* texture);

		private:
			void CreateBuffer(const TList<TList<uint8>>& textureBinary, Texture* texture);
			void DestroyBuffer() const;

		};

	private:
		static int32 m_nextId;
		static queue<int32> m_freeIds;

	public:
		static Texture* LoadFromFile(const string& fileName, const TextureLoadInfo& loadInfo = {});
		static Texture* LoadCubeMapFromFile(const string& baseFileName, const TList<string>& fileNames, const TextureLoadInfo& loadInfo = {});

	private:
		VulkanTexture* m_vulkanTexture;
		int32 m_id;

		TList<TList<uint8>> m_pixels;

	public:
		Texture();
		~Texture() override;

	public:
		[[nodiscard]] uint64 GetHashCode() const override;

		[[nodiscard]] const VkDescriptorImageInfo& GetDescriptors() const;
		[[nodiscard]] int32 GetId() const;

		DEFINE_GETTER_SETTER(Pixels, const TList<TList<uint8>>&, pixels, m_pixels)
		DEFINE_GETTER_SETTER_VARIABLE(Width, uint32, width)
		DEFINE_GETTER_SETTER_VARIABLE(Height, uint32, height)
		DEFINE_GETTER_SETTER_VARIABLE(Channels, uint32, channels)
		DEFINE_GETTER_SETTER_VARIABLE(IsNormal, bool, isNormal)
		DEFINE_GETTER_SETTER_VARIABLE(IsSrgb, bool, isSrgb)
		DEFINE_GETTER_SETTER_VARIABLE(IsCubeMap, bool, isCubeMap)
		DEFINE_GETTER_SETTER_VARIABLE(IsGreyscale, bool, isGreyscale)
		DEFINE_GETTER_SETTER_VARIABLE(IsHdr, bool, isHdr)
		DEFINE_GETTER_SETTER_VARIABLE(MipLevels, uint32, mipLevels)
		DEFINE_GETTER_SETTER_VARIABLE(Format, VkFormat, format)
		DEFINE_GETTER_SETTER_VARIABLE(BitsPerChannel, uint32, bitsPerChannel)
		DEFINE_GETTER_SETTER_VARIABLE(GreenChannelFlipped, bool, greenChannelFlipped)

	private:
		void SetTextureInfo(const TextureLoadInfo& info);

	};
}