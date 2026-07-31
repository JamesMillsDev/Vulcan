#pragma once

#include <string>

#include <glm/mat4x4.hpp>

#include <vulkan/vulkan.h>

#include "Uniforms.h"

#include "Rendering/Camera.h"
#include "Utility/Collections/TList.h"

struct GLFWwindow;

using std::string;
using glm::mat4;

namespace Vulcan
{
	class Lighting;
	struct MaterialUniform;
	class MemoryBuffer;
	class Application;
	class Camera;
	class Config;
	struct GraphicsPipelineConfig;
	class Material;
	class Mesh;
	class Vulkan;

	class Renderer
	{
		friend Application;

	private:
		static Renderer* m_instance;
		static Camera* m_currentCamera;

	public:
		static Renderer* Instance();
		[[nodiscard]] static bool IsValid();

		static Camera* GetCurrentCamera();
		static void SetCurrent(Camera* newCurrent);

	private:
		static void Create(Config* config, GLFWwindow* window);
		static void Destroy();

		static void InitVulkan(Config* config, GLFWwindow* window);
		static void DestroyVulkan();

		static void WaitIdle();

	private:
		VkCommandBuffer m_frameCmdBuf;
		Vulkan* m_vulkan;

		GlobalsUniform m_globalsUniform;
		MemoryBuffer* m_globalUniformBuffer;

		mat4* m_transforms;
		MemoryBuffer* m_transformBuffer;

		MaterialUniform* m_materials;
		MemoryBuffer* m_materialBuffer;

	private:
		explicit Renderer(Config* config, GLFWwindow* window);
		~Renderer();

	public:
		void UpdateBuffer(Material* material, uint32 objectIndex) const;
		void Render(const Mesh* mesh, const TList<Material*>& materials, uint32 objectIndex, const Lighting* lighting) const;

	private:
		void BeginFrame();
		void EndFrame();

		void UpdateBuffers();

	};
}

