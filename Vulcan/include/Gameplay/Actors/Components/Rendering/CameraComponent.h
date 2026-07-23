#pragma once

#include "Gameplay/Actors/Components/IComponent.h"

class SceneCamera;
class Window;

class CameraComponent : public IComponent
{
	friend class Renderer;

public:
	SceneCamera* camera;

private:
	Window* m_window;

public:
	CameraComponent(float fovY, float nearPlane, float farPlane);
	~CameraComponent() override;

public:
	void BeginPlay() override;

};
