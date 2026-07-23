#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Graphics/Rendering/Camera.h"

using glm::vec2;
using glm::vec3;
using glm::mat4;

class FlyCamera : public Camera
{
public:
	vec3 location;

private:
	float m_yaw;
	float m_pitch;

	float m_turnSpeed;
	float m_moveSpeed;

	vec2 m_lastMouse;

public:
	FlyCamera(float fov, float near, float far);

public:
	void GetPvm(ProjectionViewModelUniform& pvm) const override;
	void Tick();

};