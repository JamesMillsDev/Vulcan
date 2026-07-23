#pragma once

#include <cassert>
#include <functional>
#include <initializer_list>
#include <type_traits>

#include "Maths/Alias.h"

using std::function;
using std::initializer_list;
using std::less;

namespace Vulcan
{
	template<typename T, class PRED = less<T>>
	class TSet
	{
	public:
		enum class ENodeColor : uint8
		{
			Red = 0,
			Black = 1,
		};

		struct Node
		{
		public:
			T data = T{};
			ENodeColor color = ENodeColor::Red;

			Node* left = nullptr;
			Node* right = nullptr;
			Node* parent = nullptr;

		public:
			~Node();

		};

	private:
		using InOrderFnc = std::function<void(Node*)>;

	private:
		Node* m_root;

	public:
		TSet();
		TSet(const initializer_list<T>& initialData);

		TSet(const TSet& rhs);
		TSet(TSet&& rhs) noexcept;
		~TSet();

	public:
		[[nodiscard]] Node* Root() const;

		void Insert(const T& data);
		[[nodiscard]] bool Contains(const T& data) const;
		void ForEach(const InOrderFnc& predicate) const;

		uint32 Count() const;

	private:
		void InOrder(const InOrderFnc& predicate, Node* node) const;

		void RotateLeft(Node* x);
		void RotateRight(Node* x);
		void FixInsert(Node* node);

		void CloneTree(Node*& modified, Node* toCopy);

	public:
		TSet& operator=(TSet rhs);
		TSet& operator=(TSet&& rhs) noexcept;

	};

	template <typename T, class PRED>
	TSet<T, PRED>::Node::~Node()
	{
		delete left;
		left = nullptr;

		delete right;
		right = nullptr;
	}

	template <typename T, class PRED>
	TSet<T, PRED>::TSet()
		: m_root{ nullptr }
	{}

	template <typename T, class PRED>
	TSet<T, PRED>::TSet(const initializer_list<T>& initialData)
		: m_root{ nullptr }
	{
		for (T data : initialData)
		{
			Insert(data);
		}
	}

	template <typename T, class PRED>
	TSet<T, PRED>::TSet(const TSet& rhs)
		: m_root{ new Node }
	{
		CloneTree(m_root, rhs.m_root);
	}

	template <typename T, class PRED>
	TSet<T, PRED>::TSet(TSet&& rhs) noexcept
		: m_root{ rhs.m_root }
	{
		rhs.m_root = nullptr;
	}

	template <typename T, class PRED>
	TSet<T, PRED>::~TSet()
	{
		delete m_root;
	}

	template <typename T, class PRED>
	typename TSet<T, PRED>::Node* TSet<T, PRED>::Root() const
	{
		return m_root;
	}

	template <typename T, class PRED>
	void TSet<T, PRED>::Insert(const T& data)
	{
		assert(!Contains(data));

		Node* node = new Node
		{
			.data = data
		};

		Node* parent = nullptr;
		Node* current = m_root;

		PRED pred;

		// Step 1: Standard BST insert
		while (current != nullptr)
		{
			parent = current;
			current = pred(node->data, current->data) ? current->left : current->right;
		}

		node->parent = parent;

		if (parent == nullptr)
		{
			m_root = node;
		}
		else if (pred(node->data, parent->data))
		{
			parent->left = node;
		}
		else
		{
			parent->right = node;
		}

		// Step 2: Handle edge cases and fix RB properties
		if (node->parent == nullptr)
		{
			node->color = ENodeColor::Black;
			return;
		}

		if (node->parent->parent == nullptr)
		{
			return;
		}

		FixInsert(node);
	}

	template <typename T, class PRED>
	bool TSet<T, PRED>::Contains(const T& data) const
	{
		bool flag = false;
		ForEach([data, &flag](Node* node)
			{
				flag |= node->data == data;
			});

		return flag;
	}

	template <typename T, class PRED>
	void TSet<T, PRED>::ForEach(const InOrderFnc& predicate) const
	{
		InOrder(predicate, m_root);
	}

	template <typename T, class PRED>
	uint32 TSet<T, PRED>::Count() const
	{
		uint32 count = 0;
		ForEach([&count](Node* node)
			{
				++count;
			});

		return count;
	}

