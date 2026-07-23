#include "Gameplay/Actors/Components/Rendering/LightComponent.h"

using namespace Vulcan;

LightComponent::LightComponent()
	: type{ EType::Directional }, intensity{ 1.f }, color{ Color::WHITE }
{

}