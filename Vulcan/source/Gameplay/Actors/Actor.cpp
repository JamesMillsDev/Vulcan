#include "Gameplay/Actors/Actor.h"

#include "Gameplay/Actors/Transform.h"

Actor::Actor()
	: m_transform{ new Transform }
{
	m_transform->m_owner = this;
}

Actor::~Actor()
{
	while (m_transform->lastChild != nullptr)
	{
		Transform* transform = m_transform->lastChild;
		transform->SetParent(nullptr);
		delete transform->Owner();
	}

	delete m_transform;

	for (IComponent* component : m_components)
	{
		DestroyComponent(component);
	}

	ApplyComponentListChanges();
}

void Actor::BeginPlay()
{}

void Actor::Tick()
{}

void Actor::Render()
{}

void Actor::EndPlay()
{}

void Actor::DestroyComponent(IComponent* component)
{
	m_componentListChanges.Add([this, component]
		{
			m_components.Remove(component);
			component->EndPlay();
			delete component;
		});
}

Transform* Actor::GetTransform() const
{
	return m_transform;
}

void Actor::ApplyComponentListChanges()
{
	for (const ComponentListChange& change : m_componentListChanges)
	{
		change();
	}
	m_componentListChanges.Clear();
}
