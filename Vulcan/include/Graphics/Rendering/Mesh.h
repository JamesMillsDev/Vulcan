#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vulkan/vulkan.h>

#include "Maths/Color.h"
#include "Utility/Collections/TArray.h"
#include "Utility/Collections/TList.h"

using glm::vec2;
using glm::vec3;
using glm::vec4;

namespace Vulcan
{
	class Material;
	struct MaterialBindInfo;
	class MemoryBuffer;

	enum : uint8
	{
		LocationIndex,
		NormalIndex,
		TangentIndex,
		UvIndex,
		ColorIndex,
		VertexAttributeCount
	};

	struct Vertex
	{
	public:
		static VkVertexInputBindingDescription GetBindingDescription();
		static TArray<VkVertexInputAttributeDescription, VertexAttributeCount> GetAttributeDescriptions();

	public:
		/** @brief The location of the vertex in model space. */
		vec4 location;
		/** @brief The normal of the vertex in model space. */
		vec4 normal;
		/** @brief The tangent of the vertex in model space. */
		vec4 tangent;

		/** @brief The first texture coordinate of the vertex. */
		vec2 uv;
		/** @brief The first color of the vertex. */
		Color color;

	};

	class Mesh : public Object
	{
		friend class Renderer;
		friend class Vulkan;

	public:
		struct SubMesh : Object
		{
			friend Mesh;

		public:
			TList<Vertex> vertices;
			TList<uint16> indices;
			uint32 materialIndex;

		private:
			VkDeviceSize m_vertexBufferSize;
			VkDeviceSize m_indexBufferSize;

			MemoryBuffer* m_vertexBuffer;

		public:
			SubMesh(const TList<Vertex>& vertices, const TList<uint16>& indices, uint32 materialIndex);
			~SubMesh() override;

		public:
			[[nodiscard]] uint64 GetHashCode() const override;

		private:
			void CreateBuffer();

		};

	public:
		static Mesh* MakeQuad();
		static Mesh* MakeCube();
		static Mesh* MakeSphere(float radius = 1.f, uint8 stacks = 64, uint8 sectors = 64);
		static Mesh* MakeFromAssimp(const string& file);

	public:
		TList<SubMesh*> subMeshes;
		uint32 materialCount;

	public:
		Mesh(const TList<SubMesh*>& subMeshes, uint32 materialCount);
		~Mesh() override;

	public:
		[[nodiscard]] uint64 GetHashCode() const override;

	private:
		void CreateBuffers();
		void DestroyBuffers();

		void Render(VkCommandBuffer buffer, const TList<Material*>& materials, const MaterialBindInfo& bindInfo, uint32 instances = 1, uint32 firstInstance = 0) const;

	};
}

namespace std
{
	template<>
	struct hash<Vulcan::Vertex>
	{
		uint64 operator()(const Vulcan::Vertex& vertex) const noexcept;
	};
}
