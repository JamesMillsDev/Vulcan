#pragma once

#include <Application.h>
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

	private:
		EditorApplication();

	protected:
		void Init(Vulkan* vulkan) override;
		void PreRender() override;
		void Render() override;
		void PostRender() override;
		void Tick() override;
		void Shutdown() override;

	private:
		void InitialiseImGui(Vulkan* vulkan);

	};
}
