#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <ranges>

#include "Maths/Alias.h"

using std::initializer_list;
using std::ranges::view_interface;

namespace Vulcan
{
	template<typename T, uint64 LENGTH>
	class TArray
	{
		static_assert(std::is_object_v<T>, "The C++ Standard forbids containers of non-object types "
			"because of [container.requirements].");

	public:
		struct Iterator
		{
			using iterator_category = std::bidirectional_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = T;
			using pointer = value_type*;
			using reference = value_type&;

		private:
			pointer m_ptr;

		public:
			Iterator();
			Iterator(pointer ptr);

		public:
			reference operator*() const;
			pointer operator->();

			Iterator& operator++();
			Iterator& operator--();

			Iterator operator++(int);
			Iterator operator--(int);

			bool operator==(const Iterator& rhs) const;
			bool operator!=(const Iterator& rhs) const;

		};

		struct View : view_interface<View>
		{
		public:
			T value;

		public:
			Iterator begin() const;
			Iterator end() const;

		};

	private:
		T* m_elements;

	public:
		TArray();
		TArray(const initializer_list<T>& initialData);

		TArray(const TArray& rhs);
		TArray(TArray&& rhs) noexcept;
		~TArray();

	public:
		T* Data() noexcept;
		const T* Data() const noexcept;

		uint32 Count();
		uint32 Count() const;

		Iterator begin();
		Iterator end();
		[[nodiscard]] uint64 size();
		[[nodiscard]] bool empty();

		Iterator begin() const;
		Iterator end() const;
		[[nodiscard]] uint64 size() const;
		[[nodiscard]] bool empty() const;

	public:
		T& operator[](uint64 index);
		const T& operator[](uint64 index) const;

		static_assert(std::bidirectional_iterator<Iterator>);

	};

	template<typename T, typename... U>
	TArray(T, U...) -> TArray<T, 1 + sizeof...(U)>;

	template <typename T, uint64 LENGTH>
	TArray<T, LENGTH>::Iterator::Iterator()
		: m_ptr{ nullptr }
	{

	}

	template <typename T, uint64 LENGTH>
	TArray<T, LENGTH>::Iterator::Iterator(pointer ptr)
		: m_ptr{ ptr }
	{

	}

	template <typename T, uint64 LENGTH>
	typename TArray<T, LENGTH>::Iterator::reference TArray<T, LENGTH>::Iterator::operator*() const
	{
		return *m_ptr;
	}

	template <typename T, uint64 LENGTH>
	typename TArray<T, LENGTH>::Iterator::pointer TArray<T, LENGTH>::Iterator::operator->()
	{
		return m_ptr;
	}

	template <typename T, uint64 LENGTH>
	typename TArray<T, LENGTH>::Iterator& TArray<T, LENGTH>::Iterator::operator++()
	{
		++m_ptr;
		return *this;
	}

	template <typename T, uint64 LENGTH>
	typename TArray<T, LENGTH>::Iterator& TArray<T, LENGTH>::Iterator::operator--()
	{
		--m_ptr;
		return *this;
	}

	template <typename T, uint64 LENGTH>
	typename TArray<T, LENGTH>::Iterator TArray<T, LENGTH>::Iterator::operator++(int)
	{
		Iterator tmp = *this;
		++(*this);
		return tmp;
	}

	template <typename T, uint64 LENGTH>
	typename TArray<T, LENGTH>::Iterator TArray<T, LENGTH>::Iterator::operator--(int)
	{
		Iterator tmp = *this;
		--(*this);
		return tmp;
	}

	template <typename T, uint64 LENGTH>
	bool TArray<T, LENGTH>::Iterator::operator==(const Iterator& rhs) const
	{
		return m_ptr == rhs.m_ptr;
	}

	template <typename T, uint64 LENGTH>
	bool TArray<T, LENGTH>::Iterator::operator!=(const Iterator& rhs) const
	{
		return m_ptr != rhs.m_ptr;
	}

