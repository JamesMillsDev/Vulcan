#include "Gameplay/Actors/Transform.h"

#include "Maths/Maths.h"

#include "Utility/Collections/HashImpls.h"

Transform::Transform()
	: location{ 0.f }, rotation{ quat{} }, scale{ 1.f }, parent{ nullptr }, nextSibling{ nullptr },
	previousSibling{ nullptr }, lastChild{ nullptr }, m_owner{ nullptr }
{}

Transform::~Transform()
{
	while (lastChild != nullptr)
	{
		lastChild->SetParent(nullptr);
	}

	if (parent != nullptr)
	{
		SetParent(nullptr);
	}
}

uint64 Transform::GetHashCode() const
{
	return HashAll(location, rotation, scale);
}

Actor* Transform::Owner() const
{
	return m_owner;
}

mat4 Transform::LocalToWorld() const
{
	return parent != nullptr ? parent->LocalToWorld() * LocalToParent() : LocalToParent();
}

mat4 Transform::WorldToLocal() const
{
	return parent != nullptr ? ParentToLocal() * parent->WorldToLocal() : ParentToLocal();
}

vec3 Transform::Right() const
{
	return LocalToWorld()[0];
}

vec3 Transform::Up() const
{
	return LocalToWorld()[1];
}

vec3 Transform::Forward() const
{
	return LocalToWorld()[2];
}

void Transform::SetParent(Transform* newParent, Transform* before)
{
	ValidatePointers();

	if (parent != nullptr)
	{
		if (previousSibling != nullptr)
		{
			previousSibling->nextSibling = nextSibling;
		}

		if (nextSibling != nullptr)
		{
			nextSibling->previousSibling = previousSibling;
		}
		else
		{
			parent->lastChild = previousSibling;
		}

		nextSibling = previousSibling = nullptr;
	}

	parent = newParent;

	if (parent != nullptr)
	{
		if (before != nullptr)
		{
			previousSibling = before->previousSibling;
			nextSibling = before;
			nextSibling->previousSibling = this;
		}
		else
		{
			previousSibling = parent->lastChild;
			parent->lastChild = this;
		}

		if (previousSibling != nullptr)
		{
			previousSibling->nextSibling = this;
		}
	}

	ValidatePointers();
}

void Transform::ForEachChild(const IterationFunc& iteration) const
{
	int index = 0;
	Transform* child = lastChild;

	while (child != nullptr)
	{
		iteration(child, index++);

		child = child->previousSibling;
	}
}

mat4 Transform::LocalToParent() const
{
	return glm::translate(mat4{ 1.f }, location) * 
		glm::mat4_cast(rotation) * 
		glm::scale(mat4{ 1.f }, scale);
}

mat4 Transform::ParentToLocal() const
{
	const vec3 inverseScale = 
	{
		Maths::IsNearZero(scale.x) ? 0.f : 1.f / scale.x,
		Maths::IsNearZero(scale.y) ? 0.f : 1.f / scale.y,
		Maths::IsNearZero(scale.z) ? 0.f : 1.f / scale.z,
	};

	return glm::scale(mat4{ 1.f }, inverseScale) * 
		glm::mat4_cast(glm::inverse(rotation)) *
		glm::translate(mat4{ 1.f }, -location);
}

void Transform::ValidatePointers() const
{
	if (parent == nullptr)
	{
		assert(previousSibling == nullptr);
		assert(nextSibling == nullptr);
	}
	else
	{
		assert((nextSibling == nullptr) == (this == parent->lastChild));
	}

	assert(previousSibling == nullptr || previousSibling->nextSibling == this);
	assert(nextSibling == nullptr || nextSibling->previousSibling == this);
	assert(lastChild == nullptr || lastChild->parent == this);
}
