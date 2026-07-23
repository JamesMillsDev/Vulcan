#pragma once

#include <Gameplay/GameInstance.h>

class Actor;
class FlyCamera;
class Mesh;
class Material;

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
