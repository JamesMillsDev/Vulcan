#pragma once

#include <Gameplay/GameInstance.h>

namespace Fulk
{
	class Mesh;
	class Material;
}

class FlyCamera;

using Fulk::GameInstance;
using Fulk::Mesh;
using Fulk::Material;

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
