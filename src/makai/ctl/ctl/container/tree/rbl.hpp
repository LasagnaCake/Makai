#ifndef CTL_CONTAINER_TREE_RBL_H
#define CTL_CONTAINER_TREE_RBL_H

#include "../../namespace.hpp"
#include "../../cpperror.hpp"
#include "../../adapter/comparator.hpp"
#include "../../typetraits/traits.hpp"
#include "../../memory/allocator.hpp"
#include "../pair.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Tree-specific type constraints.
namespace Type::Tree {
	/// @brief Whether `TComparator` forms a valid tree comparator with `TData`.
	template <class TData, template <class> class TCompare>
	concept Comparator = requires {
		{TCompare<TData>::lesser(TData(), TData())}	-> Type::Convertible<bool>;
		{TCompare<TData>::equal(TData(), TData())}	-> Type::Convertible<bool>;
	};
};

namespace Tree {
	// Based off of https://en.wikipedia.org/wiki/Red%E2%80%93black_tree, modified to allow for easier iteration
	/// @brief Red-Black Tree + Doubly-Linked-List hybrid.
	/// @tparam TKey Node key type.
	/// @tparam TValue Node value type.
	/// @tparam TCompare<class> Comparator type.
	/// @tparam TAlloc<class> Allocator type. By default, it is `HeapAllocator`.
	/// @tparam D Whether to allow duplicate values. By default, it is `false`.
	template<
		class TKey,
		class TValue,
		template <class> class TCompare,
		template <class> class TAlloc = HeapAllocator,
		bool D = false
	>
	struct RBL: Paired<TKey, TValue> {
		struct Node;

		using Paired		= Paired<TKey, TValue>;
		using Allocatable	= Allocatable<TAlloc, Node>;
		
		using DataType		= KeyValuePair<TKey const&, TValue&>;

		using ConstantType	= KeyValuePair<TKey const&, TValue const&>;

		using typename Paired::KeyType;
		using typename Paired::ValueType;

		using typename Allocatable::AllocatorType;

		/// @brief Comparator type.
		using ComparatorType = TCompare<KeyType>;
		
		static_assert(Type::Tree::Comparator<KeyType, TCompare>, "TCompare must be a valid comparator for TData!");
		
		/// @brief Whether duplicate values are allowed
		constexpr static bool ALLOW_DUPES = D;

		/// @brief Empty constructor (defaulted).
		constexpr RBL() = default;

		/// @brief Copy constructor.
		constexpr RBL(RBL const& other) {
			append(other);
		}

		/// @brief Move constructor (defaulted).
		constexpr RBL(RBL&& other) = default;

		/// @brief Copy assignment operator.
		constexpr RBL& operator=(RBL const& other) {
			traverseAndDelete(root);
			return append(other);
		}

		/// @brief Move assignment operator.
		constexpr RBL& operator=(RBL&& other) {
			traverseAndDelete(root);
			root = other.root;
			other.root = nullptr;
			return *this;
		}

		/// @brief Destructor.
		constexpr ~RBL() {traverseAndDelete(root);}
		
		/// @brief Tree node.
		struct Node: Ordered {
			/// @brief Node key.
			KeyType		key;
			/// @brief Node value.
			ValueType	value;
			/// @brief Parent.
			ref<Node>	parent		= nullptr;
			/// @brief Left & right children.
			ref<Node>	children[2]	= {nullptr, nullptr};
			/// @brief Whether the node is red.
			bool		red			= false;
			/// @brief Previous node in the list.
			ref<Node>	prev		= nullptr;
			/// @brief Next node in the list.
			ref<Node>	next		= nullptr;
			
			/// @brief Returns the left child.
			/// @return Left child.
			constexpr ref<Node> left() const	{return children[0];}
			/// @brief Returns the right child.
			/// @return Right child.
			constexpr ref<Node> right() const	{return children[1];}
			
