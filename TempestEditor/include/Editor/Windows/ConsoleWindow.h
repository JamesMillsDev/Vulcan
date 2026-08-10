#pragma once

#include "Editor/EditorWindow.h"

namespace Tempest::Editor
{
	class ConsoleWindow : public EditorWindow
	{
	public:
		ConsoleWindow();

	protected:
		void OnRender() override;

	};
}
