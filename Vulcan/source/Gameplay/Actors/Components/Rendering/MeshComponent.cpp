#include "Gameplay/Actors/Components/Rendering/MeshComponent.h"

#include "Gameplay/Actors/Actor.h"
#include "Gameplay/Actors/Transform.h"

#include "Graphics/Renderer.h"

MeshComponent::MeshComponent(Mesh* mesh, Material* material)
	: m_mesh{ mesh }, m_material{ material }
{
	
}

void MeshComponent::Render()
{
	Renderer::Instance()->Render(m_mesh, m_material, Owner()->GetTransform()->LocalToWorld());
}