#ifndef CTL_CONTAINER_TREE_REDBLACK_H
#define CTL_CONTAINER_TREE_REDBLACK_H

#include "../../namespace.hpp"
#include "../../cpperror.hpp"
#include "../../adapter/comparator.hpp"
#include "../../typetraits/traits.hpp"
#include "../../memory/allocator.hpp"
#include "../pair.hpp"

#include "comparator.hpp"

CTL_NAMESPACE_BEGIN

namespace Tree {
	// Based off of https://en.wikipedia.org/wiki/Red%E2%80%93black_tree
	// TODO: Figure out why it's broken (& also make it use `BaseTree`, like `AVL` does)
	/// @brief Red-Black Tree.
	/// @tparam TKey Node key type.
	/// @tparam TValue Node value type.
	/// @tparam TCompare<class> Comparator type.
	/// @tparam TAlloc<class> Allocator type. By default, it is `HeapAllocator`.
	/// @warning DO NOT USE THIS STRUCTURE! It's currently not working as intended!
	template<
		class TKey,
		class TValue,
		template <class> class TCompare,
		template <class> class TAlloc = HeapAllocator
	>
	struct RedBlack: Paired<TKey const, TValue> {
		using Paired		= ::CTL::Paired<TKey const, TValue>;

		using typename Paired::KeyType;
		using typename Paired::ValueType;
		
		using DataType		= KeyValuePair<KeyType&, ValueType&>;
		using ConstantType	= KeyValuePair<KeyType&, ValueType const&>;

		/// @brief Comparator type.
		using ComparatorType = TCompare<KeyType>;
		
		static_assert(Type::Tree::Comparator<KeyType, TCompare>, "TCompare must be a valid comparator for TData!");

		/// @brief Empty constructor (defaulted).
		constexpr RedBlack() = default;

		/// @brief Copy constructor.
		constexpr RedBlack(RedBlack const& other) {
			append(other);
		}

		/// @brief Move constructor (defaulted).
		constexpr RedBlack(RedBlack&& other) {
			root = other.root;
			other.root = nullptr;
		}

		/// @brief Copy assignment operator.
		constexpr RedBlack& operator=(RedBlack const& other) {
			clear();
			return append(other);
		}

		/// @brief Move assignment operator.
		constexpr RedBlack& operator=(RedBlack&& other) {
			clear();
			root = other.root;
			other.root = nullptr;
			return *this;
		}

		/// @brief Destructor.
		constexpr ~RedBlack() {clear();}
		
		/// @brief Tree node.
		struct Node {
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
			
			/// @brief Returns the left child.
			/// @return Left child.
			constexpr ref<Node> left() const	{return children[0];}
			/// @brief Returns the right child.
			/// @return Right child.
			constexpr ref<Node> right() const	{return children[1];}
		};

		/// @brief Allocator type.
		using AllocatorType = HeapAllocator<Node>;
		
		/// @brief Tree node iterator.
		/// @tparam R Whether it is a reverse iterator.
		/// @tparaam TNode Node type.
		template<bool R = false, Type::EqualOrConst<Node> TNode = Node>
		struct NodeIterator {
			/// @brief Node type.
			using NodeType = TNode;
			/// @brief Iterator value accessor type.
			using DataType = Meta::DualType<Type::Constant<NodeType>, ConstantType, DataType>;
			
			/// @brief Constructs the iterator.
			/// @param node Pointer to node.	
			constexpr NodeIterator(ref<NodeType> const node = nullptr): current(node), previous(nullptr) {}
			
			/// @brief Wether it is a reverse iterator.
			constexpr static bool REVERSE = R;
			
			/// @brief Pre-increment operator overloading.
			constexpr NodeIterator& operator++() {
				if (current)
					advance(!REVERSE);
				return *this;
			}
			
