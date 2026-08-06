#pragma once

#include <functional>

#include "TStack.h"

namespace Fulk
{
	using CleanupFunction = std::function<void()>;

	class ResourceStack : public TStack<CleanupFunction>
	{
	public:
		~ResourceStack() override;

	};
}