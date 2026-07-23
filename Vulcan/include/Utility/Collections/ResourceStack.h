#pragma once

#include <functional>

#include "TStack.h"

using CleanupFunction = std::function<void()>;

class ResourceStack : public TStack<CleanupFunction>
{
public:
	~ResourceStack() override;

};