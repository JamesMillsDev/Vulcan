#pragma once

#include <string>

#include "ITexture.h"

using std::string;

namespace Tempest
{
	class Texture : public ITexture<uint8>
	{
		friend class Material;
		friend class Renderer;

	public:
		static Texture* LoadFromFile(const string& fileName, const TextureLoadInfo& loadInfo = {});
		static Texture* LoadCubeMapFromFile(const string& baseFileName, const TList<string>& fileNames, const TextureLoadInfo& loadInfo = {});

	};
}