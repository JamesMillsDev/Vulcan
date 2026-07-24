#include "Gameplay/Actors/Actor.h"

#include "Gameplay/Actors/Transform.h"

using namespace Vulcan;

Actor::Actor()
	: m_world{ nullptr }, m_transform{ new Transform }
{
	m_transform->m_owner = this;
}

Actor::~Actor()
{
	while (m_transform->m_lastChild != nullptr)
	{
		Transform* transform = m_transform->m_lastChild;
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

World const* Actor::GetWorld() const
{
	return m_world;
}

uint32 Actor::GetObjectIndex() const
{
	return m_objectIndex;
}

TList<DirtyTransform> Actor::CollectTransforms() const
{
	TList<DirtyTransform> transforms;
	CollectTransforms(transforms);
	return transforms;
}

void Actor::ApplyComponentListChanges()
{
	for (const ComponentListChange& change : m_componentListChanges)
	{
		change();
	}
	m_componentListChanges.Clear();
}

void Actor::CollectTransforms(TList<DirtyTransform>& transforms, const Transform* target) const
{
	if (target == nullptr)
	{
		target = m_transform;
	}

	if (m_transform->m_isDirty)
	{
		transforms.Add({ .index = m_objectIndex, .value = m_transform->LocalToWorld() });
		target->ForEachChild([&](const Transform* child, int)
			{
				CollectTransforms(transforms, child);
			});

		m_transform->m_isDirty = false;
	}
}
