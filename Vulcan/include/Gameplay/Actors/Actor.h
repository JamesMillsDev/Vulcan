#pragma once

#include <functional>
#include <utility>

#include "Utility/Collections/TList.h"
#include "Gameplay/Actors/Components/IComponent.h"

class Transform;

using std::function;
using std::pair;

using ComponentListChange = function<void()>;

class Actor
{
	friend class World;

private:
	Transform* m_transform;

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

	void DestroyComponent(IComponent* component);

	Transform* GetTransform() const;

private:
	void ApplyComponentListChanges();

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
