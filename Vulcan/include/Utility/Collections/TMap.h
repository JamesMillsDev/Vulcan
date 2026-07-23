#pragma once

#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <stdexcept>

#include "Object.h"
#include "Maths/Alias.h"

using std::hash;
using std::initializer_list;
using std::runtime_error;
using std::ranges::view_interface;

template<typename KEY, typename VALUE>
struct TKeyValuePair
{
	KEY key;
	VALUE value;
};

template<typename KEY, typename VALUE>
class TMapEntry
{
public:
	TMapEntry* next;

private:
	TKeyValuePair<KEY, VALUE> m_keyValuePair;

public:
	TMapEntry(KEY key, VALUE value);
	TMapEntry(const TMapEntry& rhs);

public:
	KEY Key() const;
	VALUE& Value();
	const VALUE& Value() const;

public:
	VALUE& operator*();
	const VALUE& operator*() const;

	TMapEntry& operator=(TMapEntry rhs);

};

template<typename KEY, typename VALUE, int64 GROWTH = 16, float LOAD_THRESHOLD = .8f>
class TMap
{
public:
	struct Iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = TMapEntry<KEY, VALUE>*;
		using pointer = value_type*;
		using reference = value_type&;

	private:
		TMapEntry<KEY, VALUE>** m_buckets;
		uint64 m_capacity;
		uint64 m_bucketIndex;
		TMapEntry<KEY, VALUE>* m_current;

	public:
		Iterator();
		Iterator(TMapEntry<KEY, VALUE>** buckets, uint64 capacity, uint64 bucketIndex, TMapEntry<KEY, VALUE>* current);

	private:
		void SkipEmptyBuckets();

	public:
		value_type operator*() const;
		value_type operator->() const;

		Iterator& operator++();

		Iterator operator++(int);

		bool operator==(const Iterator& rhs) const;
		bool operator!=(const Iterator& rhs) const;

	};

	struct View : view_interface<View>
	{
	public:
		TMapEntry<KEY, VALUE> value;

	public:
		Iterator begin() const;
		Iterator end() const;

	};

private:
	uint64 m_capacity;
	uint64 m_count;
	uint64 m_totalEntryCount;

	TMapEntry<KEY, VALUE>** m_buckets;

public:
	TMap();
	TMap(const initializer_list<TKeyValuePair<KEY, VALUE>>& initialData);

	TMap(const TMap& rhs);
	TMap(TMap&& rhs) noexcept;
	~TMap();

public:
	void Add(KEY key, VALUE value);
	void RemoveFirst(KEY key);
	void RemoveAllFor(KEY key);

	void Clear();

	[[nodiscard]] bool ContainsKey(KEY key);
	[[nodiscard]] bool IsEmpty() const;

	[[nodiscard]] uint64 Capacity() const;
	[[nodiscard]] uint64 Count() const;
	[[nodiscard]] float Load() const;

	TMapEntry<KEY, VALUE>* Data() noexcept;
	const TMapEntry<KEY, VALUE>* Data() const noexcept;

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
	uint64 IndexFor(KEY key, uint64 overrideSize = UINT64_MAX) const;
	uint64 Hash(KEY key) const;

	void Swap(TMap& other) noexcept;

public:
	TMap& operator=(TMap rhs);
	TMap& operator=(TMap&& rhs) noexcept;

	TMapEntry<KEY, VALUE>& operator[](KEY key);
	const TMapEntry<KEY, VALUE>& operator[](KEY key) const;

};

template <typename KEY, typename VALUE>
TMapEntry<KEY, VALUE>::TMapEntry(KEY key, VALUE value)
	: next{ nullptr }, m_keyValuePair{ key, value }
{

}

template <typename KEY, typename VALUE>
TMapEntry<KEY, VALUE>::TMapEntry(const TMapEntry& rhs)
	: next{ nullptr }, m_keyValuePair{ rhs.m_keyValuePair }
{
	if (rhs.next != nullptr)
	{
		next = new TMapEntry{ rhs.next->Key(), rhs.next->Value() };
	}
}

template <typename KEY, typename VALUE>
KEY TMapEntry<KEY, VALUE>::Key() const
{
	return m_keyValuePair.key;
}

