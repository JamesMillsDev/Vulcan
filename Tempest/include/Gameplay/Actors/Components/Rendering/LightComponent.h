#pragma once

#include "Gameplay/Actors/Components/IComponent.h"
#include "Maths/Alias.h"
#include "Maths/Color.h"

namespace Tempest
{
	class LightComponent : public IComponent
	{
	public:
		enum class EType : uint8
		{
			Directional,
			Point,
			Spot
		};

	public:
		EType type;
		float intensity;
		Color color;

		float constant;
		float linear;
		float quadratic;
		float cutOff;
		float outerCutOff;

	public:
		LightComponent();

	public:
		void BeginPlay() override;
		void EndPlay() override;

	};
}