#pragma once

#include <functional>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Object.h"

using glm::mat4;
using glm::quat;
using glm::vec3;

namespace Fulk
{
	class Actor;

	using IterationFunc = std::function<void(class Transform*, int)>;

	class Transform : public Object
	{
		friend Actor;
		friend class World;

	private:
		vec3 m_location;
		quat m_rotation;
		vec3 m_scale;

		Transform* m_parent;
		Transform* m_nextSibling;
		Transform* m_previousSibling;
		Transform* m_lastChild;

		Actor* m_owner;
		bool m_isDirty;

	private:
		Transform();
		~Transform() override;

	public:
		[[nodiscard]] uint64 GetHashCode() const override;
		[[nodiscard]] bool IsDirty() const;

		[[nodiscard]] Actor* Owner() const;

		[[nodiscard]] mat4 LocalToWorld() const;
		[[nodiscard]] mat4 WorldToLocal() const;

		[[nodiscard]] vec3 Right() const;
		[[nodiscard]] vec3 Up() const;
		[[nodiscard]] vec3 Forward() const;

		[[nodiscard]] vec3 Location() const;
		[[nodiscard]] quat Rotation() const;
		[[nodiscard]] vec3 Scale() const;

		void SetLocation(vec3 newValue);
		void SetRotation(quat newValue);
		void SetScale(vec3 newValue);

		void UpdateLocation(vec3 deltaValue);
		void UpdateRotation(quat deltaValue);
		void UpdateScale(vec3 deltaValue);

		void SetParent(Transform* newParent, Transform* before = nullptr);
		void ForEachChild(const IterationFunc& iteration) const;

	private:
		[[nodiscard]] mat4 LocalToParent() const;
		[[nodiscard]] mat4 ParentToLocal() const;

		void ValidatePointers() const;

	};
}
