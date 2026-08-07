#include "Gameplay/Actors/Components/Rendering/MeshComponent.h"

#include "Gameplay/Actors/Actor.h"
#include "Gameplay/Actors/Transform.h"
#include "Gameplay/Actors/World.h"

#include "Graphics/Renderer.h"
#include "Graphics/Rendering/Material.h"

using namespace Tempest;

MeshComponent::MeshComponent(Mesh* mesh, Material* material)
	: MeshComponent{ mesh, TList{ material } }
{
	
}

MeshComponent::MeshComponent(Mesh* mesh, TList<Material*> materials)
	: m_mesh{ mesh }, m_materials{ std::move(materials) }
{
	
}

Material* MeshComponent::GetMaterial(const int32 index) const
{
	return m_materials[index];
}

void MeshComponent::Render()
{
	Renderer::Instance()->Render(
		m_mesh, m_materials, Owner()->GetTransform()->LocalToWorld(), Owner()->GetWorld()->GetLighting()
	);

	for (Material* material : m_materials)
	{
		material->Dbg_ShowGui();
	}
}
