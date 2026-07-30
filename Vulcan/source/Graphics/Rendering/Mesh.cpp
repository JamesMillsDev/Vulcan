// ReSharper disable CppClangTidyBugproneUndefinedMemoryManipulation

#include "Graphics/Rendering/Mesh.h"

#include <format>
#include <tuple>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Resources.h"
#include "Graphics/Vulkan/MemoryBuffer.h"
#include "Graphics/Vulkan/Vulkan.h"
#include "Maths/Maths.h"
#include "Utility/Collections/HashImpls.h"

using glm::mat3;
using std::vector;
using VertexAttribData = std::tuple<uint8, uint8, VkFormat, size_t>;

using namespace Vulcan;

const vector<VertexAttribData> VERTEX_ATTRIBUTES =
{
	std::make_tuple(LocationIndex, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, location)),
	std::make_tuple(NormalIndex, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, normal)),
	std::make_tuple(TangentIndex, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, tangent)),
	std::make_tuple(UvIndex, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)),
	std::make_tuple(ColorIndex, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color))
};

VkVertexInputBindingDescription Vertex::GetBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription;

	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

TArray<VkVertexInputAttributeDescription, VertexAttributeCount> Vertex::GetAttributeDescriptions()
{
	TArray<VkVertexInputAttributeDescription, VertexAttributeCount> attributeDescriptions;

	// Iterate over each attribute definition and assign the descriptor
	for (const auto& [index, binding, format, offset] : VERTEX_ATTRIBUTES)
	{
		attributeDescriptions[index].binding = binding;
		attributeDescriptions[index].location = index;
		attributeDescriptions[index].format = format;
		attributeDescriptions[index].offset = static_cast<uint32>(offset);
	}

	return attributeDescriptions;
}

Mesh::SubMesh::SubMesh(const TList<Vertex>& vertices, const TList<uint16>& indices)
	:vertices{ vertices }, indices{ indices }, m_vertexBufferSize{ sizeof(Vertex) * vertices.Count() },
	m_indexBufferSize{ sizeof(uint16) * indices.Count() }, m_vertexBuffer{ VK_NULL_HANDLE }
{

}

Mesh::SubMesh::~SubMesh()
{
	delete m_vertexBuffer;
	m_vertexBuffer = nullptr;
}

uint64 Mesh::SubMesh::GetHashCode() const
{
	uint64 seed = 0;
	for (Vertex& vert : vertices)
	{
		seed = seed ^ HashAll(vert.location, vert.normal, vert.uv, vert.tangent, vert.color);
	}
	return seed;
}

void Mesh::SubMesh::CreateBuffer()
{
	m_vertexBuffer = new MemoryBuffer
	{
		m_vertexBufferSize + m_indexBufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
	};

	// Copy the vertex and index information into the buffer
	m_vertexBuffer->Fill(vertices.Data(), m_vertexBufferSize);
	m_vertexBuffer->Fill(indices.Data(), m_indexBufferSize, m_vertexBufferSize);
}

Mesh* Mesh::MakeQuad()
{
	return new Mesh
	{
		{
			new SubMesh
			{
				TList<Vertex>
				{
					Vertex
					{
						.location = { -.5f, -.5f, 0.f, 1.f },
						.normal = { 0.f, 0.f, 0.f, 0.f },
						.tangent = { 0.f, 0.f, 0.f, 0.f },
						.uv = { 1.f, 0.f },
						.color = { 1.f, 0.f, 0.f, 1.f }
					},
					Vertex
					{
						.location = { .5f, -.5f, 0.f, 1.f },
						.normal = { 0.f, 0.f, 0.f, 0.f },
						.tangent = { 0.f, 0.f, 0.f, 0.f },
						.uv = { 0.f, 0.f },
						.color = { 0.f, 1.f, 0.0f, 1.f }
					},
					Vertex
					{
						.location = { .5f, .5f, 0.f, 1.f },
						.normal = { 0.f, 0.f, 0.f, 0.f },
						.tangent = { 0.f, 0.f, 0.f, 0.f },
						.uv = { 0.f, 1.f },
						.color = { 0.f, 0.f, 1.f, 1.f }
					},
					Vertex
					{
						.location = { -.5f, .5f, 0.f, 1.f },
						.normal = { 0.f, 0.f, 0.f, 0.f },
						.tangent = { 0.f, 0.f, 0.f, 0.f },
						.uv = { 1.f, 1.f },
						.color = { 1.f, 1.f, 1.f, 1.f }
					}
				},
				TList<uint16>
				{
					0, 1, 2, 2, 3, 0
				}
			}
		}
	};
}

