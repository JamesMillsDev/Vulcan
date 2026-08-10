#pragma once

#include "Editor/EditorWindow.h"

namespace Tempest::Editor
{
	class ProjectWindow : public EditorWindow
	{
	public:
		ProjectWindow();

	protected:
		void OnRender() override;

	};
}