			/// @brief Links two nodes in the list, while respecting their children.
			/// @param node Node to parent.
			/// @param parent Parent to parent to.
			constexpr static void append(ref<Node> const node, ref<Node> parent) {
				if (!node || !parent) return;
				if (ComparatorType::lesser(node->key, parent->key)) {
					if constexpr (ALLOW_DUPES) parent = leftEdge(parent);
					node->next = parent;
					if (parent->prev) {
						parent->prev->next	= node;
						node->prev			= parent->prev;
					}
					parent->prev = node;
				} else {
					if constexpr (ALLOW_DUPES) parent = rightEdge(parent);
					node->prev = parent;
					if (parent->next) {
						parent->next->prev	= node;
						node->next			= parent->next;
					}
					parent->next = node;
				}
			}
			
			/// @brief Links two nodes, sequentially, in the list.
			/// @param prev "Before" node.
			/// @param next "After" node.
			constexpr static void link(ref<Node> const prev, ref<Node> const next) {
				if (!next || !prev) return;
				else if (!next)	prev->next = nullptr;
				else if (!prev)	next->prev = nullptr;
				else {
					prev->next = next;
					next->prev = prev;
				}
			}
			
			/// @brief Returns the leftmost edge of a node that contains the same key as itself in the list.
			/// @param node Node to get edge of.
			/// @return Edge node.
			constexpr static ref<Node> leftEdge(ref<Node> node) {
				if (!node) return node;
				while (node->prev && ComparatorType::equals(node->key, node->prev->key))
					node = node->prev;
				return node;
			}
			
			/// @brief Returns the rightmost edge of a node that contains the same key as itself in the list.
			/// @param node Node to get edge of.
			/// @return Edge node.
			constexpr static ref<Node> rightEdge(ref<Node> node) {
				if (!node) return node;
				while (node->next && ComparatorType::equals(node->key, node->next->key))
					node = node->next;
				return node;
			}
		};
		
		/// @brief Tree node iterator.
		/// @tparam R Whether it is a reverse iterator.
		/// @tparaam TNode Node type.
		template<bool R = false, Type::EqualOrConst<Node> TNode = Node>
		struct NodeIterator {
			/// @brief Node type.
			using NodeType = TNode;
			/// @brief Iterator value accessor type.
			using DataType = Meta::DualType<
				Type::Constant<NodeType>,
				KeyValuePair<TKey const&, TValue&>,
				KeyValuePair<TKey const&, TValue const&>
			>;
			
			/// @brief Constructs the iterator.
			/// @param node Pointer to node.	
			constexpr NodeIterator(ref<NodeType> const node = nullptr): current(node) {}
			
			/// @brief Wether it is a reverse iterator.
			constexpr static bool REVERSE = R;
			
			/// @brief Pre-increment operator overloading.
			constexpr NodeIterator& operator++(int) {
				if (current) {
					if constexpr (!REVERSE)	current = current->next;
					else					current = current->prev;
				}
			}
			
			/// @brief Pre-decrement operator overloading.
			constexpr NodeIterator& operator--(int) {
				if (current) {
					if constexpr (!REVERSE)	current = current->prev;
					else					current = current->next;
				}
			}
			
			/// @brief Dereference operator overloading.
			constexpr DataType operator*() const {
				return pair();
			}
			
			/// @brief Comparison operator overloading.
			/// @return Whether iterators are equal.
			constexpr bool operator==(NodeIterator const& other) const {
				return current == other.current;
			}
			
		private:
			constexpr DataType pair() const {
				if (!current) throw NullPointerException("Iterator is empty!");
				return {current->key, current->value};
			}

			/// @brief Current node.
			ref<NodeType> current;
		};
		
		/// @brief Returns an iterator to the "begginning" of the tree.
		/// @return Iterator to begginning of tree.
		constexpr NodeIterator<false, Node> begin()				{return {leftmostEdge()};	}
		/// @brief Returns an iterator to the "end" of the tree.
		/// @return Iterator to begginning of tree.
		constexpr NodeIterator<false, Node> end()				{return {};					}
		
