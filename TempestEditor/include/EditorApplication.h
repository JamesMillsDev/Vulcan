#pragma once

#include <Application.h>
#include <imgui.h>
#include <vulkan/vulkan_core.h>

namespace Tempest::Editor
{
	class FlyCamera;
	class Menu;

	class EditorApplication : public Application
	{
	public:
		static EExitCode Open();

	private:
		FlyCamera* m_camera;
		Menu* m_mainMenu;

		VkDescriptorPool m_imguiPool;
		ImGuiID m_dockSpaceId;

	private:
		EditorApplication();

	protected:
		void Init(Vulkan* vulkan) override;
		void Tick() override;
		void PreRender() override;
		void Render() override;
		void PostRender() override;
		void Shutdown() override;

	private:
		void InitialiseImGui(Vulkan* vulkan);

	};
}
