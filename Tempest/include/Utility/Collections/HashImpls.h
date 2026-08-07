#pragma once

#include "Object.h"
#include "Maths/Color.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;

namespace Tempest
{
	[[nodiscard]] inline uint64 HashCombine(uint64 seed, uint64 hash)
	{
		return seed ^ (hash + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
	}

	template<typename T>
	[[nodiscard]] static uint64 HashValue(T value)
	{
		hash<T> hasher;

		return hasher(value);
	}

	template<typename... ARGS>
	uint64 HashAll(ARGS&&... args)
	{
		uint64 seed = 0;
		((seed = HashCombine(seed, HashValue(std::forward<ARGS>(args)))), ...);
		return seed;
	}
}

namespace std
{
	template<>
	struct hash<Tempest::Color>
	{
		uint64 operator()(const Tempest::Color& color) const noexcept
		{
			return Tempest::HashAll(color.r, color.g, color.b, color.a);
		}
	};

	template<>
	struct hash<vec2>
	{
		uint64 operator()(const vec2& vec) const noexcept
		{
			return Tempest::HashAll(vec.x, vec.y);
		}
	};

	template<>
	struct hash<vec3>
	{
		uint64 operator()(const vec3& vec) const noexcept
		{
			return Tempest::HashAll(vec.x, vec.y, vec.z);
		}
	};

	template<>
	struct hash<vec4>
	{
		uint64 operator()(const vec4& vec) const noexcept
		{
			return Tempest::HashAll(vec.x, vec.y, vec.x, vec.w);
		}
	};

	template<>
	struct hash<quat>
	{
		uint64 operator()(const quat& quaternion) const noexcept
		{
			return Tempest::HashAll(quaternion.x, quaternion.y, quaternion.x, quaternion.w);
		}
	};

	template<>
	struct hash<mat4>
	{
		uint64 operator()(const mat4& matrix) const noexcept
		{
			return Tempest::HashAll(matrix[0], matrix[1], matrix[2], matrix[4]);
		}
	};
}