template <typename KEY, typename VALUE>
VALUE& TMapEntry<KEY, VALUE>::Value()
{
	return m_keyValuePair.value;
}

template <typename KEY, typename VALUE>
const VALUE& TMapEntry<KEY, VALUE>::Value() const
{
	return m_keyValuePair.value;
}

template <typename KEY, typename VALUE>
VALUE& TMapEntry<KEY, VALUE>::operator*()
{
	return m_keyValuePair.value;
}

template <typename KEY, typename VALUE>
const VALUE& TMapEntry<KEY, VALUE>::operator*() const
{
	return m_keyValuePair.value;
}

template <typename KEY, typename VALUE>
TMapEntry<KEY, VALUE>& TMapEntry<KEY, VALUE>::operator=(TMapEntry rhs)
{
	std::swap(m_keyValuePair, rhs.m_keyValuePair);
	std::swap(next, rhs.next);

	return *this;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator::Iterator()
	: m_buckets{ nullptr }, m_capacity{ 0 }, m_bucketIndex{ 0 }, m_current{ nullptr }
{}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator::Iterator(TMapEntry<KEY, VALUE>** buckets, uint64 capacity, uint64 bucketIndex, TMapEntry<KEY, VALUE>* current)
	: m_buckets{ buckets }, m_capacity{ capacity }, m_bucketIndex{ bucketIndex }, m_current{ current }
{
	SkipEmptyBuckets();
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
void TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator::SkipEmptyBuckets()
{
	while (m_current == nullptr && m_bucketIndex < m_capacity)
	{
		++m_bucketIndex;
		if (m_bucketIndex < m_capacity)
		{
			m_current = m_buckets[m_bucketIndex];
		}
	}
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
typename TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator::value_type TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator::operator*() const
{
	return m_current;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
typename TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator::value_type TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator::operator->() const
{
	return m_current;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
typename TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator& TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator::operator++()
{
	m_current = m_current->next;
	SkipEmptyBuckets();
	return *this;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
typename TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator::operator++(int)
{
	Iterator tmp = *this;
	++(*this);
	return tmp;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
bool TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator::operator==(const Iterator& rhs) const
{
	return m_current == rhs.m_current && m_bucketIndex == rhs.m_bucketIndex;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
bool TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator::operator!=(const Iterator& rhs) const
{
	return m_current != rhs.m_current || m_bucketIndex != rhs.m_bucketIndex;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
typename TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::View::begin() const
{
	return std::ranges::begin(value);
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
typename TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::View::end() const
{
	return std::ranges::end(value);
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::TMap()
	: m_capacity{ GROWTH }, m_count{ 0 }, m_totalEntryCount{ 0 }, m_buckets{ new TMapEntry<KEY, VALUE>*[m_capacity] }
{
	for (uint64 i = 0; i < m_capacity; ++i)
	{
		m_buckets[i] = nullptr;
	}
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::TMap(const initializer_list<TKeyValuePair<KEY, VALUE>>& initialData)
	: m_capacity{ GROWTH }, m_count{ 0 }, m_totalEntryCount{ 0 }, m_buckets{ new TMapEntry<KEY, VALUE>*[m_capacity] }
{
	for (uint64 i = 0; i < m_capacity; ++i)
	{
		m_buckets[i] = nullptr;
	}

	for (const TKeyValuePair<KEY, VALUE>& kvp : initialData)
	{
		Add(kvp.key, kvp.value);
	}
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::TMap(const TMap& rhs)
	: m_capacity{ rhs.m_capacity }, m_count{ rhs.m_count }, m_totalEntryCount{ rhs.m_totalEntryCount },
	m_buckets{ new TMapEntry<KEY, VALUE>*[m_capacity] }
{
	for (uint64 i = 0; i < m_capacity; ++i)
	{
		if (rhs.m_buckets[i] == nullptr)
		{
			m_buckets[i] = nullptr;
			continue;
		}

		m_buckets[i] = new TMapEntry<KEY, VALUE>{ rhs.m_buckets[i]->Key(), rhs.m_buckets[i]->Value() };

		TMapEntry<KEY, VALUE>* currentNew = m_buckets[i];
		TMapEntry<KEY, VALUE>* currentOld = rhs.m_buckets[i]->next;

		while (currentOld != nullptr)
		{
			currentNew->next = new TMapEntry<KEY, VALUE>{ currentOld->Key(), currentOld->Value() };
			currentNew = currentNew->next;
			currentOld = currentOld->next;
		}
	}
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::TMap(TMap&& rhs) noexcept
	: m_capacity{ rhs.m_capacity }, m_count{ rhs.m_count }, m_totalEntryCount{ rhs.m_totalEntryCount }, m_buckets{ rhs.m_buckets }
{
	rhs.m_capacity = 0;
	rhs.m_count = 0;
	rhs.m_totalEntryCount = 0;
	rhs.m_buckets = nullptr;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::~TMap()
{
	Clear();

	m_capacity = 0;
	m_count = 0;
	m_totalEntryCount = 0;
	delete[] m_buckets;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
void TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Add(KEY key, VALUE value)
{
	// Create a new hash entry and get the relevant index
	TMapEntry<KEY, VALUE>* newEntry = new TMapEntry<KEY, VALUE>{ key, value };
	const int64 index = IndexFor(key);

	// Add the entry into the array and assign the current entry to the next of the new ones
	newEntry->next = m_buckets[index];
	m_buckets[index] = newEntry;
	m_totalEntryCount++;

	// Increment the count if this is the first item added
	if (m_buckets[index]->next == nullptr)
	{
		m_count++;
	}

	// If the load factor of the map has surpassed the threshold, resize
	if (Load() > LOAD_THRESHOLD)
	{
		Resize(m_capacity + GROWTH);
	}
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
void TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::RemoveFirst(KEY key)
{
	const int64 index = IndexFor(key);

	// If there is a valid entry at this index
	if (m_buckets[index] != nullptr)
	{
		// Get the first and second item at this index
		TMapEntry<KEY, VALUE>* previous = nullptr;
		TMapEntry<KEY, VALUE>* current = m_buckets[index];
		TMapEntry<KEY, VALUE>* next = current->next;

		// Move to the next item whilst the current does not have the same key
		while (current->Key() != key)
		{
			previous = current;
			current = next;
			next = current->next;
		}

		// Make sure we still have a current
		if (current != nullptr)
		{
			if (previous != nullptr)
			{
				previous->next = next;
			}
			else
			{
				m_buckets[index] = next;
			}

			// Delete the current entry and decrement the count
			delete current;
			m_totalEntryCount--;

			if (m_buckets[index] == nullptr)
			{
				m_count--;
			}
		}
	}
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
void TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::RemoveAllFor(KEY key)
{
	const int64 index = IndexFor(key);

	// If there is a valid entry at this index
	while (m_buckets[index] != nullptr)
	{
		// Get the next entry at this index
		TMapEntry<KEY, VALUE>* next = m_buckets[index]->next;

		// Delete the current entry and reassign it to the next one
		delete m_buckets[index];
		m_buckets[index] = next;

		// Decrement the entry count
		m_totalEntryCount--;

		if (m_buckets[index] == nullptr)
		{
			m_count--;
		}
	}
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
void TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Clear()
{
	for (uint64 i = 0; i < m_capacity; ++i)
	{
		// If there is a valid entry at this index
		while (m_buckets[i] != nullptr)
		{
			// Get the next entry at this index
			TMapEntry<KEY, VALUE>* next = m_buckets[i]->next;

			// Delete the current entry and reassign it to the next one
			delete m_buckets[i];
			m_buckets[i] = next;

			// Decrement the entry count
			m_totalEntryCount--;

			if (m_buckets[i] == nullptr)
			{
				m_count--;
			}
		}
	}
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
bool TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::ContainsKey(KEY key)
{
	// Get the first entry for this index
	const int64 index = IndexFor(key);
	TMapEntry<KEY, VALUE>* entry = m_buckets[index];

	// While this entry isn't null and not the correct key, move to the next
	while (entry != nullptr && entry->Key() != key)
	{
		entry = entry->next;
	}

	return entry != nullptr;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
bool TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::IsEmpty() const
{
	return m_totalEntryCount == 0;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
uint64 TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Capacity() const
{
	return m_capacity;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
uint64 TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Count() const
{
	return m_totalEntryCount;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
float TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Load() const
{
	return static_cast<float>(m_count) / static_cast<float>(m_capacity);
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
TMapEntry<KEY, VALUE>* TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Data() noexcept
{
	return m_buckets;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
const TMapEntry<KEY, VALUE>* TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Data() const noexcept
{
	return m_buckets;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
void TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Resize(uint64 newSize)
{
	// Generate a new array pointer
	TMapEntry<KEY, VALUE>** newMap = new TMapEntry<KEY, VALUE>* [newSize];

	// Initialise new array to all nullptr
	for (uint64 i = 0; i < newSize; ++i)
	{
		newMap[i] = nullptr;
	}

	// Iterate over the old array
	for (uint64 i = 0; i < m_capacity; ++i)
	{
		// Iterate over every entry at this index
		TMapEntry<KEY, VALUE>* entry = m_buckets[i];
		while (entry != nullptr)
		{
			// Get the next entry and the new hash index
			TMapEntry<KEY, VALUE>* next = entry->next;
			const uint64 index = IndexFor(entry->Key(), newSize);

			// Set the entry for this index and the next entry
			entry->next = newMap[index];
			newMap[index] = entry;

			// Update the current entry to the next
			entry = next;
		}
	}

	// Delete old array and override with new one
	delete[] m_buckets;
	m_buckets = newMap;
	m_capacity = newSize;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
typename TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::begin()
{
	return Iterator{ m_buckets, m_capacity, 0, m_capacity > 0 ? m_buckets[0] : nullptr };
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
typename TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::end()
{
	return Iterator{ m_buckets, m_capacity, m_capacity, nullptr };
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
uint64 TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::size()
{
	return m_count;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
bool TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::empty()
{
	return m_count == 0;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
typename TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::begin() const
{
	return Iterator{ m_buckets, m_capacity, 0, m_capacity > 0 ? m_buckets[0] : nullptr };
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
typename TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Iterator TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::end() const
{
	return Iterator{ m_buckets, m_capacity, m_capacity, nullptr };
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
uint64 TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::size() const
{
	return m_count;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
bool TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::empty() const
{
	return m_count == 0;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
uint64 TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::IndexFor(KEY key, uint64 overrideSize) const
{
	if (overrideSize != UINT64_MAX)
	{
		return Hash(key) % overrideSize;
	}

	return Hash(key) % m_capacity;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
uint64 TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Hash(KEY key) const
{
	return hash<KEY>{}(key);
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
void TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::Swap(TMap& other) noexcept
{
	std::swap(m_capacity, other.m_capacity);
	std::swap(m_count, other.m_count);
	std::swap(m_totalEntryCount, other.m_totalEntryCount);
	std::swap(m_buckets, other.m_buckets);
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>& TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::operator=(TMap rhs)
{
	Swap(rhs);
	return *this;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>& TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::operator=(TMap&& rhs) noexcept
{
	if (this == &rhs)
	{
		return *this;
	}

	Clear();
	m_capacity = rhs.m_capacity;
	m_count = rhs.m_count;
	m_totalEntryCount = rhs.m_totalEntryCount;
	m_buckets = rhs.m_buckets;

	rhs.m_capacity = 0;
	rhs.m_count = 0;
	rhs.m_totalEntryCount = 0;
	rhs.m_buckets = nullptr;

	return *this;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
TMapEntry<KEY, VALUE>& TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::operator[](KEY key)
{
	const uint64 index = IndexFor(key);

	TMapEntry<KEY, VALUE>* entry = m_buckets[index];
	while (entry != nullptr && entry->Key() != key)
	{
		entry = entry->next;
	}

	return *entry;
}

template <typename KEY, typename VALUE, int64 GROWTH, float LOAD_THRESHOLD>
const TMapEntry<KEY, VALUE>& TMap<KEY, VALUE, GROWTH, LOAD_THRESHOLD>::operator[](KEY key) const
{
	const uint64 index = IndexFor(key);

	TMapEntry<KEY, VALUE>* entry = m_buckets[index];
	while (entry != nullptr && entry->Key() != key)
	{
		entry = entry->next;
	}

	return *entry;
}
