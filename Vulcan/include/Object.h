#pragma once

#include <bitset>

#include "Maths/Alias.h"

using std::hash;

namespace Vulcan
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
	struct hash<Vulcan::Object>
	{
		uint64 operator()(const Vulcan::Object& obj) const noexcept
		{
			return obj.GetHashCode();
		}
	};
}
