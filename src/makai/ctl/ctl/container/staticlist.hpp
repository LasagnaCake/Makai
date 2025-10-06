#ifndef CTL_CONTAINER_STATICLIST_H
#define CTL_CONTAINER_STATICLIST_H

#include "../templates.hpp"
#include "../ctypes.hpp"
#include "../cpperror.hpp"
#include "../typetraits/traits.hpp"
#include "arguments.hpp"
#include "iterator.hpp"
#include "function.hpp"
#include "span.hpp"
#include "../algorithm/sort.hpp"
#include "../algorithm/reverse.hpp"
#include "../adapter/comparator.hpp"
#include "../memory/memory.hpp"

CTL_NAMESPACE_BEGIN

template<
	class TData,
	Type::Integer TIndex = usize,
	template <class> class TAlloc = HeapAllocator
>
struct StaticList;

/// @brief Container-specific type constraints.
namespace Type::Container {
	/// @brief Implementation of type constraints.
	namespace Impl {
		template<class T>
		struct IsStaticList;

		template<template <class, class, template <class> class> class T0, class T1, class T2, template <class> class T3>
		struct IsStaticList<T0<T1, T2, T3>>: BooleanConstant<Type::Equal<T0<T1, T2, T3>, ::CTL::StaticList<T1, T2, T3>>> {};
	}

	/// Type must be `StaticList`.
	template<class T>
	concept StaticList = Impl::IsStaticList<T>::value; 
}

/// @brief Static-sized, heap-allocated array of objects.
/// @tparam TData Element type.
/// @tparam TIndex Index type.
/// @tparam TAlloc<class> Allocator type.
/// @note
///		This list's capacity cannot automatically grow.
///		If capacity is changed (via `resize`), its previous contents are cleared.
template<
	class TData,
	Type::Integer TIndex,
	template <class> class TAlloc
