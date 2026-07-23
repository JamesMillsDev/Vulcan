#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

using glm::mat4;
using glm::vec3;

namespace Vulcan
{
	struct GlobalsUniform
	{
		mat4 proj;
		mat4 view;
		vec3 cameraLocation;

		float exposure;
		float gamma;
		float prefilteredCubeMipLevels;
		float scaleIBLAmbient;
	};

	struct TransformUniform
	{
		mat4* values = nullptr;
	};

}