		/// @brief Returns an iterator to the "begginning" of the tree.
		/// @return Iterator to begginning of tree.
		constexpr NodeIterator<false, Node const> begin() const	{return {leftmostEdge()};	}
		/// @brief Returns an iterator to the "end" of the tree.
		/// @return Iterator to begginning of tree.
		constexpr NodeIterator<false, Node const> end() const	{return {};					}
		
		/// @brief Returns a reverse iterator to the "begginning" of the tree.
		/// @return Reverse iterator to begginning of tree.
		constexpr NodeIterator<true, Node> rbegin()				{return {rightmostEdge()};	}
		/// @brief Returns a reverse iterator to the "end" of the tree.
		/// @return Reverse iterator to begginning of tree.
		constexpr NodeIterator<true, Node> rend()				{return {};					}
		
		/// @brief Returns a reverse iterator to the "begginning" of the tree.
		/// @return Reverse iterator to begginning of tree.
		constexpr NodeIterator<true, Node const> rbegin() const	{return {rightmostEdge()};	}
		/// @brief Returns a reverse iterator to the "end" of the tree.
		/// @return Reverse iterator to begginning of tree.
		constexpr NodeIterator<true, Node const> rend() const	{return {};					}

		/// @brief Returns whether the tree is empty.
		/// @return Whether tree is empty.
		constexpr bool empty() const {return root;}
		
		/// @brief Returns the key-value pair at the "begginning" of the tree.
		/// @return Value at begginning of tree.
		/// @throw NonexistentValueException if tree is empty.
		constexpr ConstantType front() const {
			if (!root) throw NonexistentValueException("Tree is empty!");
			auto const edge = leftmostEdge();
			return {edge->key, edge->value};
		}
		
		/// @brief Returns the key-value pair at the "begginning" of the tree.
		/// @return Value at begginning of tree.
		/// @throw NonexistentValueException if tree is empty.
		constexpr DataType front() {
			if (!root) throw NonexistentValueException("Tree is empty!");
			auto const edge = leftmostEdge();
			return {edge->key, edge->value};
		}
		
		/// @brief Returns the key-value pair at the "end" of the tree.
		/// @return Value at end of tree.
		/// @throw NonexistentValueException if tree is empty.
		constexpr ConstantType back() const {
			if (!root) throw NonexistentValueException("Tree is empty!");
			auto const edge = rightmostEdge();
			return {edge->key, edge->value};
		}
		
		/// @brief Returns the key-value pair at the "end" of the tree.
		/// @return Value at end of tree.
		/// @throw NonexistentValueException if tree is empty.
		constexpr DataType back() {
			if (!root) throw NonexistentValueException("Tree is empty!");
			auto const edge = rightmostEdge();
			return {edge->key, edge->value};
		}
		
		/// @brief Returns whether a node is the right-side child of its parent.
		/// @return Whether node is right child.
		/// @note Returns `false` if node doesn't have a parent.
		constexpr static bool isRightChild(ref<Node> const node) {
			if (!node) return false;
			return node->parent && node == node->parent->children[1];
		}
		
		/// @brief Rotates a branch.
		/// @param left Whether to do a leftwise rotation.
		constexpr void rotateBranch(ref<Node> const branch, bool const left) {
			if (!branch) return;
			ref<Node> const newRoot	= branch->children[left];
			ref<Node> const child	= branch->children[!left];
			branch->children[left]	= child;
			if (child) child->parent = branch;
			newRoot->children[!left]	= branch;
			if (branch->parent)
				branch->parent->children[isRightChild(branch)] = newRoot;
			else root = newRoot;
		}
		
