#pragma once

#include <string>
#include <vulkan/vulkan.h>

#include "Object.h"
#include "Resources.h"
#include "Graphics/Vulkan/VulkanTexture.h"
#include "Maths/Alias.h"
#include "Utility/Collections/HashImpls.h"
#include "Utility/Collections/TList.h"

using std::string;

namespace Tempest
{
	template<typename T>
	class ITexture : public Object
	{
		friend class Material;
		friend class Renderer;

	protected:
		VulkanTexture<T>* m_vulkanTexture;
		int32 m_id;

		TList<TList<T>> m_pixels;

	public:
		ITexture();
		~ITexture() override;

	public:
		[[nodiscard]] uint64 GetHashCode() const override;

		[[nodiscard]] const VkDescriptorImageInfo& GetDescriptors() const;
		[[nodiscard]] int32 GetId() const;

		DEFINE_GETTER_SETTER(Pixels, const TList<TList<T>>&, pixels, m_pixels)
		DEFINE_GETTER_SETTER_VARIABLE(Width, uint32, width)
		DEFINE_GETTER_SETTER_VARIABLE(Height, uint32, height)
		DEFINE_GETTER_SETTER_VARIABLE(Channels, uint32, channels)
		DEFINE_GETTER_SETTER_VARIABLE(IsNormal, bool, isNormal)
		DEFINE_GETTER_SETTER_VARIABLE(IsSrgb, bool, isSrgb)
		DEFINE_GETTER_SETTER_VARIABLE(IsCubeMap, bool, isCubeMap)
		DEFINE_GETTER_SETTER_VARIABLE(IsGreyscale, bool, isGreyscale)
		DEFINE_GETTER_SETTER_VARIABLE(MipLevels, uint32, mipLevels)
		DEFINE_GETTER_SETTER_VARIABLE(Format, VkFormat, format)
		DEFINE_GETTER_SETTER_VARIABLE(BitsPerChannel, uint32, bitsPerChannel)
		DEFINE_GETTER_SETTER_VARIABLE(GreenChannelFlipped, bool, greenChannelFlipped)

	protected:
		void SetTextureInfo(const TextureLoadInfo& info);

	};

	template<typename T>
	ITexture<T>::ITexture()
		: m_vulkanTexture{ nullptr }, m_id{ Resources::RequestNewTextureId() }, m_width{ 0 }, m_height{ 0 }, m_channels{ 4 },
		m_isNormal{ false }, m_isSrgb{ false }, m_isCubeMap{ false }, m_isGreyscale{ false }, m_mipLevels{ 1 },
		m_format{}, m_bitsPerChannel{ 8 }, m_greenChannelFlipped{ false }
	{}

	template<typename T>
	ITexture<T>::~ITexture()
	{
		Resources::ReturnTextureId(m_id);

		if (m_vulkanTexture != nullptr)
		{
			m_vulkanTexture->DestroyBuffer();
			delete m_vulkanTexture;
		}
	}

	template<typename T>
	uint64 ITexture<T>::GetHashCode() const
	{
		return HashAll(m_id, m_width, m_height, m_channels, m_isNormal, m_isSrgb, m_isCubeMap, m_isGreyscale, m_mipLevels, m_bitsPerChannel);
	}

	template<typename T>
	const VkDescriptorImageInfo& ITexture<T>::GetDescriptors() const
	{
		return m_vulkanTexture->m_textureDescriptors;
	}

	template<typename T>
	int32 ITexture<T>::GetId() const
	{
		return m_id;
	}

	template<typename T>
	void ITexture<T>::SetTextureInfo(const TextureLoadInfo& info)
	{
		m_isSrgb = info.sRgb;
		m_isNormal = info.normalMap;
		m_greenChannelFlipped = info.invertGreen;
		m_channels = info.channels;
		m_isGreyscale = info.greyscale;
		m_mipLevels = info.mipLevels;
	}
}