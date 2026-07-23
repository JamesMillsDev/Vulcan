#include "Graphics/Rendering/SceneCamera.h"

#include "Gameplay/Actors/Transform.h"

SceneCamera::SceneCamera(const float fov, const float near, const float far)
	: Camera{ fov, near, far }, m_transform{ nullptr }
{
	
}

void SceneCamera::SetTransform(Transform* transform)
{
	m_transform = transform;
}

void SceneCamera::GetPvm(ProjectionViewModelUniform& pvm) const
{
	Camera::GetPvm(pvm);
	pvm.view = m_transform->LocalToWorld();
	pvm.cameraLocation = m_transform->location;
}