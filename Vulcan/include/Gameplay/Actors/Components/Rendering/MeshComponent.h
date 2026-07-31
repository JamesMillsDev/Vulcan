#pragma once

#include "Gameplay/Actors/Components/IComponent.h"
#include "Utility/Collections/TList.h"

namespace Vulcan
{
	class Material;
	class Mesh;

	class MeshComponent final : public IComponent
	{
	private:
		Mesh* m_mesh;
		TList<Material*> m_materials;

	public:
		explicit MeshComponent(Mesh* mesh, TList<Material*> materials);

	public:
		Material* GetMaterial(int32 index) const;

		void PreRender() override;
		void Render() override;

	};
}
