#pragma once

#include <functional>
#include <utility>
#include <glm/mat4x4.hpp>

#include "Utility/Collections/TList.h"
#include "Gameplay/Actors/Components/IComponent.h"

using std::function;
using std::pair;
using glm::mat4;

namespace Vulcan
{
	class Transform;

	using ComponentListChange = function<void()>;

	struct DirtyTransform
	{
		uint32 index;
		mat4 value;
	};

	class Actor
	{
		friend class World;

	private:
		World* m_world;
		Transform* m_transform;
		uint32 m_objectIndex;

		TList<IComponent*> m_components;
		TList<ComponentListChange> m_componentListChanges;

	private:
		Actor();
		virtual ~Actor();

	public:
		virtual void BeginPlay();

		virtual void Tick();
		virtual void Render();

		virtual void EndPlay();

		template<typename T, typename... ARGS>
		T* MakeComponent(ARGS... args);

		template<typename T>
		T* GetComponent();

		template<typename T>
		TList<T*> GetComponents();

		void DestroyComponent(IComponent* component);

		Transform* GetTransform() const;

		World const* GetWorld() const;
		uint32 GetObjectIndex() const;

		TList<DirtyTransform> CollectDirtyTransforms();

	private:
		void ApplyComponentListChanges();

		void CollectDirtyTransforms(TList<DirtyTransform>& transforms, Transform* target = nullptr);

	};

	template <typename T, typename... ARGS>
	T* Actor::MakeComponent(ARGS... args)
	{
		static_assert(std::is_base_of_v<IComponent, T>, "T must derive from IComponent");

		T* newComp = new T{ args... };
		m_componentListChanges.Add([this, newComp]
			{
				newComp->BeginPlay();
				m_components.Add(newComp);
			});

		newComp->m_owner = this;
		return newComp;
	}

	template <typename T>
	T* Actor::GetComponent()
	{
		for (IComponent* component : m_components)
		{
			if (T* comp = dynamic_cast<T*>(component))
			{
				return comp;
			}
		}

		return nullptr;
	}

	template <typename T>
	TList<T*> Actor::GetComponents()
	{
		TList<T*> components;

		for (IComponent* component : m_components)
		{
			if (T* comp = dynamic_cast<T*>(component))
			{
				components.Add(comp);
			}
		}

		return components;
	}

}