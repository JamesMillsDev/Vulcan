#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <ranges>

#include "Maths/Alias.h"

using std::initializer_list;
using std::ranges::view_interface;

namespace Tempest
{
	template<typename T, int64 GROWTH = 16>
	class TList
	{
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
		int64 m_capacity;
		int64 m_count;

		T* m_data;

	public:
		TList();
		TList(const initializer_list<T>& initialData);

		TList(const TList& rhs);
		TList(TList&& rhs) noexcept;
		~TList();

	public:
		void Add(T item);
		void AddRange(const initializer_list<T>& items);

		void Insert(T item, int64 index);

		void Remove(T item);
		void RemoveAt(int64 index);

		void Clear();

		[[nodiscard]] bool Contains(T item);
		[[nodiscard]] bool IsEmpty() const;

		[[nodiscard]] int64 Find(T item) const;
		[[nodiscard]] int64 Capacity() const;
		[[nodiscard]] int64 Count() const;

		T At(int64 index) const;
		T Front() const;
		T Back() const;

		void SetData(T* data, uint64 length);
		T* Data() noexcept;
		const T* Data() const noexcept;

		void Resize(uint64 newSize);

		Iterator begin();
		Iterator end();
		[[nodiscard]] uint64 size();
		[[nodiscard]] bool empty();

		Iterator begin() const;
		Iterator end() const;
		[[nodiscard]] uint64 size() const;
		[[nodiscard]] bool empty() const;

	private:
		void Expand();

	public:
		TList& operator=(TList rhs);
		TList& operator=(TList&& rhs) noexcept;

		T& operator[](int64 index);
		const T& operator[](int64 index) const;

		static_assert(std::bidirectional_iterator<Iterator>);

	};

