#include "Gameplay/Actors/Transform.h"

#include "Maths/Maths.h"
#include "Utility/Collections/HashImpls.h"

using namespace Tempest;

Transform::Transform() :
	m_location{ 0.f }, m_rotation{ quat{} }, m_scale{ 1.f }, m_parent{ nullptr }, m_nextSibling{ nullptr },
	m_previousSibling{ nullptr }, m_lastChild{ nullptr }, m_owner{ nullptr }, m_isDirty{ false }
{}

Transform::~Transform()
{
	while (m_lastChild != nullptr)
	{
		m_lastChild->SetParent(nullptr);
	}

	if (m_parent != nullptr)
	{
		SetParent(nullptr);
	}
}

uint64 Transform::GetHashCode() const
{
	return HashAll(m_location, m_rotation, m_scale);
}

bool Transform::IsDirty() const
{
	return m_isDirty;
}

Actor* Transform::Owner() const
{
	return m_owner;
}

mat4 Transform::LocalToWorld() const
{
	return m_parent != nullptr ? m_parent->LocalToWorld() * LocalToParent() : LocalToParent();
}

mat4 Transform::WorldToLocal() const
{
	return m_parent != nullptr ? ParentToLocal() * m_parent->WorldToLocal() : ParentToLocal();
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

vec3 Transform::Location() const
{
	return m_location;
}

quat Transform::Rotation() const
{
	return m_rotation;
}

vec3 Transform::Scale() const
{
	return m_scale;
}

void Transform::SetLocation(const vec3 newValue)
{
	m_location = newValue;
	m_isDirty = true;
}

void Transform::SetRotation(const quat newValue)
{
	m_rotation = newValue;
	m_isDirty = true;
}

void Transform::SetScale(const vec3 newValue)
{
	m_scale = newValue;
	m_isDirty = true;
}

void Transform::UpdateLocation(const vec3 deltaValue)
{
	m_location += deltaValue;
	m_isDirty = true;
}

void Transform::UpdateRotation(const quat deltaValue)
{
	m_rotation *= deltaValue;
	m_isDirty = true;
}

void Transform::UpdateScale(const vec3 deltaValue)
{
	m_scale += deltaValue;
	m_isDirty = true;
}

void Transform::SetParent(Transform* newParent, Transform* before)
{
	ValidatePointers();

	if (m_parent != nullptr)
	{
		if (m_previousSibling != nullptr)
		{
			m_previousSibling->m_nextSibling = m_nextSibling;
		}

		if (m_nextSibling != nullptr)
		{
			m_nextSibling->m_previousSibling = m_previousSibling;
		}
		else
		{
			m_parent->m_lastChild = m_previousSibling;
		}

		m_nextSibling = m_previousSibling = nullptr;
	}

	m_parent = newParent;

	if (m_parent != nullptr)
	{
		if (before != nullptr)
		{
			m_previousSibling = before->m_previousSibling;
			m_nextSibling = before;
			m_nextSibling->m_previousSibling = this;
		}
		else
		{
			m_previousSibling = m_parent->m_lastChild;
			m_parent->m_lastChild = this;
		}

		if (m_previousSibling != nullptr)
		{
			m_previousSibling->m_nextSibling = this;
		}
	}

	ValidatePointers();
}

void Transform::ForEachChild(const IterationFunc& iteration) const
{
	int index = 0;
	Transform* child = m_lastChild;

	while (child != nullptr)
	{
		iteration(child, index++);

		child = child->m_previousSibling;
	}
}

mat4 Transform::LocalToParent() const
{
	return glm::translate(mat4{ 1.f }, m_location) *
		glm::mat4_cast(m_rotation) *
		glm::scale(mat4{ 1.f }, m_scale);
}

mat4 Transform::ParentToLocal() const
{
	const vec3 inverseScale =
	{
		Maths::IsNearZero(m_scale.x) ? 0.f : 1.f / m_scale.x,
		Maths::IsNearZero(m_scale.y) ? 0.f : 1.f / m_scale.y,
		Maths::IsNearZero(m_scale.z) ? 0.f : 1.f / m_scale.z,
	};

	return glm::scale(mat4{ 1.f }, inverseScale) *
		glm::mat4_cast(glm::inverse(m_rotation)) *
		glm::translate(mat4{ 1.f }, -m_location);
}

void Transform::ValidatePointers() const
{
	if (m_parent == nullptr)
	{
		assert(m_previousSibling == nullptr);
		assert(m_nextSibling == nullptr);
	}
	else
	{
		assert((m_nextSibling == nullptr) == (this == m_parent->m_lastChild));
	}

	assert(m_previousSibling == nullptr || m_previousSibling->m_nextSibling == this);
	assert(m_nextSibling == nullptr || m_nextSibling->m_previousSibling == this);
	assert(m_lastChild == nullptr || m_lastChild->m_parent == this);
}
