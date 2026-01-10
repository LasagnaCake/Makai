#ifndef CTL_CONTAINER_LISTS_LINKEDLIST_H
#define CTL_CONTAINER_LISTS_LINKEDLIST_H

#include "../../templates.hpp"
#include "../../ctypes.hpp"
#include "../../cpperror.hpp"
#include "../../typetraits/traits.hpp"
#include "../../typetraits/forcestate.hpp"
#include "../iterator.hpp"
#include "../function.hpp"
#include "../span.hpp"
#include "../../algorithm/sort.hpp"
#include "../../algorithm/reverse.hpp"
#include "../../algorithm/search.hpp"
#include "../../algorithm/transform.hpp"
#include "../../adapter/comparator.hpp"
#include "../../memory/memory.hpp"

CTL_NAMESPACE_BEGIN

template<
	class TData,
	Type::Integer TIndex = usize,
	template <class> class TAlloc		= HeapAllocator,
	template <class> class TConstAlloc	= ConstantAllocator
>
struct LinkedList;

/// @brief Container-specific type constraints.
namespace Type::Container {
	/// @brief Implementation of type constraints.
	namespace Impl {
		template<class T>
		struct IsLinkedList;

		template<
			template <
				class,
				class,
				template <class> class,
				template <class> class
			> class T0,
			class T1,
			class T2,
			template <class> class T3,
			template <class> class T4
		>
		struct IsLinkedList<T0<T1, T2, T3, T4>>: BooleanConstant<Type::Equal<T0<T1, T2, T3, T4>, ::CTL::LinkedList<T1, T2, T3, T4>>> {};
	}

	/// Type must be `LinkedList`.
	template<class T>
	concept LinkedList = Impl::IsLinkedList<T>::value; 
}

/// @brief Dynamic array of objects.
/// @tparam TData Element type.
/// @tparam TIndex Index type.
/// @tparam TAlloc<class> Runtime allocator type. By default, it is `HeapAllocator`.
/// @tparam TConstAlloc<class> Compile-time allocator type. By default, it is `ConstantAllocator`.
template<
	class TData,
	Type::Integer TIndex,
	template <class> class TAlloc,
	template <class> class TConstAlloc
