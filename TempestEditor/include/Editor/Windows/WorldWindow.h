#pragma once

#include "Editor/EditorWindow.h"

namespace Tempest::Editor
{
	class WorldWindow : public EditorWindow
	{
	public:
		WorldWindow();

	protected:
		void OnRender() override;

	};
}
