#include "Gameplay/Actors/World.h"

#include "Gameplay/Actors/Transform.h"
#include "Graphics/Rendering/Lighting.h"

using namespace Vulcan;

World::World()
	: m_nextObjectIndex{ 0 }, m_root{ new Actor }, m_lighting{ new Lighting }
{}

World::~World()
{
	delete m_root;
	delete m_lighting;
}

void World::DestroyActor(Actor* actor)
{
	m_lifetimeChanges.Add([this, actor]
		{
			m_returnedObjectIndices.push(actor->GetObjectIndex());
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

Lighting* World::GetLighting() const
{
	return m_lighting;
}

Actor* World::GetRootActor() const
{
	return m_root;
}

void World::Tick(Actor* actor)
{
	if (actor == nullptr)
	{
		m_lighting->UpdateBuffers();

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

void World::PreRender(Actor* actor)
{
	if (actor == nullptr)
	{
		actor = m_root;
	}

	if (actor != m_root)
	{
		for (IComponent* component : actor->m_components)
		{
			component->PreRender();
		}
	}

	actor->GetTransform()->ForEachChild([this](const Transform* child, int index)
		{
			Render(child->Owner());
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

void World::PostRender(Actor* actor)
{
	if (actor == nullptr)
	{
		actor = m_root;

		m_lighting->Dbg_ShowGui();
	}

	if (actor != m_root)
	{
		for (IComponent* component : actor->m_components)
		{
			component->PostRender();
		}
	}

	actor->GetTransform()->ForEachChild([this](const Transform* child, int index)
		{
			Render(child->Owner());
		});
}