			/// @brief Pre-decrement operator overloading.
			constexpr NodeIterator& operator--() {
				if (current)
					advance(REVERSE);
				return *this;
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
			constexpr void advance(bool const forward) {
				if (paused) {
					if (current->children[forward])
						current = current->children[forward];
					else current = current->parent;
				}
				paused = false;
				while (current) {
					if (previous == current->parent) {
						previous = current;
						if (current->children[!forward]) {
							current = current->children[!forward];
						} else {
							paused = true;
							break;
						}
					} else if (previous == current->children[!forward]) {
						previous = current;
						paused = true;
						break;
					} else if (previous == current->children[forward]) {
						previous = current;
						current = current->parent;
					}
				}
			}

			constexpr DataType pair() const {
				if (!current) throw NullPointerException("Iterator is empty!");
				return {current->key, current->value};
			}

			/// @brief Current node.
			ref<NodeType>	current;
			/// @brief Previous visited node.
			ref<NodeType>	previous	= nullptr;
			/// @brief Whether traversal was paused until the next iteration.
			bool			paused		= false;
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
		/// @param right Whether to do a rightwise rotation.
		constexpr void rotateBranch(ref<Node> const branch, bool const right) {
			if (!branch) return;
			ref<Node> const parent	= branch->parent;
			ref<Node> const newRoot	= branch->children[!right];
			ref<Node> const child	= branch->children[right];
			branch->children[!right] = child;
			if (child) child->parent = branch;
			newRoot->children[right] = branch;
			newRoot->parent = parent;
			branch->parent = newRoot;
			if (parent)
				parent->children[isRightChild(branch)] = newRoot;
			else root = newRoot;
		}
		
		/// @brief Inserts a node into a parent.
		/// @param node Node to insert.
		/// @param parent Parent to insert to.
		/// @param right Whether to insert as the right child. 
		constexpr void insertNode(ref<Node> node, ref<Node> parent, bool right) {
			if (!node) return;
			node->red = true;
			node->parent = parent;
			if (!parent) {
				root = node;
				return;
			}
			parent->children[right] = node;
			do {
				if (!parent->red) return;
				ref<Node> grandparent = parent->parent;
				if (!grandparent) {
					parent->red = false;
					return;
				}
				right = isRightChild(parent);
				ref<Node> const uncle = grandparent->children[!right];
				if (!uncle || !uncle->red) {
					if (node == parent->children[!right]) {
						rotateBranch(parent, right);
						node = parent;
						parent = grandparent->children[right];
					}
					rotateBranch(grandparent, !right);
					parent->red			= false;
					grandparent->red	= true;
					return;
				}
				parent->red			= false;
				uncle->red			= false;
				grandparent->red	= true;
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
			bool right = !isRightChild(node);
			parent->children[right] = nullptr;
			do {
				sibling = parent->children[!right];
				farNephew = sibling->children[!right];
				closeNephew = sibling->children[right];
				if (sibling->red) {
					rotateBranch(parent, right);
					parent->red = true;
					sibling->red = false;
					sibling = closeNephew;
					farNephew = sibling->child[!right];
					if (farNephew && farNephew->red) {
						repaintRight(sibling, parent, farNephew, right);
						return;
					}
					closeNephew = sibling->child[right];
					if (closeNephew && closeNephew->red) {
						repaintLeft(sibling, closeNephew, farNephew, right);
						repaintRight(sibling, parent, farNephew, right);
						return;
					}
					sibling->red = true;
					parent->red = false;
					return;
				}
				if (farNephew && farNephew->red) {
					repaintRight(sibling, parent, farNephew, right);
					return;
				}
				if (closeNephew && closeNephew->red) {
					repaintLeft(sibling, closeNephew, farNephew, right);
					repaintRight(sibling, parent, farNephew, right);
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
				right = !isRightChild(node);
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
			auto const parent = findParent(key);
			if (parent && ComparatorType::equals(parent->key, key))
				return parent;
			ref<Node> const node = MX::construct(alloc.allocate(), key);
			if (!parent)
				return (root = node);
			insertNode(node, parent, !ComparatorType::lesser(key, parent->key));
			return node;
		}
		
		/// @brief Finds a node containing a key in the tree.
		/// @param key Key to match.
		/// @return Node containing the key, or `nullptr`.
		constexpr ref<Node const> find(KeyType const& key) const {
			auto result = findParent(key);
			if (result && ComparatorType::equals(key, result->key))
				return result;
			return nullptr;
		}
		
		/// @brief Finds a node containing a key in the tree.
		/// @param val Key to match.
		/// @return Node containing the key, or `nullptr`.
		constexpr ref<Node> find(KeyType const& key) {
			auto result = findParent(key);
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
			root = nullptr;
		}

		/// @brief Adds another container's items to this one.
		/// @param other Container to copy from.
		/// @return Reference to self.
		constexpr RedBlack& append(RedBlack const& other) {
			for (auto node: other)
				if (auto const newNode = insert(node->front()))
					newNode->value = node.back();
				else throw FailedActionException("Failed to insert key-value pair!");
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
			traverseAndDelete(node->left());
			traverseAndDelete(node->right());
			alloc.deallocate(MX::destruct(node));
		}

		/// @brief Returns the leftmost node in the linked list.
		/// @return Rightmost node.
		constexpr ref<Node> leftmostEdge() const {
			if (!root) return nullptr;
			ref<Node> edge = root;
			while (edge && edge->left()) edge = edge->left();
			return edge;
		}
		
		/// @brief Returns the rightmost node in the linked list.
		/// @return Rightmost node.
		constexpr ref<Node> rightmostEdge() const {
			if (!root) return nullptr;
			ref<Node> edge = root;
			while (edge && edge->right()) edge = edge->right();
			return edge;
		}
	
		constexpr static ref<Node> searchBranch(ref<Node> node, KeyType const& key) {
			if (!node) return nullptr;
			if (!(node->left() || node->right())) return node;
			ref<Node> parent = node;
			while (node) {
				parent = node;
				if (ComparatorType::equals(node->key, key)) break;
				node = node->children[!ComparatorType::lesser(key, node->key)];
			}
			return parent;
		}

		constexpr void repaintLeft(ref<Node>& sibling, ref<Node>& closeNephew, ref<Node>& farNephew, bool& right) {
			rotateBranch(sibling, !right);
			sibling->red = true;
			closeNephew->red = false;
			farNephew = sibling;
			sibling = closeNephew;
		}
		
		constexpr void repaintRight(ref<Node>& sibling, ref<Node>& parent, ref<Node>& farNephew, bool& right) {
			rotateBranch(parent, right);
			sibling->red = parent->red;
			parent->red = false;
			farNephew->red = false;
		}
	};
}

CTL_NAMESPACE_END

#endif