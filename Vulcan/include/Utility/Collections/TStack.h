#pragma once

#include <cassert>
#include <initializer_list>

#include "Maths/Alias.h"

using std::initializer_list;

template<typename T, uint32 GROW_SIZE = 16>
class TStack
{
protected:
	T* m_stack;
	int32 m_top;
	int32 m_capacity;

public:
	TStack();
	TStack(const initializer_list<T>& initialData);

	TStack(const TStack& rhs);
	TStack(TStack&& rhs) noexcept;
	virtual ~TStack();

public:
	T& Top();
	const T& Top() const;

	void Push(const T& value);
	T Pop();

	void Clear();

private:
	void Expand();

public:
	bool operator==(const TStack& rhs) const;
	bool operator!=(const TStack& rhs) const;

	TStack& operator=(TStack rhs);
	TStack& operator=(TStack&& rhs) noexcept;

};

template <typename T, uint32 GROW_SIZE>
TStack<T, GROW_SIZE>::TStack()
	: m_stack{ new T[GROW_SIZE] }, m_top{ -1 }, m_capacity{ GROW_SIZE }
{
	
}

template <typename T, uint32 GROW_SIZE>
TStack<T, GROW_SIZE>::TStack(const initializer_list<T>& initialData)
	: m_stack{ new T[GROW_SIZE] }, m_top{ -1 }, m_capacity{ GROW_SIZE }
{
	for (const T& value : initialData)
	{
		Push(value);
	}
}

template <typename T, uint32 GROW_SIZE>
TStack<T, GROW_SIZE>::TStack(const TStack& rhs)
	: m_stack{ new T[rhs.m_capacity] }, m_top{ rhs.m_top }, m_capacity{ rhs.m_capacity }
{
	for (int32 i = 0; i < m_capacity; ++i)
	{
		m_stack[i] = rhs.m_stack[i];
	}
}

template <typename T, uint32 GROW_SIZE>
TStack<T, GROW_SIZE>::TStack(TStack&& rhs) noexcept
	: m_stack{ rhs.m_stack }, m_top{ rhs.m_top }, m_capacity{ rhs.m_capacity }
{
	rhs.m_stack = nullptr;
	rhs.m_capacity = 0;
	rhs.m_top = -1;
}

template <typename T, uint32 GROW_SIZE>
TStack<T, GROW_SIZE>::~TStack()
{
	delete[] m_stack;
	m_stack = nullptr;
}

template <typename T, uint32 GROW_SIZE>
T& TStack<T, GROW_SIZE>::Top()
{
	assert(m_top > -1);
	return m_stack[m_top];
}

template <typename T, uint32 GROW_SIZE>
const T& TStack<T, GROW_SIZE>::Top() const
{
	assert(m_top > -1);
	return m_stack[m_top];
}

template <typename T, uint32 GROW_SIZE>
void TStack<T, GROW_SIZE>::Push(const T& value)
{
	const int newTop = m_top + 1;
	if (newTop >= m_capacity)
	{
		Expand();
	}

	m_stack[newTop] = value;
	m_top++;
}

template <typename T, uint32 GROW_SIZE>
T TStack<T, GROW_SIZE>::Pop()
{
	assert(m_top > -1);

	T topVal = m_stack[m_top];
	m_top--;
	return topVal;
}

template <typename T, uint32 GROW_SIZE>
void TStack<T, GROW_SIZE>::Clear()
{
	m_top = -1;
}

template <typename T, uint32 GROW_SIZE>
void TStack<T, GROW_SIZE>::Expand()
{
	m_stack = static_cast<T*>(realloc(m_stack, m_capacity + GROW_SIZE));
	m_capacity += GROW_SIZE;
}

template <typename T, uint32 GROW_SIZE>
bool TStack<T, GROW_SIZE>::operator==(const TStack& rhs) const
{
	return m_stack == rhs.m_stack && m_top == rhs.m_top && m_capacity == rhs.m_capacity;
}

template <typename T, uint32 GROW_SIZE>
bool TStack<T, GROW_SIZE>::operator!=(const TStack& rhs) const
{
	return m_stack != rhs.m_stack || m_top != rhs.m_top || m_capacity != rhs.m_capacity;
}

template <typename T, uint32 GROW_SIZE>
TStack<T, GROW_SIZE>& TStack<T, GROW_SIZE>::operator=(TStack rhs)
{
	std::swap(m_stack, rhs.m_stack);
	std::swap(m_capacity, rhs.m_capacity);
	std::swap(m_top, rhs.m_top);
	return *this;
}

template <typename T, uint32 GROW_SIZE>
TStack<T, GROW_SIZE>& TStack<T, GROW_SIZE>::operator=(TStack&& rhs) noexcept
{
	if (this == &rhs)
	{
		return *this;
	}

	m_stack = rhs.m_stack;
	m_capacity = rhs.m_capacity;
	m_top = rhs.m_top;

	rhs.m_stack = nullptr;
	rhs.m_capacity = 0;
	rhs.m_top = -1;

	return *this;
}
