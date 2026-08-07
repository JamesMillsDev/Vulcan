#include "Graphics/Rendering/Texture.h"

#include <format>
#include <stdexcept>

#include "Resources.h"
#include "Utility/Console.h"

using namespace Tempest;

const TArray TEXTURE_EXTENSIONS =
{
	".png",
	".tga",
	".jpg",
	".bmp"
};

using std::runtime_error;

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

	texture->m_vulkanTexture = new VulkanTexture{ textureData, texture, false };

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

	texture->m_vulkanTexture = new VulkanTexture{ textureData, texture, false };

	return texture;
}