#pragma once

#include "Editor/EditorWindow.h"

namespace Tempest::Editor
{
	class InspectorWindow : public EditorWindow
	{
	public:
		InspectorWindow();

	protected:
		void OnRender() override;

	};
}
