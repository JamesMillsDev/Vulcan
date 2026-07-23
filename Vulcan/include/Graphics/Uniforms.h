#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

using glm::mat4;
using glm::vec3;

struct ProjectionViewModelUniform
{
	mat4 proj;
	mat4 view;
	mat4 model;
	vec3 cameraLocation;
};