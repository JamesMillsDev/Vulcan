#pragma once

#include "Debug.h"

#include "Maths/Alias.h"
#include "Maths/Color.h"

#include "Utility/Collections/TList.h"

namespace Vulcan
{
	class LightComponent;

	constexpr uint8 MAX_LIGHT_COUNT = 16;

	struct LightUniform
	{
		vec4 location;
		vec4 direction;
		Color color;
		float intensity;

		float constant;
		float linear;
		float quadratic;
		float cutOff;
		float outerCutOff;

		int32 type;
		int32 enabled;
	};

	struct SceneLightingUniform
	{
		Color ambientColor;
		float ambientStrength;
	};

	class Lighting
	{
	private:
		SceneLightingUniform m_sceneLighting;
		TList<LightComponent*> m_lights;

	public:
		Lighting();

	public:
		void UpdateBuffers();

		DEFINE_DEBUG_FUNCTION(ShowGui)

			void AddLight(LightComponent* light);
		void RemoveLight(LightComponent* light);

	};
}