>
struct StaticList:
	Iteratable<TData, TIndex>,
	SelfIdentified<StaticList<TData, TIndex>>,
	ContextAwareAllocatable<TAlloc, TData>,
	Ordered {
public:
	using Iteratable				= ::CTL::Iteratable<TData, TIndex>;
	using SelfIdentified			= ::CTL::SelfIdentified<StaticList<TData, TIndex>>;
	using ContextAwareAllocatable	= ::CTL::ContextAwareAllocatable<TAlloc, TData>;

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

	using PredicateType	= Decay::AsFunction<bool(ConstReferenceType)>;
	using CompareType	= Decay::AsFunction<bool(ConstReferenceType, ConstReferenceType)>;

	using ComparatorType = SimpleComparator<DataType>;

	/// Default constructor.
	constexpr StaticList(): count(0) {invoke(1);}

	/// @brief Constructs the `StaticList` with a preallocated capacity.
	/// @param size Size to preallocate.
	constexpr explicit StaticList(SizeType const size) {
		invoke(size);
	}

	/// @brief Constructs a `StaticList` of a given size.
	/// @tparam ...Args Argument types.
	/// @param size StaticList size.
	/// @param ...args Arguments to pass to element's constructor.
	template<class... Args>
	constexpr explicit StaticList(SizeType const size, Args... args) {
		invoke(size);
		for (usize i = 0; i < size; ++i)
			MX::construct(&contents[i], args...);
		count = size;
	}
	
	/// @brief Constructs the `StaticList` with a parameter pack.
	/// @tparam ...Args Parameter pack.
	/// @param ...args Pack elements.
	template<typename... Args>
	constexpr StaticList(Args&&... args)
	requires (... && Type::Equal<Args, DataType>) {
		invoke(sizeof...(Args));
		(..., constructBack(CTL::move(args)));
	}

	/// @brief Move constructor.
	/// @param other `StaticList` to move from.
	constexpr StaticList(SelfType&& other) {
		maximum			= ::CTL::move(other.maximum);
		contents		= ::CTL::move(other.contents);
		count			= ::CTL::move(other.count);
		other.contents	= nullptr;
	}

	/// Destructor.
	constexpr ~StaticList() {dump();}

	/// @brief Constructs and adds new element to the end of the `StaticList`. 
	/// @tparam ...Args Argument types.
	/// @param ...args Values to pass to constructor.
	/// @return Reference to self.
	template<class... Args>
	constexpr SelfType& constructBack(Args... args) {
		if (count >= maximum)
			capacityReachedError();
		MX::construct(contents+(count++), args...);
		return *this;
	}

	/// @brief Removes an element from the end of the `StaticList`.
	/// @return Value of the element removed.
	/// @throw OutOfBoundsException when `StaticList` is empty.
	constexpr DataType popBack() {
		if (empty()) emptyError();
		DataType value = back();
		count--;
		if (count) MX::destruct<DataType>(contents+count);
		return value;
	}

	/// @brief Resizes the `StaticList`, so the capacity is of a given size.
	/// @param newSize New `StaticList` size.
	/// @return Reference to self.
	/// @note Will destroy previously-held elements.
	constexpr SelfType& resize(SizeType const newSize) {
		if (!newSize) return clear();
		if (contents) memdestroy(contents, count);
		contents = memcreate(newSize);
		maximum = newSize;
		count = 0;
		return *this;
	}
	
	/// @brief Resizes the `StaticList`, so the capacity is of a given size, then sets current size to it.
	/// @tparam ...Args Argument types.
	/// @param newSize New `StaticList` size.
	/// @param ...args Arguments to pass to the object constructor.
	/// @return Reference to self.
	/// @note Will destroy previously-held elements, and replace them with newly-constructed ones, with the given `args`.
	template<class... Args>
	constexpr SelfType& resize(SizeType const newSize, Args... args) {
		if (!newSize) return clear();
		resize(newSize);
		if (newSize > count)
			for (SizeType i = count; i < newSize; ++i)
				MX::construct(&contents[i], args...);
		count = newSize;
		return *this;
	}

	/// @brief Finds the the position of the first element that matches a value.
	/// @param value Value to search for.
	/// @return The index of the value, or -1 if not found.
	constexpr IndexType find(DataType const& value) const
	requires Type::Comparator::Equals<DataType, DataType> {
		if (empty()) return -1;
		auto const start = begin(), stop = end();
		for (auto i = start; i != stop; ++i)
			if (ComparatorType::equals(*i, value))
				return i-start;
		return -1;
	}

	/// @brief Finds the the position of the last element that matches a value.
	/// @param value Value to search for.
	/// @return The index of the value, or -1 if not found.
	constexpr IndexType rfind(DataType const& value) const
	requires Type::Comparator::Equals<DataType, DataType> {
		if (empty()) return -1;
		auto const start = rbegin(), stop = rend();
		for (auto i = start; i != stop; ++i)
			if (ComparatorType::equals(*i, value))
				return count-(i-start)-1;
		return -1;
	}

	/// @brief Performs a binary search to find the index of an element that matches a value.
	/// @param value Value to search for.
	/// @return The index of the value, or -1 if not found.
	/// @note Requires the array to be sorted.
	constexpr IndexType bsearch(DataType const& value) const
	requires (Type::Comparator::Threeway<DataType, DataType>) {
		if (empty()) return -1;
		if (ComparatorType::equals(front(), value)) return 0;
		if (ComparatorType::equals(back(), value)) return size() - 1;
		IndexType lo = 0, hi = size() - 1, i = -1;
		SizeType loop = 0;
		while (hi >= lo && loop < size()) {
			i = lo + (hi - lo) / 2;
			switch(ComparatorType::compare(value, *(cbegin() + i))) {
				case Order::LESS:		hi = i-1; break;
				case Order::EQUAL:		return i;
				case Order::GREATER:	lo = i+1; break;
				default:
				case Order::UNORDERED:	return -1;
			}
		}
		return -1;
	}

	/// @brief Clears the `StaticList`.
	/// @return Reference to self.
	/// @note
	///		Does not free the underlying array held by the `StaticList`.
	///		To actually free the underlying array, call `dispose`. 
	constexpr SelfType& clear() {
		memdestruct(contents, count);
		count = 0;
		return *this;
	}

	/// @brief Frees the underlying array held by the `StaticList`.
	/// @return Reference to self.
	/// @note To not free the underlying array, call `clear`. 
	constexpr SelfType& dispose() {
		dump();
		return *this;
	}

	/// @brief Move assignment operator.
	/// @param other Other `StaticList`.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType&& other) {
		memdestroy(contents, count);
		maximum			= CTL::move(other.maximum);
		contents		= CTL::move(other.contents);
		count			= CTL::move(other.count);
		other.contents	= nullptr;
		return *this;
	}

	/// @brief Returns the value of the element at a given index.
	/// @param index Index of the element.
	/// @return Reference to the element.
	/// @throw OutOfBoundsException when index is bigger than `StaticList` size.
	constexpr ReferenceType	operator[](IndexType index)						{return at(index);}
	/// @brief Returns the value of the element at a given index.
	/// @param index Index of the element.
	/// @return Value of the element.
	/// @throw OutOfBoundsException when index is bigger than `StaticList` size.
	constexpr ConstReferenceType operator[](IndexType const index) const	{return at(index);}

	/// @brief Returns the current element count.
	/// @return Element count.
	constexpr SizeType size() const		{return count;		}
	/// @brief Returns the current size of the underlying array.
	/// @return Size of the underlying array.
	constexpr SizeType capacity() const	{return maximum;	}
	/// @brief Returns whether the list is empty.
	/// @return Whether the array is list.
	constexpr SizeType empty() const	{return count == 0;	}

	/// @brief Returns a pointer to the underlying array.
	/// @return Pointer to the underlying array.
	constexpr PointerType		data()			{return contents;	}
	/// @brief Returns a pointer to the underlying array.
	/// @return Pointer to the underlying array.
	constexpr ConstPointerType	data() const	{return contents;	}

	/// @brief Returns an iterator to the beginning of the `StaticList`.
	/// @return Iterator to the beginning of the `StaticList`.
	constexpr IteratorType		begin()			{return contents;		}
	/// @brief Returns an iterator to the end of the `StaticList`.
	/// @return Iterator to the end of the `StaticList`.
	constexpr IteratorType		end()			{return contents+count;	}
	/// @brief Returns an iterator to the beginning of the `StaticList`.
	/// @return Iterator to the beginning of the `StaticList`.
	constexpr ConstIteratorType	begin() const	{return contents;		}
	/// @brief Returns an iterator to the end of the `StaticList`.
	/// @return Iterator to the end of the `StaticList`.
	constexpr ConstIteratorType	end() const		{return contents+count;	}

	/// @brief Returns a reverse iterator to the beginning of the `StaticList`.
	/// @return Reverse iterator to the beginning of the `StaticList`.
	constexpr ReverseIteratorType		rbegin()		{return ReverseIteratorType(contents+count);		}
	/// @brief Returns a reverse iterator to the end of the `StaticList`.
	/// @return Reverse iterator to the end of the `StaticList`.
	constexpr ReverseIteratorType		rend()			{return ReverseIteratorType(contents);				}
	/// @brief Returns a reverse iterator to the beginning of the `StaticList`.
	/// @return Reverse iterator to the beginning of the `StaticList`.
	constexpr ConstReverseIteratorType	rbegin() const	{return ConstReverseIteratorType(contents+count);	}
	/// @brief Returns a reverse iterator to the end of the `StaticList`.
	/// @return Reverse iterator to the end of the `StaticList`.
	constexpr ConstReverseIteratorType	rend() const	{return ConstReverseIteratorType(contents);			}

	/// @brief Returns a pointer to the beginning of the `StaticList`.
	/// @return Pointer to the beginning of the `StaticList`.
	constexpr PointerType	cbegin()			{return contents;		}
	/// @brief Returns a pointer to the end of the `StaticList`.
	/// @return Pointer to the end of the `StaticList`.
	constexpr PointerType	cend()				{return contents+count;	}
	/// @brief Returns a pointer to the beginning of the `StaticList`.
	/// @return Pointer to the beginning of the `StaticList`.
	constexpr ConstPointerType	cbegin() const	{return contents;		}
	/// @brief Returns a pointer to the end of the `StaticList`.
	/// @return Pointer to the end of the `StaticList`.
	constexpr ConstPointerType	cend() const	{return contents+count;	}
	
	/// @brief Returns the value of the first element.
	/// @return Reference to the first element.
	/// @throw OutOfBoundsException when `StaticList` is empty.
	constexpr ReferenceType		front()			{return at(0);			}
	/// @brief Returns the value of the last element.
	/// @return Reference to the last element.
	/// @throw OutOfBoundsException when `StaticList` is empty.
	constexpr ReferenceType 	back()			{return at(count-1);	}
	/// @brief Returns the value of the first element.
	/// @return Value of the first element.
	/// @throw OutOfBoundsException when `StaticList` is empty.
	constexpr DataType			front() const	{return at(0);			}
	/// @brief Returns the value of the last element.
	/// @return Value of the last element.
	/// @throw OutOfBoundsException when `StaticList` is empty.
	constexpr DataType			back() const	{return at(count-1);	}

	/// @brief Returns the value of the element at a given index.
	/// @param index Index of the element.
	/// @return Reference to the element.
	/// @throw OutOfBoundsException when `StaticList` is empty.
	constexpr DataType& at(IndexType index) {
		if (!count) emptyError();
		assertIsInBounds(index);
		wrapBounds(index, count);
		return contents[index];
	}

	/// @brief Returns the value of the element at a given index.
	/// @param index Index of the element.
	/// @return Value of the element.
	/// @throw OutOfBoundsException when index is bigger than `StaticList` size.
	constexpr DataType const& at(IndexType index) const {
		if (!count) emptyError();
		assertIsInBounds(index);
		wrapBounds(index, count);
		return contents[index];
	}

	/// @brief Returns whether all elements match a given predicate.
	/// @tparam TPredicate Predicate type.
	/// @param cond Predicate to match.
	/// @return Whether all elements match.
	template<class TPredicate>
	constexpr bool validate(TPredicate const& cond) const {
		if (!count) return false;
		for (DataType const& c: *this)
			if (!cond(c))
				return false;
		return true;
	}

	/// @brief Apllies a procedure to all elements of the `StaticList`.
	/// @tparam TProcedure Procedure type.
	/// @param fun Procedure to apply.
	/// @return Reference to self.
	template <class TProcedure>
	constexpr SelfType& transform(TProcedure const& fun) {
		for(DataType& v: *this)
			v = fun(v);
		return *this;
	}

	/// @brief Returns whether this `StaticList` is equal to another `StaticList`.
	/// @param other Other `StaticList` to compare with.
	/// @return Whether they're equal.
	/// @note Requires element type to be equally comparable.
	/// @sa Comparator::equals()
	constexpr SizeType equals(SelfType const& other) const
	requires Type::Comparator::Equals<DataType, DataType> {
		bool result = true;
		SizeType i = 0;
		while (result) {
			if (i == count || i == other.count)
				return count == other.count;
			result = ComparatorType::equals(contents[i], other.contents[i]);
			++i;
		}
		return result;
	}

	/// @brief Returns the result of a threeway comparison with another `StaticList`.
	/// @param other Other `StaticList` to compare with.
	/// @return Order between both `StaticList`s.
	/// @note Requires element type to be threeway comparable.
	/// @sa Comparator::compare()
	constexpr OrderType compare(SelfType const& other) const
	requires Type::Comparator::Threeway<DataType, DataType> {
		OrderType result = Order::EQUAL;
		SizeType i = 0;
		while (result == Order::EQUAL) {
			if (i == count || i == other.count)
				return count <=> other.count;
			result = ComparatorType::compare(contents[i], other.contents[i]);
			++i;
		}
		return result;
	}

	/// @brief Returns how different this `StaticList` is from another `StaticList`.
	/// @param other Other `StaticList` to compare with.
	/// @return How different it is.
	/// @note
	///		Compares elements with equivalent positions.
	///		Returns the amount of different elements, plus the size difference.
	constexpr SizeType disparity(SelfType const& other) const
	requires Type::Comparator::Equals<DataType, DataType> {
		SizeType
			diff	= 0,
			max		= (count > other.count ? count : other.count),
			min		= (count < other.count ? count : other.count)
		;
		for (SizeType i = 0; i < max; ++i)
			if (!ComparatorType::equals(contents[i], other.contents[i])) ++diff;
		return diff + (max - min);
	}

	/// @brief Equality operator.
	/// @param other Other `StaticList` to compare with.
	/// @return Whether they're equal.
	/// @note Requires element type to be equally comparable.
	/// @sa Comparator::equals()
	constexpr bool operator==(SelfType const& other) const
	requires Type::Comparator::Equals<DataType, DataType> {
		return equals(other);
	}

	/// @brief Threeway comparison operator.
	/// @param other Other `StaticList` to compare with.
	/// @return Order between both `StaticList`s.
	/// @note Requires element type to be threeway comparable.
	/// @sa Comparator::compare()
	constexpr OrderType operator<=>(SelfType const& other) const
	requires Type::Comparator::Threeway<DataType, DataType> {
		return compare(other);
	}

	/// @brief `swap` algorithm for `StaticList`.
	/// @param a `StaticList` to swap.
	/// @param b `StaticList` to swap with.
	friend constexpr void swap(SelfType& a, SelfType& b) noexcept {
		swap(a.contents, b.contents);
		swap(a.maximum, b.maximum);
		swap(a.count, b.count);
	}

