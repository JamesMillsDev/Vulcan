#pragma once

#include <Gameplay/GameInstance.h>

namespace Tempest
{
	class Mesh;
	class Material;
}

class FlyCamera;

using Tempest::GameInstance;
using Tempest::Mesh;
using Tempest::Material;

class ExampleGameInstance final : public GameInstance
{
private:
	FlyCamera* m_camera;

	Material* m_material;
	Mesh* m_mesh;
	Mesh* m_cubeMesh;

public:
	ExampleGameInstance();

public:
	void Init() override;
	void Shutdown() override;
	void Tick() override;
	void Render() override;

};
