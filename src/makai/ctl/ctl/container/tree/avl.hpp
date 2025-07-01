#ifndef CTL_CONTAINER_TREE_AVL_H
#define CTL_CONTAINER_TREE_AVL_H

#include "basetree.hpp"
#include "comparator.hpp"

CTL_NAMESPACE_BEGIN

namespace Tree {
	namespace Base {
		struct AVLNode {
			/// @brief The node's current depth.
			usize depth		= 0;
			/// @brief The node's current weight.
			ssize weight	= 0;
		};
	}

	/// @brief AVL Tree.
	/// @tparam TKey Node key type.
	/// @tparam TValue Node value type.
	/// @tparam TCompare<class> Comparator type.
	/// @tparam TAlloc<class> Allocator type. By default, it is `HeapAllocator`.
	template<
		class TKey,
		class TValue,
		template <class> class TCompare,
		template <class> class TAlloc = HeapAllocator
	>
	struct AVL:
		BaseTree<TKey, TValue, TCompare, TAlloc, Base::AVLNode>,
		Derived<BaseTree<TKey, TValue, TCompare, TAlloc, Base::AVLNode>> {
		using Derived = ::CTL::Derived<BaseTree<TKey, TValue, TCompare, TAlloc, Base::AVLNode>>;

		using typename Derived::BaseType;

		using typename BaseType::KeyType;
		using typename BaseType::ValueType;
		using typename BaseType::DataType;
		using typename BaseType::ConstantType;
		using typename BaseType::Node;
		using typename BaseType::ComparatorType;
		using typename BaseType::IteratorType;
		using typename BaseType::ReverseIteratorType;
		using typename BaseType::ConstIteratorType;
		using typename BaseType::ConstReverseIteratorType;

		using BaseType::alloc;

		/// @brief Empty constructor (defaulted).
		constexpr AVL() = default;

		/// @brief Copy constructor.
		constexpr AVL(AVL const& other) {
			append(other);
		}

		/// @brief Move constructor (defaulted).
		constexpr AVL(AVL&& other) {
			root = other.root;
			other.root = nullptr;
		}

		/// @brief Copy assignment operator.
		constexpr AVL& operator=(AVL const& other) {
			clear();
			return append(other);
		}

		/// @brief Move assignment operator.
		constexpr AVL& operator=(AVL&& other) {
			clear();
			root = other.root;
			other.root = nullptr;
			return *this;
		}

		/// @brief Destructor.
		constexpr ~AVL() {clear();}
		
		/// @brief Returns an iterator to the "begginning" of the tree.
		/// @return Iterator to begginning of tree.
		constexpr IteratorType begin()						{return {leftmostEdge()};	}
		/// @brief Returns an iterator to the "end" of the tree.
		/// @return Iterator to begginning of tree.
		constexpr IteratorType end()						{return {};					}
		
		/// @brief Returns an iterator to the "begginning" of the tree.
		/// @return Iterator to begginning of tree.
		constexpr ConstIteratorType begin() const			{return {leftmostEdge()};	}
		/// @brief Returns an iterator to the "end" of the tree.
		/// @return Iterator to begginning of tree.
		constexpr ConstIteratorType end() const				{return {};					}
		
		/// @brief Returns a reverse iterator to the "begginning" of the tree.
		/// @return Reverse iterator to begginning of tree.
		constexpr ReverseIteratorType rbegin()				{return {rightmostEdge()};	}
		/// @brief Returns a reverse iterator to the "end" of the tree.
		/// @return Reverse iterator to begginning of tree.
		constexpr ReverseIteratorType rend()				{return {};					}
		
		/// @brief Returns a reverse iterator to the "begginning" of the tree.
		/// @return Reverse iterator to begginning of tree.
		constexpr ConstReverseIteratorType rbegin() const	{return {rightmostEdge()};	}
		/// @brief Returns a reverse iterator to the "end" of the tree.
		/// @return Reverse iterator to begginning of tree.
		constexpr ConstReverseIteratorType rend() const		{return {};					}

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
		
		/// @brief Inserts a node into a parent.
		/// @param node Node to insert.
		/// @param parent Parent to insert to.
		/// @param right Whether to insert as the right child. 
		constexpr static void insertNode(ref<Node> node, ref<Node> parent, bool right) {
			if (!(node || parent)) return;
			node->parent			= parent;
			parent->children[right]	= node;
			while (parent) {
				if (parent->weight > 1) {
					switch (parent->right()->weight) {
						case -1:
						case 0:		rotate(parent->right(), false);	break;
						case 1:		shuffle(parent->right(), false);	break;
						default: break;
					}
				} else if (parent->weight < -1) {
					switch (parent->left()->weight) {
						case -1:	shuffle(parent->left(), true);	break;
						case 0:
						case 1:		rotate(parent->left(), true);	break;
						default: break;
					}
				} else break;
				parent = parent->parent;
			}
		}
		
		/// @brief Removes a node from the tree.
		/// @param node Node to remove.
		constexpr static void removeNode(ref<Node> node) {
			if (!node) return;
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
			ref<Node> const node = MX::construct(alloc.allocate(), Node{cachedDepth(parent), 0, key});
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
		
		/// @brief Erases a node with a given key from the tree.
		/// @param key Key to erase.
		constexpr void erase(KeyType const& key) {
			if (auto node = find(key)) {
				//removeNode(node);
				//alloc.deallocate(MX::destruct(node));
			}
		}

		/// @brief Deletes all nodes in the tree.
		constexpr void clear() {
			traverseAndDelete(root);
			root = nullptr;
		}

		/// @brief Adds another container's items to this one.
		/// @param other Container to copy from.
		/// @return Reference to self.
		constexpr AVL& append(AVL const& other) {
			for (auto node: other)
				if (auto const newNode = insert(node->front()))
					newNode->value = node.back();
				else throw FailedActionException("Failed to insert key-value pair!");
			return *this;
		}
		
	private:
		/// @brief Tree root.
		owner<Node>		root = nullptr;
		
		constexpr static usize depth(ref<Node> const node) {
			if (!node) return 0;
			if (!(node->left() || node->right())) return node->depth;
			if (!node->left())	return depth(node->right());
			if (!node->right())	return depth(node->left());
			usize const dl = depth(node->left()), dr = depth(node->right());
			return dl < dr ? dr : dl;
		}

		constexpr static usize cachedDepth(ref<Node> const node) {
			if (!node) return 0;
			return node->depth;
		}

		constexpr static ssize weight(ref<Node> const node) {
			return
				static_cast<ssize>(depth(node->right()))
			-	static_cast<ssize>(depth(node->left()))
			;
		}

		constexpr static ssize cachedWeight(ref<Node> const node) {
			if(!node) return 0;
			if (!(node->left() || node->right())) return branchWeight(node);
			if (!node->left())	return node->right()->weight + branchWeight(node);
			if (!node->right())	return node->left()->weight + branchWeight(node);
			return node->right()->weight + node->left()->weight + branchWeight(node);
		}

		constexpr static ssize branchWeight(ref<Node> const node) {
			if (!node || !(node->left() || node->right())) return 0;
			if (!node->left())	return +1;
			if (!node->right())	return -1;
			return 0;
		}

		/// @brief Rotates a node.
		/// @param left Whether to do a left-wise rotation.
		constexpr static void rotate(ref<Node> const node, bool const right) {
			if (!(node && node->parent)) return;
			ref<Node> const root	= node;
			ref<Node> const parent	= node->parent;
			ref<Node> const forest	= root->children[right];
			parent->children[!right]	= forest;
			root->children[right]		= parent;
			forest->parent				= parent;
			parent->parent				= root;
			--root->depth;
			++parent->depth;
			parent->weight	= cachedWeight(root);
			root->weight	= cachedWeight(root);
			--parent->weight;
		}

		/// @brief Shuffles a node.
		/// @param left Whether to do a right-left rotation.
		constexpr static void shuffle(ref<Node> const node, bool const right) {
			rotate(node, right);
			rotate(node, !right);
		}

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
	};
}

CTL_NAMESPACE_END

#endif