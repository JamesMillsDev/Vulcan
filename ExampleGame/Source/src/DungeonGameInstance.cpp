#include "DungeonGameInstance.h"

#include "FlyCamera.h"

#include "Gameplay/Actors/Transform.h"
#include "Gameplay/Actors/World.h"
#include "Gameplay/Actors/Components/Rendering/MeshComponent.h"

#include "Graphics/Rendering/Material.h"
#include "Graphics/Rendering/Mesh.h"
#include "Graphics/Rendering/Texture.h"
#include "Graphics/Vulkan/GraphicsPipeline.h"

#include "ImGui/imgui.h"

DungeonGameInstance::DungeonGameInstance() :
	m_meshActor{ nullptr }, m_camera{ nullptr }, m_material{ nullptr }, m_mesh{ nullptr }
{}

void DungeonGameInstance::Init()
{
	m_camera = new FlyCamera{ 45.f, .1f, 100.f };
	m_camera->location = vec3{ 0.f, 2.f, -10.f };

	m_mesh = Mesh::MakeFromAssimp("Meshes/SM_Soulspear.fbx");
	m_material = new Material{ "Shaders/pbr" };
	m_material->AddTexture(new Texture{ "Textures/T_Soulspear_B" });
	m_material->AddTexture(new Texture{ "Textures/T_Soulspear_N" });
	m_material->AddTexture(new Texture{ "Textures/T_Soulspear_ORM" });

	m_meshActor = GetWorld()->MakeActor<Actor>();
	m_meshActor->MakeComponent<MeshComponent>(m_mesh, m_material);
	m_material->color = Color{ 1.f, .5f, .31f, 1.f };
}

void DungeonGameInstance::Shutdown()
{
	delete m_camera;
	delete m_mesh;
	delete m_material;
}

void DungeonGameInstance::Tick()
{
	m_camera->Tick();
}

void DungeonGameInstance::Render()
{
	ImGui::ShowDemoWindow();
}