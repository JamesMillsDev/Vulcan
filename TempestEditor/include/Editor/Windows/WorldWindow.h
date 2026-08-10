#pragma once

#include <functional>

#include "Editor/EditorWindow.h"

using AddActorCb = std::function<void()>;

namespace Tempest::Editor
{
	class WorldWindow : public EditorWindow
	{
	private:
		static void AddActorButton(const AddActorCb& addActorCb, float buttonPercentage = .1f, float rounding = 6.f);

	public:
		WorldWindow();

	protected:
		void OnRender() override;

	};
}
