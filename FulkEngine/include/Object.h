#pragma once

#include <bitset>

#include "Maths/Alias.h"

using std::hash;

namespace Fulk
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
	struct hash<Fulk::Object>
	{
		uint64 operator()(const Fulk::Object& obj) const noexcept
		{
			return obj.GetHashCode();
		}
	};
}