	template <typename T, class PRED>
	void TSet<T, PRED>::InOrder(const InOrderFnc& predicate, Node* node) const
	{
		if (node == nullptr)
		{
			return;
		}

		InOrder(predicate, node->left);

		predicate(node);

		InOrder(predicate, node->right);
	}

	template <typename T, class PRED>
	void TSet<T, PRED>::RotateLeft(Node* x)
	{
		Node* y = x->right;
		x->right = y->left;

		if (y->left == nullptr)
		{
			y->left->parent = x;
		}

		y->parent = x->parent;

		if (x->parent == nullptr)
		{
			m_root = y;
		}
		else if (x == x->parent->left)
		{
			x->parent->left = y;
		}
		else
		{
			x->parent->right = y;
		}

		y->left = x;
		x->parent = y;
	}

	template <typename T, class PRED>
	void TSet<T, PRED>::RotateRight(Node* x)
	{
		Node* y = x->left;
		x->left = y->right;

		if (y->right == nullptr)
		{
			y->right->parent = x;
		}

		y->parent = x->parent;

		if (x->parent == nullptr)
		{
			m_root = y;
		}
		else if (x == x->parent->right)
		{
			x->parent->right = y;
		}
		else
		{
			x->parent->left = y;
		}

		y->right = x;
		x->parent = y;
	}

	template <typename T, class PRED>
	void TSet<T, PRED>::FixInsert(Node* node)
	{
		// Continue while the parent is Red (violates RB property)
		while (node != m_root && node->parent->color == ENodeColor::Red)
		{
			if (node->parent == node->parent->parent->left)
			{
				// Case 1: Uncle is Red -> Recolor parent, uncle, and grandparent
				Node* uncle = node->parent->parent->right;

				if (uncle->color == ENodeColor::Red)
				{
					node->parent->color = ENodeColor::Black;
					uncle->color = ENodeColor::Black;
					node->parent->parent->color = ENodeColor::Red;

					node = node->parent->parent;
				}
				else
				{
					if (node == node->parent->right)
					{
						// Case 2: Triangle shape -> Left rotate parent to form a line
						node = node->parent;
						RotateLeft(node);
					}

					// Case 3: Line shape -> Recolor and right rotate grandparent
					node->parent->color = ENodeColor::Black;
					node->parent->parent->color = ENodeColor::Red;
					RotateRight(node->parent->parent);
				}
			}
			else
			{
				// Mirror Case: Parent is the right child
				Node* uncle = node->parent->parent->left;

				if (uncle->color == ENodeColor::Red)
				{
					node->parent->color = ENodeColor::Black;
					uncle->color = ENodeColor::Black;

					node->parent->parent->color = ENodeColor::Red;
					node = node->parent->parent;
				}
				else
				{
					if (node == node->parent->left)
					{
						node = node->parent;
						RotateRight(node);
					}

					node->parent->color = ENodeColor::Black;
					node->parent->parent->color = ENodeColor::Red;
					RotateLeft(node->parent->parent);
				}
			}
		}

		// Root must always be Black
		m_root->color = ENodeColor::Black;
	}

	template <typename T, class PRED>
	void TSet<T, PRED>::CloneTree(Node*& modified, Node* toCopy)
	{
		modified->data = toCopy->data;
		modified->color = toCopy->color;

		if (toCopy->left != nullptr)
		{
			modified->left = new Node;
			modified->left->parent = modified;
			CloneTree(modified->left, toCopy->left);
		}

		if (toCopy->right != nullptr)
		{
			modified->right = new Node;
			modified->right->parent = modified;
			CloneTree(modified->right, toCopy->right);
		}
	}

	template <typename T, class PRED>
	TSet<T, PRED>& TSet<T, PRED>::operator=(TSet rhs)
	{
		std::swap(m_root, rhs.m_root);

		return *this;
	}

	template <typename T, class PRED>
	TSet<T, PRED>& TSet<T, PRED>::operator=(TSet&& rhs) noexcept
	{
		if (this == &rhs)
		{
			return *this;
		}

		m_root = rhs.m_root;
		rhs.m_root = nullptr;

		return *this;
	}
}
