#include "ExampleGameInstance.h"

#include "FlyCamera.h"
#include "GameTime.h"
#include "Gameplay/Actors/Transform.h"
#include "Gameplay/Actors/World.h"
#include "Gameplay/Actors/Components/Rendering/LightComponent.h"
#include "Gameplay/Actors/Components/Rendering/MeshComponent.h"
#include "Graphics/Rendering/Lighting.h"
#include "Graphics/Rendering/Material.h"
#include "Graphics/Rendering/Mesh.h"
#include "Graphics/Rendering/Texture.h"
#include "ImGui/imgui.h"
#include "Maths/Maths.h"

using namespace Vulcan;

constexpr int32 LIGHT_COUNT = 1;
namespace
{
	TArray<Actor*, LIGHT_COUNT> lights;
	TArray<Material*, LIGHT_COUNT> lightMaterials;
	mat4 rotationMatrix(1.f);
}
constexpr float ANGLE_STEP = 360.f / LIGHT_COUNT;

ExampleGameInstance::ExampleGameInstance() :
	m_camera{ nullptr }, m_material{ nullptr }, m_mesh{ nullptr }, m_cubeMesh{ nullptr }
{}

void ExampleGameInstance::Init()
{
	m_camera = new FlyCamera{ 45.f, .1f, 100.f };
	m_camera->location = vec3{ 0.f, 2.f, 10.f };
	m_camera->yaw = 180.f;

	m_mesh = Mesh::MakeFromAssimp("Meshes/shaderBall.fbx");
	m_material = new Material{ "Shaders/pbr" };
	m_material->SetTexture(BASE_COLOR_MAP_NAME, Texture::LoadFromFile("Textures/T_RebarConcrete_BC"));
	m_material->SetTexture(NORMAL_MAP_NAME, Texture::LoadFromFile("Textures/T_RebarConcrete_N", { .sRgb = false, .normalMap = true, .invertGreen = true }));
	m_material->SetTexture(ORM_MAP_NAME, Texture::LoadFromFile("Textures/T_RebarConcrete_ORM", { .sRgb = false }));

	Actor* meshActor = GetWorld()->MakeActor<Actor>();
	meshActor->MakeComponent<MeshComponent>(m_mesh, m_material);
	m_material->color = Color{ 1.f, 1.f, 1.f, 1.f };

	m_cubeMesh = Mesh::MakeCube();

	float offset = 0.f;
	for (uint8 i = 0; i < LIGHT_COUNT; ++i)
	{
		lightMaterials[i] = new Material{ "Shaders/unlit" };
		lightMaterials[i]->color = Color::WHITE;

		Actor* lightActor = GetWorld()->MakeActor<Actor>();
		LightComponent* light = lightActor->MakeComponent<LightComponent>();
		lightActor->MakeComponent<MeshComponent>(m_cubeMesh, lightMaterials[i]);
		lightActor->GetTransform()->SetScale(vec3{ .25f });

		light->type = LightComponent::EType::Point;
		light->color = lightMaterials[i]->color;

		lights[i] = lightActor;
		offset += ANGLE_STEP;
	}
}

void ExampleGameInstance::Shutdown()
{
	for (Material* material : lightMaterials)
	{
		delete material;
	}

	delete m_camera;
	delete m_cubeMesh;
	delete m_mesh;
	delete m_material;
}

void ExampleGameInstance::Tick()
{
	m_camera->Tick();

	float offset = 0;
	for (Actor* light : lights)
	{
		rotationMatrix = glm::rotate(mat4{ 1.f }, GameTime::Time(), vec3{ 0.f, 1.f, 0.f });
		rotationMatrix = glm::rotate(rotationMatrix, Maths::Radians(offset), vec3{ 0.f, 1.f, 0.f });

		offset += ANGLE_STEP;
		vec3 forward = rotationMatrix * vec4{ 0.f, 0.f, 1.f, 0.f };

		light->GetTransform()->SetLocation(forward * 15.f + vec3{ 0.f, 2.5f, 0.f });
	}
}

void ExampleGameInstance::Render()
{}