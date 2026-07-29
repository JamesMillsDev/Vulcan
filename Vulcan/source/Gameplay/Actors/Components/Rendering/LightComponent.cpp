#include "Gameplay/Actors/Components/Rendering/LightComponent.h"

#include "Gameplay/Actors/Actor.h"
#include "Gameplay/Actors/World.h"
#include "Graphics/Rendering/Lighting.h"

using namespace Vulcan;

LightComponent::LightComponent() :
	type{ EType::Directional }, intensity{ 1.f }, color{ Color::WHITE }, constant{ 1.f },
	linear{ .09f }, quadratic{ .32f }, cutOff{ 0 }, outerCutOff{ 0 }
{}

void LightComponent::BeginPlay()
{
	Owner()->GetWorld()->GetLighting()->AddLight(this);
}

void LightComponent::EndPlay()
{
	Owner()->GetWorld()->GetLighting()->RemoveLight(this);
}