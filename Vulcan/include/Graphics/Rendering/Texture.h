#pragma once

#include <ktx.h>
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

	class Texture : public Object
	{
		friend class Material;
		friend class Renderer;

	public:
		class VulkanTexture
		{
			friend class Texture;

		private:
			VkImage m_image;
			VmaAllocation m_imageAllocation;
			VkImageView m_imageView;
			VkSampler m_sampler;

			VkExtent3D m_imageExtent;
			VkFormat m_imageFormat;

			VkDescriptorImageInfo m_textureDescriptors;

			ktxTexture2* m_texture;
			MemoryBuffer* m_buffer;

		private:
			VulkanTexture(const uint8* pixels, uint64 numPixels, Texture* texture);

		private:
			void CreateBuffer(const uint8* pixels, uint64 numPixels, Texture* texture);
			void DestroyBuffer() const;

			void TransitionImage() const;

		};

	private:
		static int32 m_nextId;
		static queue<int32> m_freeIds;

	public:
		static Texture* LoadFromFile(const string& fileName);

	private:
		VulkanTexture* m_vulkanTexture;
		int32 m_id;

		TList<uint8> m_pixels;

	public:
		Texture();
		~Texture() override;

	public:
		[[nodiscard]] uint64 GetHashCode() const override;

		[[nodiscard]] const VkDescriptorImageInfo& GetDescriptors() const;
		[[nodiscard]] int32 GetId() const;

		DEFINE_GETTER_SETTER(Pixels, const TList<uint8>&, pixels, m_pixels)
		DEFINE_GETTER_SETTER_VARIABLE(Width, uint32, width)
		DEFINE_GETTER_SETTER_VARIABLE(Height, uint32, height)
		DEFINE_GETTER_SETTER_VARIABLE(IsNormal, bool, isNormal)
		DEFINE_GETTER_SETTER_VARIABLE(IsSrgb, bool, isSrgb)
		DEFINE_GETTER_SETTER_VARIABLE(Format, VkFormat, format)

		void Apply();

	};
}