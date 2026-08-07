#include "FlyCamera.h"

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>

#include "GameTime.h"
#include "SimpleInput.h"
#include "Window.h"
#include "Maths/Maths.h"

using Tempest::Maths;
using Tempest::SimpleInput;
using Tempest::EInputCodes;
using Tempest::GameTime;

FlyCamera::FlyCamera(const float fov, const float near, const float far)
	: Camera{ fov, near, far }, location{ 0.f, 0.f, 0.f }, yaw{ 0 }, pitch{ 0 },
	m_turnSpeed{ Maths::Radians(180.f) }, m_moveSpeed{ 5.f }, m_lastMouse{ 0.f, 0.f }
{}

void FlyCamera::GetPvm(GlobalsUniform& pvm) const
{
	Camera::GetPvm(pvm);

	const float yawR = Maths::Radians(yaw);
	const float pitchR = Maths::Radians(pitch);
	const vec3 forward
	{
		Maths::Cos(pitchR) * Maths::Sin(yawR),
		Maths::Sin(pitchR),
		Maths::Cos(pitchR) * Maths::Cos(yawR)
	};

	pvm.view = glm::lookAt(location, location + forward, vec3{ 0.f, 1.f, 0.f });
}

void FlyCamera::Tick()
{
	SimpleInput* input = SimpleInput::Instance();

	if (input->WasMouseButtonReleased(EInputCodes::MouseButtonRight))
	{
		glfwSetInputMode(m_window->GlfwHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	if (!input->IsMouseButtonDown(EInputCodes::MouseButtonRight))
	{
		return;
	}

	// Get the mouse coordinates
	const float mx = input->GetMouseX();
	const float my = input->GetMouseY();

	if (input->WasMouseButtonPressed(EInputCodes::MouseButtonRight))
	{
		m_lastMouse = vec2{ mx, my };
		glfwSetInputMode(m_window->GlfwHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	const float yawR = Maths::Radians(yaw);
	const float pitchR = Maths::Radians(pitch);
	const vec3 forward
	{
		Maths::Cos(pitchR) * Maths::Sin(yawR),
		Maths::Sin(pitchR),
		Maths::Cos(pitchR) * Maths::Cos(yawR)
	};
	const vec3 right{ Maths::Cos(yawR), 0.f, -Maths::Sin(yawR) };
	constexpr vec3 up{ 0.f, 1.f, 0.f };

	// We will use WASD to move and the Q & E to go up and down
	if (input->IsKeyDown(EInputCodes::KeyW))
	{
		location += forward * GameTime::DeltaTime() * m_moveSpeed;
	}

	if (input->IsKeyDown(EInputCodes::KeyS))
	{
		location -= forward * GameTime::DeltaTime() * m_moveSpeed;
	}

	if (input->IsKeyDown(EInputCodes::KeyA))
	{
		location += right * GameTime::DeltaTime() * m_moveSpeed;
	}

	if (input->IsKeyDown(EInputCodes::KeyD))
	{
		location -= right * GameTime::DeltaTime() * m_moveSpeed;
	}

	if (input->IsKeyDown(EInputCodes::KeyQ))
	{
		location -= up * GameTime::DeltaTime() * m_moveSpeed;
	}

	if (input->IsKeyDown(EInputCodes::KeyE))
	{
		location += up * GameTime::DeltaTime() * m_moveSpeed;
	}

	// If the right button is held down, increment theta and phi (rotate)
	if (input->IsMouseButtonDown(EInputCodes::MouseButtonRight))
	{
		yaw -= m_turnSpeed * (mx - m_lastMouse.x) * GameTime::DeltaTime();
		pitch += m_turnSpeed * (my - m_lastMouse.y) * GameTime::DeltaTime();
	}

	m_lastMouse = vec2(mx, my);
}