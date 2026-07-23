#pragma once

#include <vk_mem_alloc.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vulkan/vulkan.h>

#include "Maths/Color.h"
#include "Utility/Collections/TArray.h"
#include "Utility/Collections/TList.h"

class MemoryBuffer;

using glm::vec2;
using glm::vec3;
using glm::vec4;

enum : uint8
{
	LocationIndex,
	NormalIndex,
	TangentIndex,
	BiTangentIndex,
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
	vec3 location;
	/** @brief The normal of the vertex in model space. */
	vec4 normal;
	/** @brief The tangent of the vertex in model space. */
	vec4 tangent;
	/** @brief The bitangent of the vertex in model space. */
	vec4 biTangent;

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

	private:
		VkDeviceSize m_vertexBufferSize;
		VkDeviceSize m_indexBufferSize;

		MemoryBuffer* m_vertexBuffer;

	public:
		SubMesh(const TList<Vertex>& vertices, const TList<uint16>& indices);
		~SubMesh() override;

	public:
		[[nodiscard]] uint64 GetHashCode() const override;

	private:
		void CreateBuffer();

	};

public:
	static Mesh* MakeQuad();
	static Mesh* MakeFromAssimp(const string& file);

public:
	TList<SubMesh*> subMeshes;

public:
	explicit Mesh(const TList<SubMesh*>& subMeshes);
	~Mesh() override;

public:
	[[nodiscard]] uint64 GetHashCode() const override;

private:
	void CreateBuffers();
	void DestroyBuffers();

	void Render(VkCommandBuffer buffer, uint32 instances = 1, uint32 firstInstance = 0) const;

};

namespace std
{
	template<>
	struct hash<Vertex>
	{
		uint64 operator()(const Vertex& vertex) const noexcept;
	};
}