	template <typename T, uint64 LENGTH>
	typename TArray<T, LENGTH>::Iterator TArray<T, LENGTH>::View::begin() const
	{
		return std::ranges::begin(value);
	}

	template <typename T, uint64 LENGTH>
	typename TArray<T, LENGTH>::Iterator TArray<T, LENGTH>::View::end() const
	{
		return std::ranges::end(value);
	}

	template <typename T, uint64 LENGTH>
	TArray<T, LENGTH>::TArray()
		: m_elements{ new T[LENGTH]{} }
	{

	}

	template <typename T, uint64 LENGTH>
	TArray<T, LENGTH>::TArray(const initializer_list<T>& initialData)
		: m_elements{ new T[LENGTH] }
	{
		for (uint64 i = 0; i < LENGTH && i < initialData.size(); ++i)
		{
			m_elements[i] = *(initialData.begin() + i);
		}
	}

	template <typename T, uint64 LENGTH>
	TArray<T, LENGTH>::TArray(const TArray& rhs)
		: m_elements{ new T[LENGTH] }
	{
		for (uint64 i = 0; i < LENGTH; ++i)
		{
			m_elements[i] = rhs.m_elements[i];
		}
	}

	template <typename T, uint64 LENGTH>
	TArray<T, LENGTH>::TArray(TArray&& rhs) noexcept
		: m_elements{ rhs.m_elements }
	{
		rhs.m_elements = nullptr;
	}

	template <typename T, uint64 LENGTH>
	TArray<T, LENGTH>::~TArray()
	{
		delete[] m_elements;
	}

	template <typename T, uint64 LENGTH>
	T* TArray<T, LENGTH>::Data() noexcept
	{
		return m_elements;
	}

	template <typename T, uint64 LENGTH>
	const T* TArray<T, LENGTH>::Data() const noexcept
	{
		return m_elements;
	}

	template <typename T, uint64 LENGTH>
	uint32 TArray<T, LENGTH>::Count()
	{
		return static_cast<uint32>(LENGTH);
	}

	template <typename T, uint64 LENGTH>
	uint32 TArray<T, LENGTH>::Count() const
	{
		return static_cast<uint32>(LENGTH);
	}

	template <typename T, uint64 LENGTH>
	typename TArray<T, LENGTH>::Iterator TArray<T, LENGTH>::begin()
	{
		return Iterator{ &m_elements[0] };
	}

	template <typename T, uint64 LENGTH>
	typename TArray<T, LENGTH>::Iterator TArray<T, LENGTH>::end()
	{
		return Iterator{ &m_elements[LENGTH] };
	}

	template <typename T, uint64 LENGTH>
	uint64 TArray<T, LENGTH>::size()
	{
		return LENGTH;
	}

	template <typename T, uint64 LENGTH>
	bool TArray<T, LENGTH>::empty()
	{
		return false;
	}

	template <typename T, uint64 LENGTH>
	typename TArray<T, LENGTH>::Iterator TArray<T, LENGTH>::begin() const
	{
		return Iterator{ &m_elements[0] };
	}

	template <typename T, uint64 LENGTH>
	typename TArray<T, LENGTH>::Iterator TArray<T, LENGTH>::end() const
	{
		return Iterator{ &m_elements[LENGTH] };
	}

	template <typename T, uint64 LENGTH>
	uint64 TArray<T, LENGTH>::size() const
	{
		return LENGTH;
	}

	template <typename T, uint64 LENGTH>
	bool TArray<T, LENGTH>::empty() const
	{
		return false;
	}

	template <typename T, uint64 LENGTH>
	T& TArray<T, LENGTH>::operator[](uint64 index)
	{
		assert(index < LENGTH);
		return m_elements[index];
	}

	template <typename T, uint64 LENGTH>
	const T& TArray<T, LENGTH>::operator[](uint64 index) const
	{
		assert(index < LENGTH);
		return m_elements[index];
	}
}