protected:
	using
		ContextAwareAllocatable::contextAllocate,
		ContextAwareAllocatable::contextDeallocate
	;

private:
	using Iteratable::wrapBounds;

	constexpr void dump() {
		memdestroy(contents, count);
		contents	= nullptr;
		maximum		= 0;
		count		= 0;
	}

	constexpr static void memdestruct(ref<DataType> const& p, SizeType const sz) {
		if (!(sz && p)) return;
		if constexpr (!Type::Standard<DataType>) {
			for (auto i = p; i != (p+sz); ++i)
				MX::destruct(i);
		}
	}

	constexpr void memdestroy(owner<DataType> const& p, SizeType const sz) {
		if (!p) return;
		memdestruct(p, sz);
		contextDeallocate(p, sz);
	}

	constexpr owner<DataType> memcreate(SizeType const sz) {
		return contextAllocate(sz);
	}

	constexpr SelfType& invoke(SizeType const size) {
		if (contents) return *this;
		else contents = memcreate(size);
		maximum = size;
		return *this;
	}

	constexpr void assertIsInBounds(IndexType const index) const {
		if (index > IndexType(count-1)) outOfBoundsError();
	}

	[[noreturn]] constexpr static void capacityReachedError() {
		throw OutOfBoundsException("Maximum list capacity reached!");
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

	/// @brief True underlying array size.
	SizeType		maximum		= 0;
	/// @brief Element count.
	SizeType		count		= 0;
	/// @brief Underlying array.
	owner<DataType>	contents	= nullptr;
};

CTL_NAMESPACE_END

#endif