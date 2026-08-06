#pragma once

#include "Debug.h"

#include "Maths/Alias.h"
#include "Maths/Color.h"

#include "Utility/Collections/TList.h"

namespace Vulcan
{
	class Actor;
	class HdrTexture;
	class LightComponent;
	class Material;
	class MemoryBuffer;
	class Mesh;
	class World;

	constexpr uint8 MAX_LIGHT_COUNT = 10;

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
		friend class Renderer;

	private:
		SceneLightingUniform m_sceneLighting;
		TList<LightComponent*> m_lights;

		MemoryBuffer* m_sceneLightingBuffer;
		TList<MemoryBuffer*> m_lightBuffers;

		Actor* m_skyboxActor;
		Mesh* m_skyboxMesh;
		Material* m_skyboxMaterial;
		HdrTexture* m_skyboxTexture;

	public:
		explicit Lighting(World* world);
		~Lighting();

	public:
		void UpdateBuffers();

		DEFINE_DEBUG_FUNCTION(ShowGui)

		void AddLight(LightComponent* light);
		void RemoveLight(LightComponent* light);

	};
}