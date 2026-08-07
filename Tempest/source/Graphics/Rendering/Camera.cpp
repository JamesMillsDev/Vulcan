#include "Graphics/Rendering/Camera.h"

#include <bitset>
#include <glm/ext/matrix_clip_space.hpp>

#include "Application.h"
#include "Window.h"
#include "Graphics/Renderer.h"
#include "Maths/Maths.h"
#include "Utility/Collections/HashImpls.h"

using std::hash;

using namespace Tempest;

Camera::Camera(const float fovY, const float nearPlane, const float farPlane)
	: fovY{ fovY }, nearPlane{ nearPlane }, farPlane{ farPlane }, m_window{ Application::GetWindow() },
	m_isCurrent{ false }
{
	Renderer::SetCurrent(this);
}

void Camera::GetPvm(GlobalsUniform& pvm) const
{
	pvm.proj = glm::perspective(Maths::Radians(fovY), m_window->Aspect(), nearPlane, farPlane);
}

bool Camera::IsCurrent() const
{
	return m_isCurrent;
}

uint64 Camera::GetHashCode() const
{
	return HashValue(fovY) + HashValue(nearPlane) + HashValue(farPlane);
}