	template<typename T, typename... U>
	TList(T, U...) -> TList<T, 1 + sizeof...(U)>;

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::Iterator::Iterator()
		: m_ptr{ nullptr }
	{

	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::Iterator::Iterator(pointer ptr)
		: m_ptr{ ptr }
	{

	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::template Iterator::reference TList<T, GROWTH>::Iterator::operator*() const
	{
		return *m_ptr;
	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::template Iterator::pointer TList<T, GROWTH>::Iterator::operator->()
	{
		return m_ptr;
	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::template Iterator& TList<T, GROWTH>::Iterator::operator++()
	{
		++m_ptr;
		return *this;
	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::template Iterator& TList<T, GROWTH>::Iterator::operator--()
	{
		--m_ptr;
		return *this;
	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::template Iterator TList<T, GROWTH>::Iterator::operator++(int)
	{
		Iterator tmp = *this;
		++(*this);
		return tmp;
	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::template Iterator TList<T, GROWTH>::Iterator::operator--(int)
	{
		Iterator tmp = *this;
		--(*this);
		return tmp;
	}

	template <typename T, int64 GROWTH>
	bool TList<T, GROWTH>::Iterator::operator==(const Iterator& rhs) const
	{
		return m_ptr == rhs.m_ptr;
	}

	template <typename T, int64 GROWTH>
	bool TList<T, GROWTH>::Iterator::operator!=(const Iterator& rhs) const
	{
		return m_ptr != rhs.m_ptr;
	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::template Iterator TList<T, GROWTH>::View::begin() const
	{
		return std::ranges::begin(value);
	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::template Iterator TList<T, GROWTH>::View::end() const
	{
		return std::ranges::end(value);
	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::TList()
		: m_capacity{ GROWTH }, m_count{ 0 }, m_data{ new T[m_capacity]{} }
	{

	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::TList(const initializer_list<T>& initialData)
		: m_capacity{ GROWTH }, m_count{ 0 }, m_data{ new T[m_capacity]{} }
	{
		for (uint64 i = 0; i < initialData.size(); ++i)
		{
			Add(*(initialData.begin() + i));
		}
	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::TList(const TList& rhs)
		: m_capacity{ rhs.m_capacity }, m_count{ rhs.m_count }, m_data{ new T[m_capacity]{} }
	{
		for (int64 i = 0; i < m_capacity; ++i)
		{
			m_data[i] = rhs.m_data[i];
		}
	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::TList(TList&& rhs) noexcept
		: m_capacity{ rhs.m_capacity }, m_count{ rhs.m_count }, m_data{ rhs.m_data }
	{
		rhs.m_capacity = 0;
		rhs.m_count = 0;
		rhs.m_data = nullptr;
	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::~TList()
	{
		m_capacity = 0;
		m_count = 0;
		delete[] m_data;
	}

	template <typename T, int64 GROWTH>
	void TList<T, GROWTH>::Add(T item)
	{
		if (m_count + 1 >= m_capacity)
		{
			Expand();
		}

		m_data[m_count++] = item;
	}

	template <typename T, int64 GROWTH>
	void TList<T, GROWTH>::AddRange(const initializer_list<T>& items)
	{
		for (const T& item : items)
		{
			Add(item);
		}
	}

	template <typename T, int64 GROWTH>
	void TList<T, GROWTH>::Insert(T item, int64 index)
	{
		if (m_count + 1 >= m_capacity)
		{
			Expand();
		}

		memmove(&m_data[index + 1], &m_data[index], (m_capacity - index - 1) * sizeof(T));
		m_data[index] = item;
		m_count++;
	}

	template <typename T, int64 GROWTH>
	void TList<T, GROWTH>::Remove(T item)
	{
		int64 index = Find(item);
		if (index == -1)
		{
			return;
		}

		RemoveAt(index);
	}

	template <typename T, int64 GROWTH>
	void TList<T, GROWTH>::RemoveAt(int64 index)
	{
		if (m_count == 0)
		{
			return;
		}

		memmove(&m_data[index], &m_data[index + 1], (m_count - index - 1) * sizeof(T));
		m_count--;
	}

	template <typename T, int64 GROWTH>
	void TList<T, GROWTH>::Clear()
	{
		m_count = 0;
	}

	template <typename T, int64 GROWTH>
	bool TList<T, GROWTH>::Contains(T item)
	{
		return Find(item) != -1;
	}

	template <typename T, int64 GROWTH>
	bool TList<T, GROWTH>::IsEmpty() const
	{
		return m_count == 0;
	}

	template <typename T, int64 GROWTH>
	int64 TList<T, GROWTH>::Find(T item) const
	{
		for (int64 i = 0; i < m_count; ++i)
		{
			if (m_data[i] == item)
			{
				return i;
			}
		}

		return -1;
	}

	template <typename T, int64 GROWTH>
	int64 TList<T, GROWTH>::Capacity() const
	{
		return m_capacity;
	}

	template <typename T, int64 GROWTH>
	int64 TList<T, GROWTH>::Count() const
	{
		return m_count;
	}

	template <typename T, int64 GROWTH>
	T TList<T, GROWTH>::At(int64 index) const
	{
		return this->operator[](index);
	}

	template <typename T, int64 GROWTH>
	T TList<T, GROWTH>::Front() const
	{
		return At(0);
	}

	template <typename T, int64 GROWTH>
	T TList<T, GROWTH>::Back() const
	{
		return At(m_count - 1);
	}

	template <typename T, int64 GROWTH>
	void TList<T, GROWTH>::SetData(T* data, uint64 length)
	{
		Resize(length);
		std::memcpy(m_data, data, length);
	}

	template <typename T, int64 GROWTH>
	T* TList<T, GROWTH>::Data() noexcept
	{
		return m_data;
	}

	template <typename T, int64 GROWTH>
	const T* TList<T, GROWTH>::Data() const noexcept
	{
		return m_data;
	}

	template <typename T, int64 GROWTH>
	void TList<T, GROWTH>::Resize(uint64 newSize)
	{
		uint64 copySize = newSize < m_capacity ? newSize : m_capacity;

		T* newData = new T[newSize];
		std::copy(m_data, m_data + copySize, newData);
		delete[] m_data;
		m_data = newData;

		m_capacity = newSize;
		m_count = m_capacity;
	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::template Iterator TList<T, GROWTH>::begin()
	{
		return Iterator{ &m_data[0] };
	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::template Iterator TList<T, GROWTH>::end()
	{
		return Iterator{ &m_data[m_count] };
	}

	template <typename T, int64 GROWTH>
	uint64 TList<T, GROWTH>::size()
	{
		return m_count;
	}

	template <typename T, int64 GROWTH>
	bool TList<T, GROWTH>::empty()
	{
		return m_count == 0;
	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::template Iterator TList<T, GROWTH>::begin() const
	{
		return Iterator{ &m_data[0] };
	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>::template Iterator TList<T, GROWTH>::end() const
	{
		return Iterator{ &m_data[m_count] };
	}

	template <typename T, int64 GROWTH>
	uint64 TList<T, GROWTH>::size() const
	{
		return static_cast<uint64>(Count());
	}

	template <typename T, int64 GROWTH>
	bool TList<T, GROWTH>::empty() const
	{
		return IsEmpty();
	}

	template <typename T, int64 GROWTH>
	void TList<T, GROWTH>::Expand()
	{
		T* newData = new T[m_capacity + GROWTH];
		std::copy(m_data, m_data + m_capacity, newData);
		delete[] m_data;
		m_data = newData;
		m_capacity += GROWTH;
	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>& TList<T, GROWTH>::operator=(TList rhs)
	{
		std::swap(m_capacity, rhs.m_capacity);
		std::swap(m_count, rhs.m_count);
		std::swap(m_data, rhs.m_data);

		return *this;
	}

	template <typename T, int64 GROWTH>
	TList<T, GROWTH>& TList<T, GROWTH>::operator=(TList&& rhs) noexcept
	{
		if (this == &rhs)
		{
			return *this;
		}

		m_capacity = rhs.m_capacity;
		m_count = rhs.m_count;
		m_data = rhs.m_data;

		rhs.m_capacity = 0;
		rhs.m_count = 0;
		rhs.m_data = nullptr;

		return *this;
	}

	template <typename T, int64 GROWTH>
	T& TList<T, GROWTH>::operator[](int64 index)
	{
		assert(index < m_capacity && index >= 0);
		return m_data[index];
	}

	template <typename T, int64 GROWTH>
	const T& TList<T, GROWTH>::operator[](int64 index) const
	{
		assert(index < m_count && index >= 0);
		return m_data[index];
	}
}