#include "Gameplay/Actors/Components/Rendering/CameraComponent.h"

#include "Application.h"
#include "Gameplay/Actors/Actor.h"
#include "Graphics/Renderer.h"
#include "Graphics/Rendering/Camera.h"
#include "Graphics/Rendering/SceneCamera.h"

CameraComponent::CameraComponent(const float fovY, const float nearPlane, const float farPlane)
	: camera{ new SceneCamera{ fovY, nearPlane, farPlane } }, m_window{ Application::GetWindow() }
{
	Renderer::SetCurrent(camera);
}

CameraComponent::~CameraComponent()
{
	delete camera;
	camera = nullptr;
}

void CameraComponent::BeginPlay()
{
	camera->SetTransform(Owner()->GetTransform());
}