>
struct LinkedList:
	Iteratable<TData, TIndex>,
	SelfIdentified<LinkedList<TData, TIndex, TAlloc, TConstAlloc>>,
	ContextAwareAllocatable<TData, TAlloc>,
	Ordered {
	using Iteratable				= ::CTL::Iteratable<TData, TIndex>;
	using SelfIdentified			= ::CTL::SelfIdentified<LinkedList<TData, TIndex, TAlloc, TConstAlloc>>;
	using ContextAwareAllocatable	= ::CTL::ContextAwareAllocatable<TData, TAlloc, TConstAlloc>;

	using
		typename Iteratable::DataType,
		typename Iteratable::ConstantType,
		typename Iteratable::PointerType,
		typename Iteratable::ConstPointerType,
		typename Iteratable::ReferenceType,
		typename Iteratable::ConstReferenceType
	;

	using
		typename Iteratable::IndexType,
		typename Iteratable::SizeType
	;

	using
		typename Iteratable::IteratorType,
		typename Iteratable::ConstIteratorType,
		typename Iteratable::ReverseIteratorType,
		typename Iteratable::ConstReverseIteratorType
	;

	using
		typename SelfIdentified::SelfType
	;

	using
		typename ContextAwareAllocatable::ContextAllocatorType
	;

	/// @brief Transformation function type.
	using TransformType	= Decay::AsFunction<DataType(ConstReferenceType)>;

	/// @brief Predicate function type.
	using PredicateType	= Decay::AsFunction<bool(ConstReferenceType)>;
	/// @brief Comparison function type.
	using CompareType	= Decay::AsFunction<bool(ConstReferenceType, ConstReferenceType)>;

	/// @brief Comparator type.
	using ComparatorType = SimpleComparator<DataType>;

	/// @brief Underlying storage type.
	using StorageType = MemorySlice<DataType, TAlloc>;

private:
	struct Node {
		using DataType = DataType;

		DataType	value;
		ref<Node>	prev = nullptr;
		ref<Node>	next = nullptr;

		constexpr static void unlink(Node& node) {
			auto left	= node.prev;
			auto right	= node.next;
			if (left)	left->next	= right;
			if (right)	right->prev	= left;
		}

		constexpr static void link(Node& left, Node& right) {
			left.next	= &right;
			right.prev	= &left;
		}

		constexpr static void parent(Node& parent, Node& child) {
			if (parent.next) {
				auto const next = parent.next;
				link(child, *next);
			}
			link(parent, child);
		}
	};
public:

	template <bool R, bool C = false>
	struct Iterator {
		using NodeType			= Meta::MakeConstIf<C,	Node>;
		using LinkedListType	= Meta::MakeConstIf<C,	LinkedList>;

		constexpr Iterator(ref<NodeType> const node, ref<LinkedListType> const parent): current(node), parent(parent) {}

		constexpr Iterator(Iterator const&)	= default;
		constexpr Iterator(Iterator&&)		= default;

		constexpr Iterator& operator++()	{if constexpr (R) retreat(); else advance(); return *this;}
		constexpr Iterator& operator--()	{if constexpr (R) advance(); else retreat(); return *this;}
		

		constexpr Iterator operator++(int)	{auto prev = copy(*this); if constexpr (R) retreat(); else advance(); return prev;}
		constexpr Iterator operator--(int)	{auto prev = copy(*this); if constexpr (R) advance(); else retreat(); return prev;}

		constexpr auto operator*() const {
			if (!current) emptyError();
			return current->value;
		}

		constexpr bool operator==(Iterator const& other) const {return current == other.current;}

		constexpr NodeType& node() const {
			if (!current) emptyError();
			return *current;
		}
		
		constexpr LinkedListType& list() const {
			if (!parent) emptyError();
			return *parent;
		}

	private:
		[[noreturn]] constexpr static void emptyError() {
			throw NullPointerException("Iterator does not point to anything!");
		}

		void advance() {if (current && current->next) current = current->next;}
		void retreat() {if (current && current->prev) current = current->prev;}

		ref<NodeType>		current	= nullptr;
		ref<LinkedListType>	parent	= nullptr;
	};

	template <bool A, bool B> friend struct Iterator;

	constexpr LinkedList(SelfType const& other) {for (auto const& e: other) pushBack(e);}

	template <class... Args>
	constexpr LinkedList(Args const&... args)
	requires (... && Type::CanBecome<Args, DataType>) {
		(..., pushBack(args));
	}

	template<class... Args>
	constexpr SelfType& constructBack(Args... args) {
		ref<Node> const node = new Node{args...};
		if (head == nullptr)
			head = node;
		else Node::parent(*tail, *node);
		tail = node;
		++count;
		return *this;
	}

	constexpr SelfType& pushBack(ConstReferenceType value) {
		ref<Node> const node = new Node{value};
		if (head == nullptr)
			head = node;
		else Node::parent(*tail, *node);
		tail = node;
		++count;
		return *this;
	}

	constexpr DataType popBack() {
		if (!count) emptyError();
		--count;
		auto value = tail->value;
		if (tail && head != tail) {
			ref<Node> const newTail = tail->prev;
			Node::unlink(*tail);
			delete tail;
			tail = newTail;
		} else if (tail) 
			removeHeadAndTail();
		return value;
	}
	
	constexpr SelfType& pushFront(ConstReferenceType value) {
		ref<Node> const node = new Node{value};
		if (tail == nullptr)
			tail = node;
		else Node::parent(*node, *head);
		head = node;
		++count;
		return *this;
	}

	constexpr DataType popFront() {
		if (!count) emptyError();
		--count;
		auto value = head->value;
		if (head && head != tail) {
			ref<Node> const newHead = head->next;
			Node::unlink(*head);
			delete head;
			head = newHead;
		} else if (head)
			removeHeadAndTail();
		return value;
	}

	constexpr ConstReferenceType front() const {
		if (!head) emptyError();
		return head->value;
	}

	constexpr ConstReferenceType back() const {
		if (!tail) emptyError();
		return tail->value;
	}

	constexpr ReferenceType front() {
		if (!head) emptyError();
		return head->value;
	}

	constexpr ReferenceType back() {
		if (!tail) emptyError();
		return tail->value;
	}

	constexpr usize size() const {return count;}

	constexpr Iterator<false, false>	begin()			{return {head,		this};	};
	constexpr Iterator<false, false>	end()			{return {nullptr,	this};	};
	constexpr Iterator<false, true>		begin() const	{return {head,		this};	};
	constexpr Iterator<false, true>		end() const		{return {nullptr,	this};	};
	
	constexpr Iterator<true, false>		rbegin()		{return {tail,		this};	};
	constexpr Iterator<true, false>		rend()			{return {nullptr,	this};	};
	constexpr Iterator<true, true>		rbegin() const	{return {tail,		this};	};
	constexpr Iterator<true, true>		rend() const	{return {nullptr,	this};	};

	template <bool R, bool C>
	constexpr SelfType& erase(Iterator<R, C> const& at) {
		if (&at.list() != this) return *this;
		Node::unlink(at.node());
		if (&at.node() == head)
			head = head->next;
		if (&at.node() == tail)
			tail = tail->prev;
		delete &at.node();
		--count;
		return *this;
	}

	template <bool R, bool C>
	constexpr SelfType& insert(Iterator<R, C> const& at, ConstReferenceType value) {
		if (&at.list() != this) return *this;
		if (at == (R ? rend() : end())) return *this;
		auto const newNode = new Node{value};
		Node::parent(*newNode, at.node());
		if (&at.node() == head) head = newNode;
		return *this;
	}

private:
	constexpr void removeHeadAndTail() {
		delete head;
		head = tail = nullptr;
	}

	owner<Node>	head	= nullptr;
	owner<Node>	tail	= nullptr;
	usize		count	= 0;

	constexpr void assertIsInBounds(IndexType const index) const {
		CTL_DEVMODE_FN_DECL;
		if (index > static_cast<IndexType>(count-1)) outOfBoundsError();
	}

	[[noreturn]] constexpr static void invalidSizeError() {
		throw InvalidValueException("Invalid list size!");
	}

	[[noreturn]] constexpr static void atItsLimitError() {
		throw MaximumSizeFailure();
	}

	[[noreturn]] constexpr static void outOfBoundsError() {
		throw OutOfBoundsException("Index is out of bounds!");
	}

	[[noreturn]] constexpr static void emptyError() {
		throw OutOfBoundsException("Container is empty!");
	}
};

CTL_NAMESPACE_END

#endif