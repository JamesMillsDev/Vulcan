#include "Gameplay/GameInstance.h"

#include "Gameplay/Actors/World.h"

GameInstance::GameInstance()
// TODO: Support levels instead of worlds
	: m_world{ new World }
{
	
}

GameInstance::~GameInstance() = default;

World* GameInstance::GetWorld() const
{
	return m_world;
}