		/// @brief Inserts a node into a parent.
		/// @param node Node to insert.
		/// @param parent Parent to insert to.
		/// @param left Whether to insert as the left child. 
		constexpr void insertNode(ref<Node> node, ref<Node> parent, bool left) {
			if (!node) return;
			node->red = true;
			node->parent = parent;
			if (!parent) {
				root = node;
				return;
			}
			ref<Node> grandparent = parent->parent;
			parent->children[!left] = node;
			do {
				if (!parent->red) return;
				if (!grandparent) {
					parent->red = true;
					return;
				}
				left = parent != grandparent->right;
				ref<Node> const uncle = grandparent->children[left];
				if (!uncle || !uncle->red) {
					if (node == grandparent->children[left]) {
						rotateBranch(parent, left);
						node = parent;
						parent = grandparent->children[!left];
					}
					rotateBranch(grandparent, !left);
					parent->red			= false;
					grandparent->red	= true;
					return;
				}
				parent->red	= false;
				uncle->red	= false;
				node = grandparent;
			} while ((parent = node->parent));
		}
		
		/// @brief Removes a node from the tree.
		/// @param node Node to remove.
		/// @warning DO NOT USE THIS ONE DIRECTLY! If you MUST, use `removeAndRelink` instead!
		constexpr void removeNode(ref<Node> node) {
			ref<Node> parent = node->parent;
			ref<Node> sibling		= nullptr;
			ref<Node> closeNephew	= nullptr;
			ref<Node> farNephew		= nullptr;
			bool left = !isRightChild(node);
			parent->children[!left] = nullptr;
			do {
				sibling = parent->children[left];
				farNephew = sibling->children[left];
				closeNephew = sibling->children[!left];
				if (sibling->red) {
					rotateBranch(parent, left);
					parent->red = true;
					sibling->red = false;
					sibling = closeNephew;
					farNephew = sibling->child[left];
					if (farNephew && farNephew->red) {
						repaintLeft(sibling, parent, farNephew, left);
						return;
					}
					closeNephew = sibling->child[!left];
					if (closeNephew && closeNephew->red) {
						repaintRight(sibling, closeNephew, farNephew, left);
						repaintLeft(sibling, parent, farNephew, left);
						return;
					}
					sibling->red = true;
					parent->red = false;
					return;
				}
				if (farNephew && farNephew->red) {
					repaintLeft(sibling, parent, farNephew, left);
					return;
				}
				if (closeNephew && closeNephew->red) {
					repaintRight(sibling, closeNephew, farNephew, left);
					repaintLeft(sibling, parent, farNephew, left);
					return;
				}
				if (parent && parent->red) {
					sibling->red = true;
					parent->red = false;
					return;
				}
				if (!parent) return;
				sibling->red = true;
				node = parent;
				left = !isRightChild(node);
			} while ((parent = node->parent));
		}
		
		/// @brief Finds the appropriate parent node for a key.
		/// @param key Key to find parent for.
		/// @return Appropriate parent, or `nullptr` if tree is empty.
		constexpr ref<Node const> findParent(KeyType const& key) const {
			if (!root) return nullptr;
			return searchBranch(root, key);
		}
		
		/// @brief Finds the appropriate parent node for a key.
		/// @param val Key to find parent for.
		/// @return Appropriate parent, or `nullptr` if tree is empty.
		constexpr ref<Node> findParent(KeyType const& key) {
			if (!root) return nullptr;
			return searchBranch(root, key);
		}
		
		/// @brief Inserts a key in the tree.
		/// @param key Key to insert.
		/// @return Node containing the key.
		constexpr ref<Node> insert(KeyType const& key) {
			ref<Node> const parent = findParent(key);
			if constexpr (!ALLOW_DUPES) {
				if (ComparatorType::equals(parent->key, key))
					return parent;
			}
			ref<Node> const node = MX::construct(alloc.allocate(), key);
			insertNode(node, parent, true);
			Node::append(node, parent);
			return node;
		}
		
