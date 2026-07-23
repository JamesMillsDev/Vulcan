#pragma once

#include "Gameplay/Actors/Components/IComponent.h"
#include "Maths/Alias.h"
#include "Maths/Color.h"

constexpr int MAX_LIGHT_COUNT = 8;

struct LightUniform
{
	vec3 location;
	vec3 direction;
	Color color;
	int32 type;
};

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

public:
	LightComponent();

};