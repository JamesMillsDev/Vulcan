#pragma once

#include <Gameplay/GameInstance.h>

namespace Vulcan
{
	class Mesh;
	class Material;
}

class FlyCamera;

using Vulcan::GameInstance;
using Vulcan::Mesh;
using Vulcan::Material;

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
