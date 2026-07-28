#include "Gameplay/Actors/Components/Rendering/MeshComponent.h"

#include "Gameplay/Actors/Actor.h"
#include "Gameplay/Actors/World.h"

#include "Graphics/Renderer.h"
#include "Graphics/Rendering/Material.h"

using namespace Vulcan;

MeshComponent::MeshComponent(Mesh* mesh, Material* material)
	: m_mesh{ mesh }, m_material{ material }
{
	
}

Material* MeshComponent::GetMaterial() const
{
	return m_material;
}

void MeshComponent::PreRender()
{
	Renderer::Instance()->UpdateBuffer(
		m_material, Owner()->GetObjectIndex()
	);
}

void MeshComponent::Render()
{
	Renderer::Instance()->Render(
		m_mesh, m_material, Owner()->GetObjectIndex(), Owner()->GetWorld()->GetLighting()
	);

	m_material->Dbg_ShowGui(); 
}