Mesh* Mesh::MakeCube()
{
	TList<Vertex> vertices;
	TList<uint16> indices;

	TArray directions =
	{
		vec4{ 0.f, 1.f, 0.f, 0.f },
		vec4{ 0.f, -1.f, 0.f, 0.f },
		vec4{ 1.f, 0.f, 0.f, 0.f },
		vec4{ -1.f, 0.f, 0.f, 0.f },
		vec4{ 0.f, 0.f, 1.f, 0.f },
		vec4{ 0.f, 0.f, -1.f, 0.f },
	};

	TArray points =
	{
		vec4{ .5f, .5f, -.5f, 1.f },
		vec4{ .5f, .5f, .5f, 1.f },
		vec4{ -.5f, .5f, .5f, 1.f },
		vec4{ -.5f, .5f, -.5f, 1.f },
	};

	TArray uvs =
	{
		vec2{ 0.f, 0.f },
		vec2{ 0.f, 1.f },
		vec2{ 1.f, 1.f },
		vec2{ 1.f, 0.f },
	};

	const vec2 deltaUV1 = uvs[1] - uvs[0];
	const vec2 deltaUV2 = uvs[2] - uvs[0];
	const float f = 1.f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	for (uint64 i = 0; i < directions.Count(); ++i)
	{
		const mat4 orientation = glm::mat4_cast(glm::rotation(vec3{ 0.f, 1.f, 0.f }, vec3{ directions[i] }));

		const vec4 edge1 = orientation * points[1] - orientation * points[0];
		const vec4 edge2 = orientation * points[2] - orientation * points[0];

		for (uint64 j = 0; j < points.Count(); ++j)
		{
			Vertex vert =
			{
				.location = orientation * points[j],
				.normal = directions[i],
				.tangent = {},
				.uv = uvs[j],
				.color = Color::WHITE
			};

			vert.tangent =
			{
				f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
				f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
				f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z),
				0.f
			};

			vertices.Add(vert);
		}
	}

	indices.AddRange({ 0,  3,  1,  1,  3,  2 });
	indices.AddRange({ 4,  7,  5,  5,  7,  6 });
	indices.AddRange({ 8, 11,  9,  9, 11, 10 });
	indices.AddRange({ 12, 15, 13, 13, 15, 14 });
	indices.AddRange({ 16, 19, 17, 17, 19, 18 });
	indices.AddRange({ 20, 23, 21, 21, 23, 22 });

	return new Mesh{ { new SubMesh { vertices, indices } } };
}

Mesh* Mesh::MakeSphere(const float radius, const uint8 stacks, const uint8 sectors)
{
	TList<Vertex> vertices;
	TList<uint16> indices;

	const float sectorStep = 2.f * Maths::PI / static_cast<float>(sectors);
	const float stackStep = Maths::PI / static_cast<float>(stacks);
	const float lengthInv = 1.f / radius;

	for (uint8 i = 0; i <= stacks; ++i)
	{
		const float phi = Maths::PI / 2 - static_cast<float>(i) * stackStep;
		const float xz = radius * Maths::Cos(phi);
		const float y = radius * Maths::Sin(phi);

		const float u = static_cast<float>(i) / static_cast<float>(stacks);

		for (uint8 j = 0; j <= sectors; ++j)
		{
			const float theta = static_cast<float>(j) * sectorStep;
			const float x = xz * Maths::Cos(theta);
			const float z = xz * Maths::Sin(theta);

			const float nX = x * lengthInv;
			const float nY = y * lengthInv;
			const float nZ = z * lengthInv;

			const float v = static_cast<float>(j) / static_cast<float>(sectors);

			vertices.Add(
				{
					.location = { x, y, z, 1.f },
					.normal = vec4{ nX, nY, nZ, 1.f },
					.tangent = vec4{ 0.f },
					.uv = { u, v },
					.color = Color::WHITE
				}
			);
		}
	}

	for (uint8 i = 0; i < stacks; ++i)
	{
		uint16 k1 = i * (sectors + 1);
		uint16 k2 = k1 + sectors + 1;

		for (uint8 j = 0; j < sectors; ++j, ++k1, ++k2)
		{
			if (i != 0)
			{
				indices.Add(k1);
				indices.Add(k1 + 1);
				indices.Add(k2);
			}

			if (i != stacks - 1)
			{
				indices.Add(k1 + 1);
				indices.Add(k2 + 1);
				indices.Add(k2);
			}
		}
	}

	return new Mesh
	{
		{
			new SubMesh
			{
				vertices, indices
			}
		}
	};
}

