#pragma once

#include "Camera.h"

namespace Fulk
{
	class Transform;

	class SceneCamera : public Camera
	{
	private:
		Transform* m_transform;

	public:
		SceneCamera(float fov, float near, float far);

	public:
		void SetTransform(Transform* transform);
		void GetPvm(GlobalsUniform& pvm) const override;

	};
}