		/// @brief Finds a node containing a key in the tree.
		/// @param key Key to match.
		/// @return Node containing the key, or `nullptr`.
		constexpr ref<Node const> find(KeyType const& key) const {
			ref<Node> result = findParent(key);
			if (result && ComparatorType::equals(key, result->key))
				return result;
			return nullptr;
		}
		
		/// @brief Finds a node containing a key in the tree.
		/// @param val Key to match.
		/// @return Node containing the key, or `nullptr`.
		constexpr ref<Node> find(KeyType const& key) {
			ref<Node> result = findParent(key);
			if (result && ComparatorType::equals(key, result->key))
				return result;
			return nullptr;
		}
		
		/// @brief Removes a node from the tree, and updates its links.
		/// @param node Node to remove.
		/// @return Removed node, or `nullptr` if node does not exist.
		constexpr ref<Node> removeAndRelink(ref<Node> const node) {
			if (!node) return nullptr;
			removeNode(node);
			Node::link(node->prev, node->next);
			return node;
		}
		
		/// @brief Erases a node with a given key from the tree.
		/// @param key Key to erase.
		constexpr void erase(KeyType const& key) {
			if (auto node = removeAndRelink(find(key)))
				alloc.deallocate(MX::destruct(node));
		}

		/// @brief Deletes all nodes in the tree.
		constexpr void clear() {
			traverseAndDelete(root);
		}

		/// @brief Adds another container's items to this one.
		/// @param other Container to copy from.
		/// @return Reference to self.
		constexpr RBL& append(RBL const& other) {
			for (auto& node: other)
				insert(node.front())->value = node.back();
			return *this;
		}

		/// @brief Returns the associated allocator.
		/// @return Associated allocator.
		constexpr AllocatorType& allocator() {return alloc;}
		
	private:
		/// @brief Tree root.
		owner<Node>		root = nullptr;
		/// @brief Allocator.
		AllocatorType	alloc;

		constexpr void traverseAndDelete(ref<Node> const node) {
			if (!node) return;
			traverseAndDelete(node->left);
			traverseAndDelete(node->right);
			alloc.deallocate(MX::destruct(node));
		}

		/// @brief Returns the leftmost node in the linked list.
		/// @return Rightmost node.
		constexpr ref<Node> leftmostEdge() const {
			if (!root) return nullptr;
			ref<Node> edge = root;
			while (edge && edge->left()) edge = edge->left();
			if constexpr (ALLOW_DUPES)
				return Node::leftEdge(edge);
			else return edge;
		}
		
		/// @brief Returns the rightmost node in the linked list.
		/// @return Rightmost node.
		constexpr ref<Node> rightmostEdge() const {
			if (!root) return nullptr;
			ref<Node> edge = root;
			while (edge && edge->right()) edge = edge->right();
			if constexpr (ALLOW_DUPES)
				return Node::rightEdge(edge);
			else return edge;
		}
	
		constexpr static ref<Node> searchBranch(ref<Node> node, KeyType const& key) {
			if (!node) return nullptr;
			if (!(node->left() || node->right())) return node;
			while (node->left() || node->right()) {
				if (ComparatorType::lesser(node->key, key) && node->left())
					node = node->left();
				else if ((!ComparatorType::equals(node->key, key)) && node->right())
					node = node->right();
				else break;
			}
			return node;
		}

		constexpr void repaintRight(ref<Node>& sibling, ref<Node>& closeNephew, ref<Node>& farNephew, bool& left) {
			rotateBranch(sibling, !left);
			sibling->red = true;
			closeNephew->red = false;
			farNephew = sibling;
			sibling = closeNephew;
		}
		
		constexpr void repaintLeft(ref<Node>& sibling, ref<Node>& parent, ref<Node>& farNephew, bool& left) {
			rotateBranch(parent, left);
			sibling->red = parent->red;
			parent->red = false;
			farNephew->red = false;
		}
	};
}

CTL_NAMESPACE_END

#endif