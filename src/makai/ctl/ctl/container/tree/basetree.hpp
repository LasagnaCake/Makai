#ifndef CTL_CONTAINER_TREE_BASETREE_H
#define CTL_CONTAINER_TREE_BASETREE_H

#include "../../namespace.hpp"
#include "../../cpperror.hpp"
#include "../../adapter/comparator.hpp"
#include "../../typetraits/traits.hpp"
#include "../../memory/allocator.hpp"
#include "../pair.hpp"
#include "../../meta/empty.hpp"

#include "comparator.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Tree data structures.
namespace Tree {
	/// @brief BST (Binary Search Tree) base.
	/// @tparam TKey Node key type.
	/// @tparam TValue Node value type.
	/// @tparam TCompare<class> Comparator type.
	/// @tparam TAlloc<class> Allocator type. By default, it is `HeapAllocator`.
	/// @tparam TNodeExtension Node extension type. By default, it is `Empty`.
	template<
		class TKey,
		class TValue,
		template <class> class TCompare,
		template <class> class TAlloc = HeapAllocator,
		class TNodeExtension = Empty
	>
	struct BaseTree: Paired<TKey const, TValue> {
		using Paired		= ::CTL::Paired<TKey const, TValue>;

		using typename Paired::KeyType;
		using typename Paired::ValueType;
		
		using DataType		= KeyValuePair<KeyType&, ValueType&>;
		using ConstantType	= KeyValuePair<KeyType&, ValueType const&>;

		/// @brief Comparator type.
		using ComparatorType = TCompare<KeyType>;
		
		static_assert(Type::Tree::Comparator<KeyType, TCompare>, "TCompare must be a valid comparator for TData!");
		
		/// @brief Tree node.
		struct Node: TNodeExtension {
			/// @brief Node key.
			KeyType		key;
			/// @brief Node value.
			ValueType	value;
			/// @brief Parent.
			ref<Node>	parent		= nullptr;
			/// @brief Left & right children.
			ref<Node>	children[2]	= {nullptr, nullptr};
			
			/// @brief Returns the left child.
			/// @return Left child.
			constexpr ref<Node> left() const	{return children[0];}
			/// @brief Returns the right child.
			/// @return Right child.
			constexpr ref<Node> right() const	{return children[1];}
		};
		
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
			constexpr NodeIterator(ref<NodeType> const node = nullptr): current(node), previous(nullptr) {
				advance(!REVERSE);
			}
			
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

		/// @brief Forward iterator type.
		using IteratorType				= NodeIterator<false, Node>;
		/// @brief Reverse iterator type.
		using ReverseIteratorType		= NodeIterator<true, Node>;
		/// @brief Forward const iterator type.
		using ConstIteratorType			= NodeIterator<false, Node const>;
		/// @brief Reverse const iterator type.
		using ConstReverseIteratorType	= NodeIterator<true, Node const>;
		
		/// @brief Allocator type.
		using AllocatorType			= typename ContextAwareAllocatable<TAlloc, Node>::AllocatorType;
		/// @brief Constant allocator type.
		using ConstantAllocatorType	= typename ContextAwareAllocatable<TAlloc, Node>::ConstantAllocatorType;

		/// @brief Returns the associated allocator.
		/// @return Associated allocator.
		constexpr auto& allocator() {
			if (inCompileTime())
				return calloc;
			else return alloc;
		}

	protected:
		/// @brief Allocator.
		AllocatorType	alloc;
		/// @brief Constant allocator.
		AllocatorType	calloc;
	};
}

CTL_NAMESPACE_END

#endif