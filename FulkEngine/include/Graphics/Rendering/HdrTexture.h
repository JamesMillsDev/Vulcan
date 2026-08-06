#pragma once

#include <string>
#include "ITexture.h"

using std::string;

namespace Fulk
{
	class HdrTexture : public ITexture<float>
	{
		friend class Material;
		friend class Renderer;

	public:
		static HdrTexture* LoadFromFile(const string& fileName, bool flipV = true);

	};
}