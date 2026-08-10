#include "Editor/Windows/WorldWindow.h"

#include <imgui.h>
#include <iostream>

using namespace Tempest::Editor;

void WorldWindow::AddActorButton(const AddActorCb& addActorCb, const float buttonPercentage, const float rounding)
{
	const float availableWidth = ImGui::GetContentRegionAvail().x;
	const float buttonWidth = availableWidth * buttonPercentage; // button will be 10% of the available area

	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, rounding);

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availableWidth - buttonWidth);
	if (ImGui::Button("+", { buttonWidth, 0.f }))
	{
		addActorCb();
	}

	ImGui::PopStyleVar();
}

WorldWindow::WorldWindow()
	: EditorWindow{ "World" }
{
	
}

void WorldWindow::OnRender()
{
	AddActorButton([]
		{
			std::cout << "Adding actor\n";
		});
}
