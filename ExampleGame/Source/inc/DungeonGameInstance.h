#pragma once

#include <Gameplay/GameInstance.h>

namespace Vulcan
{
	class Actor;
	class Mesh;
	class Material;
}

class FlyCamera;

using Vulcan::Actor;
using Vulcan::GameInstance;
using Vulcan::Mesh;
using Vulcan::Material;

class DungeonGameInstance final : public GameInstance
{
private:
	Actor* m_meshActor;
	FlyCamera* m_camera;

	Material* m_material;
	Mesh* m_mesh;

public:
	DungeonGameInstance();

public:
	void Init() override;
	void Shutdown() override;
	void Tick() override;
	void Render() override;

};
