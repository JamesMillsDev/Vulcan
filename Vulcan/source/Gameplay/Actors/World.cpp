#include "Gameplay/Actors/World.h"

#include "Gameplay/Actors/Transform.h"

World::World()
	: m_root{ new Actor }
{}

World::~World()
{
	delete m_root;
}

void World::DestroyActor(Actor* actor)
{
	m_lifetimeChanges.Add([this, actor]
		{
			actor->GetTransform()->SetParent(nullptr);

			actor->EndPlay();
			actor->ApplyComponentListChanges();

			for (IComponent* component : actor->m_components)
			{
				component->EndPlay();
			}

			delete actor;
		});
}

void World::Tick(Actor* actor)
{
	if (actor == nullptr)
	{
		actor = m_root;
		for (const ActorLifetimeChange& change : m_lifetimeChanges)
		{
			change();
		}
		m_lifetimeChanges.Clear();
	}

	actor->ApplyComponentListChanges();

	actor->Tick();
	for (IComponent* component : actor->m_components)
	{
		component->Tick();
	}

	actor->GetTransform()->ForEachChild([this](const Transform* child, int index)
		{
			Tick(child->Owner());
		});
}

void World::Render(Actor* actor)
{
	if (actor == nullptr)
	{
		actor = m_root;
	}

	if (actor != m_root)
	{
		actor->Render();
		for (IComponent* component : actor->m_components)
		{
			component->Render();
		}
	}

	actor->GetTransform()->ForEachChild([this](const Transform* child, int index)
		{
			Render(child->Owner());
		});
}