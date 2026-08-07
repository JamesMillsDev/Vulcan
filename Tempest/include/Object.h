#pragma once

#include <bitset>

#include "Maths/Alias.h"

using std::hash;

namespace Tempest
{
	class Object
	{
	public:
		virtual ~Object() = default;

	public:
		[[nodiscard]] virtual uint64 GetHashCode() const = 0;

	};
}

namespace std
{
	template<>
	struct hash<Tempest::Object>
	{
		uint64 operator()(const Tempest::Object& obj) const noexcept
		{
			return obj.GetHashCode();
		}
	};
}
