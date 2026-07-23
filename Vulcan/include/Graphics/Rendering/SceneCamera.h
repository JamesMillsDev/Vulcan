#pragma once
#include "Camera.h"

class Transform;

class SceneCamera : public Camera
{
private:
	Transform* m_transform;

public:
	SceneCamera(float fov, float near, float far);

public:
	void SetTransform(Transform* transform);
	void GetPvm(ProjectionViewModelUniform& pvm) const override;

};
