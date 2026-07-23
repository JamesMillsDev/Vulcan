#pragma once

#include "Object.h"
#include "Graphics/Uniforms.h"

class Window;

class Camera : public Object
{
	friend class Renderer;

public:
	float fovY;
	float nearPlane;
	float farPlane;

protected:
	Window* m_window;

private:
	bool m_isCurrent;

public:
	Camera(float fovY, float nearPlane, float farPlane);

public:
	virtual void GetPvm(ProjectionViewModelUniform& pvm) const;

	[[nodiscard]] bool IsCurrent() const;

	[[nodiscard]] uint64 GetHashCode() const override;

};
