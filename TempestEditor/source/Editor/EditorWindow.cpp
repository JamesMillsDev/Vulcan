#include "Editor/EditorWindow.h"

#include <imgui.h>

using namespace Tempest::Editor;

EditorWindow::EditorWindow(string title)
	: m_title{ std::move(title) }
{
	
}

EditorWindow::~EditorWindow() = default;

void EditorWindow::Render()
{
	ImGui::Begin(m_title.c_str());

	OnRender();

	ImGui::End();
}
