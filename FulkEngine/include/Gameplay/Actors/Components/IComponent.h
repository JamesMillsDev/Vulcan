#pragma once

namespace Fulk
{
	class Actor;

	class IComponent
	{
		friend Actor;
		friend class World;

	private:
		Actor* m_owner;

	protected:
		IComponent()
			: m_owner{ nullptr }
		{}

		virtual ~IComponent() = default;

	public:
		Actor* Owner() const
		{
			return m_owner;
		}

	public:
		virtual void BeginPlay()
		{}

		virtual void Tick()
		{}

		virtual void PreRender()
		{}

		virtual void Render()
		{}

		virtual void PostRender()
		{}

		virtual void EndPlay()
		{}

	};
}