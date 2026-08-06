#include "Graphics/Rendering/HdrTexture.h"

#include "Utility/Console.h"

using namespace Vulcan;

HdrTexture* HdrTexture::LoadFromFile(const string& fileName)
{
	ResourceData resourceData = {};

	try
	{
		resourceData = Resources::Find(fileName + ".hdr");
	}
	catch ([[maybe_unused]] runtime_error& e)
	{
		Console::Exception("Texture for filename: '" + fileName + "' not found!");
		return nullptr;
	}

	HdrTexture* texture = new HdrTexture;
	TList<uint8> data;
	data.SetData(resourceData.data, resourceData.length);

	TList<TList<uint8>> textureData;
	textureData.Add(data);

	texture->SetTextureInfo({ .sRgb = false });

	texture->m_vulkanTexture = new VulkanTexture{ textureData, texture, true };

	return texture;
}
