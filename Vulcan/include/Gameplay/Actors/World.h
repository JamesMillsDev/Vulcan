#pragma once

#include <functional>
#include <queue>
#include <utility>

#include "Gameplay/Actors/Actor.h"

#include "Utility/Collections/TList.h"

using std::function;
using std::pair;
using std::queue;

namespace Vulcan
{
	class Lighting;

	using ActorLifetimeChange = function<void()>;

	class World
	{
		friend class Application;
		friend class GameInstance;

	private:
		uint32 m_nextObjectIndex;
		queue<uint32> m_returnedObjectIndices;

		Actor* m_root;
		Lighting* m_lighting;

		TList<ActorLifetimeChange> m_lifetimeChanges;

	private:
		World();
		~World();

	public:
		template<typename T, typename... ARGS>
		T* MakeActor(ARGS... args);

		void DestroyActor(Actor* actor);

		Lighting* GetLighting() const;
		Actor* GetRootActor() const;

	private:
		void Tick(Actor* actor = nullptr);
		void Render(Actor* actor = nullptr);

	};

	template <typename T, typename ... ARGS>
	T* World::MakeActor(ARGS... args)
	{
		static_assert(std::is_base_of_v<Actor, T>, "T must derive from Actor");

		T* actor = new T{ args... };
		m_lifetimeChanges.Add([this, actor]()
			{
				uint32 objectIndex;
				if (!m_returnedObjectIndices.empty())
				{
					objectIndex = m_returnedObjectIndices.front();
					m_returnedObjectIndices.pop();
				}
				else
				{
					objectIndex = m_nextObjectIndex++;
				}

				actor->m_objectIndex = objectIndex;
				actor->m_world = this;
				actor->GetTransform()->m_isDirty = true;
				actor->GetTransform()->SetParent(m_root->GetTransform());

				actor->BeginPlay();
				actor->ApplyComponentListChanges();

			});

		return actor;
	}
}