Mesh* Mesh::MakeFromAssimp(const string& file)
{
	ResourceData meshData = Resources::Find(file);

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFileFromMemory(
		meshData.data, meshData.length,
		aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GlobalScale | aiProcess_FlipUVs
	);

	TList<SubMesh*> subMeshes;

	for (uint32 i = 0; i < scene->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[i];

		TList<Vertex> vertices;
		vertices.Resize(mesh->mNumVertices);

		TList<uint16> indices;

		for (uint32 v = 0; v < mesh->mNumVertices; ++v)
		{
			Vertex vert;

			if (mesh->HasPositions())
			{
				aiVector3D location = mesh->mVertices[v];

				vert.location = { location.x, location.y, location.z, 1.f };
			}

			if (mesh->HasNormals())
			{
				aiVector3D normal = mesh->mNormals[v];

				vert.normal = { normal.x, normal.y, normal.z, 0.f };
			}

			if (mesh->HasTangentsAndBitangents())
			{
				aiVector3D tangent = mesh->mTangents[v];

				vert.tangent = { tangent.x, tangent.y, tangent.z, 0.f };
			}

			if (mesh->HasTextureCoords(0))
			{
				aiVector3D uv0 = mesh->mTextureCoords[0][v];

				vert.uv = { uv0.x, uv0.y };
			}

			if (mesh->HasVertexColors(0))
			{
				aiColor4D color0 = mesh->mColors[0][v];

				vert.color = Color{ color0.r, color0.g, color0.b, color0.a };
			}
			else
			{
				vert.color = Color{ 1.f, 1.f, 1.f, 1.f };
			}

			vertices[v] = vert;
		}

		if (mesh->HasFaces())
		{
			for (uint32 f = 0; f < mesh->mNumFaces; ++f)
			{
				indices.Add(static_cast<uint16>(mesh->mFaces[f].mIndices[1]));
				indices.Add(static_cast<uint16>(mesh->mFaces[f].mIndices[2]));
				indices.Add(static_cast<uint16>(mesh->mFaces[f].mIndices[0]));

				// generate a second triangle for quads
				if (mesh->mFaces[f].mNumIndices == 4)
				{
					indices.Add(static_cast<uint16>(mesh->mFaces[f].mIndices[2]));
					indices.Add(static_cast<uint16>(mesh->mFaces[f].mIndices[3]));
					indices.Add(static_cast<uint16>(mesh->mFaces[f].mIndices[0]));
				}
			}
		}

		subMeshes.Add(new SubMesh{ vertices, indices });
	}

	return new Mesh{ subMeshes };
}

Mesh::Mesh(const TList<SubMesh*>& subMeshes)
	: subMeshes{ subMeshes }
{
	CreateBuffers();
}

Mesh::~Mesh()
{
	DestroyBuffers();
}

uint64 Mesh::GetHashCode() const
{
	uint64 seed = 0;
	for (SubMesh*& subMesh : subMeshes)
	{
		seed = seed ^ subMesh->GetHashCode();
	}
	return seed;
}

void Mesh::CreateBuffers()
{
	for (SubMesh*& subMesh : subMeshes)
	{
		subMesh->CreateBuffer();
	}
}

void Mesh::DestroyBuffers()
{
	for (SubMesh*& subMesh : subMeshes)
	{
		delete subMesh;
	}

	subMeshes.Clear();
}

void Mesh::Render(const VkCommandBuffer buffer, const uint32 instances, const uint32 firstInstance) const
{
	VkDeviceSize offsets[] = { 0 };

	for (SubMesh* subMesh : subMeshes)
	{
		vkCmdBindVertexBuffers(buffer, 0, 1, &subMesh->m_vertexBuffer->Get(), offsets);
		vkCmdBindIndexBuffer(buffer, subMesh->m_vertexBuffer->Get(), subMesh->m_vertexBufferSize, VK_INDEX_TYPE_UINT16);

		vkCmdDrawIndexed(
			buffer, static_cast<uint32>(subMesh->indices.Count()), instances, 0, 0, firstInstance
		);
	}
}

uint64 hash<Vertex>::operator()(const Vertex& vertex) const noexcept
{
	return HashAll(vertex.location, vertex.normal, vertex.tangent, vertex.uv, vertex.color);
}
