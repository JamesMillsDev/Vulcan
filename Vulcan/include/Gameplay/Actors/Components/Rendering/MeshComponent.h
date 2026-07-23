#pragma once

#include "Gameplay/Actors/Components/IComponent.h"

namespace Vulcan
{
	class Material;
	class Mesh;

	class MeshComponent final : public IComponent
	{
	private:
		Mesh* m_mesh;
		Material* m_material;

	public:
		MeshComponent(Mesh* mesh, Material* material);

	public:
		void Render() override;

	};
}