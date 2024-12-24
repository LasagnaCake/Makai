#ifndef CTL_CONTAINER_LIST_H
#define CTL_CONTAINER_LIST_H

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
struct List;

/// @brief Container-specific type constraints.
namespace Type::Container {
	/// @brief Implementation of type constraints.
	namespace Impl {
		template<class T>
		struct IsList;

		template<template <class, class, template <class> class> class T0, class T1, class T2, template <class> class T3>
		struct IsList<T0<T1, T2, T3>>: BooleanConstant<Type::Equal<T0<T1, T2, T3>, ::CTL::List<T1, T2, T3>>> {};
	}

	/// Type must be `List`.
	template<class T>
	concept List = Impl::IsList<T>::value;
}

/// @brief Dynamic array of objects.
/// @tparam TData Element type.
/// @tparam TIndex Index type.
/// @tparam TAlloc<class> Allocator type.
template<
	class TData,
	Type::Integer TIndex,
	template <class> class TAlloc
>
struct List:
	Iteratable<TData, TIndex>,
	SelfIdentified<List<TData, TIndex>>,
	ListInitializable<TData>,
	Allocatable<TAlloc, TData>,
	Ordered {
public:
	using Iteratable		= ::CTL::Iteratable<TData, TIndex>;
	using SelfIdentified	= ::CTL::SelfIdentified<List<TData, TIndex>>;
	using ListInitializable	= ::CTL::ListInitializable<TData>;
	using Allocatable		= ::CTL::Allocatable<TAlloc, TData>;

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
		typename ListInitializable::ArgumentListType
	;

	using
		typename Allocatable::AllocatorType
	;

	using PredicateType	= Decay::AsFunction<bool(ConstReferenceType)>;
	using CompareType	= Decay::AsFunction<bool(ConstReferenceType, ConstReferenceType)>;

	using ComparatorType = SimpleComparator<DataType>;

	/// Empty constructor.
	constexpr List() {invoke(1);}

	/// @brief Constructs a `List` with a preallocated capacity.
	/// @param size Size to preallocate.
	constexpr List(SizeType const& size) {
		invoke(size);
	}

	/// @brief Constructs a `List` of a given size and a given fill.
	/// @param size Size to allocate.
	/// @param fill Value to set for elements.
	constexpr List(SizeType const& size, DataType const& fill) {
		invoke(size);
		for (usize i = 0; i < size; ++i)
			contents[i] = fill;
		count = size;
	}

	/// @brief Constructs a `List` with a given argument list.
	/// @param values Values to add to `List`.
	constexpr List(ArgumentListType const& values) {
		invoke(values.size());
		for (DataType const& v: values) pushBack(v);
	}
	
	/// @brief Constructs a `List` with a parameter pack.
	/// @tparam ...Args Parameter pack.
	/// @param ...args Pack elements.
	template<typename... Args>
	constexpr List(Args const&... args)
	requires (... && Type::Convertible<Args, DataType>) {
		invoke(sizeof...(Args));
		(..., pushBack(args));
	}

	/// @brief Constructs a `List` from a fixed array of elements.
	/// @tparam S Size of array.
	/// @param values Elements to add to `List`.
	template<SizeType S>
	constexpr explicit List(Decay::AsType<ConstantType[S]> const& values) {
		invoke(S);
		copy(values, contents, S);
		count = S;
	}

	/// @brief Copy constructor.
	/// @param other `List` to copy from.
	constexpr List(SelfType const& other) {
		invoke(other.maximum);
		copy(other.contents, contents, other.count);
		count = other.count;
	}

	/// @brief Move constructor.
	/// @param other `List` to move from.
	constexpr List(SelfType&& other) {
		maximum			= ::CTL::move(other.maximum);
		contents		= ::CTL::move(other.contents);
		count			= ::CTL::move(other.count);
		magnitude		= ::CTL::move(other.magnitude);
		other.contents	= nullptr;
	}

	/// @brief Constructs a `List` from a range of values.
	/// @tparam T2 Type of object the iterator points to (different than `List`'s own data type).
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator to end of range.
	template<Type::Convertible<DataType> T2>
	constexpr explicit List(ForwardIterator<T2 const> const& begin, ForwardIterator<T2 const> const& end)
	requires Type::Different<T2, DataType> {
		if (end <= begin) return;
		invoke(end - begin + 1);
		copy(begin, contents, end - begin);
		count = end - begin;
	}

	/// @brief Constructs a `List` from a range of values.
	/// @tparam T2 Type of object the iterator points to (different than `List`'s own data type).
	/// @param begin Reverse iterator to beginning of range.
	/// @param end Reverse iterator to end of range.
	template<Type::Convertible<DataType> T2>
	constexpr explicit List(ReverseIterator<T2 const> const& begin, ReverseIterator<T2 const> const& end)
	requires Type::Different<T2, DataType> {
		if (end <= begin) return;
		invoke(end - begin + 1);
		for (auto i = begin; i != end; ++i)
			pushBack(*i);
		count = end - begin;
	}
	
	/// @brief Constructs a `List` from a range of values.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator to end of range.
	constexpr List(ConstIteratorType const& begin, ConstIteratorType const& end) {
		if (end <= begin) return;
		invoke(end - begin + 1);
		copy(begin, contents, end - begin);
		count = end - begin;
	}

	/// @brief Constructs a `List` from a range of elements.
	/// @param begin Reverse iterator to beginning of range.
	/// @param end Reverse iterator to end of range.
	constexpr List(ConstReverseIteratorType const& begin, ConstReverseIteratorType const& end) {
		if (end <= begin) return;
		invoke(end - begin + 1);
		for (auto i = begin; i != end; ++i)
			pushBack(*i);
		count = end - begin;
	}

	/// @brief Constructs a `List` from a "C-style" range of elements.
	/// @param start Start of range.
	/// @param size Size of range.
	constexpr List(ConstPointerType const& start, SizeType const& size): List(start, start + size) {}

	/// @brief Constructs a `List`, from a ranged object of (non-subclass) type T.
	/// @tparam T Ranged type.
	/// @param other Object to copy from.
	template<Type::Container::Ranged<IteratorType, ConstIteratorType> T>
	constexpr explicit List(T const& other)
	requires requires {
//		requires !Type::Constructible<T, ConstIteratorType, ConstIteratorType>;
		requires !Type::Subclass<T, SelfType>;
	}: List(other.begin(), other.end()) {}

	/// @brief Constructs a `List`, from a bounded object of (non-list) type T.
	/// @tparam T Ranged type.
	/// @param other Object to copy from.
	template<Type::Container::Bounded<PointerType, SizeType> T>
	constexpr explicit List(T const& other)
	requires requires {
//		requires !Type::Constructible<T, ConstIteratorType, ConstIteratorType>;
		requires !Type::Container::List<T>;
		requires !Type::Container::Ranged<T, IteratorType, ConstIteratorType>;
	}: List(other.data(), other.size()) {}

	/// @brief Constructs a `List` from a list of ranged objects.
	/// @tparam T Object type of `List`.
	/// @param other `List` to copy from.
	template<class T>
	constexpr explicit List(List<T, SizeType> const& other)
	requires requires (T t) {
		requires Type::Constructible<DataType, ConstIteratorType, ConstIteratorType>;
		requires !Type::Subclass<T, SelfType>;
	} {
		invoke(other.size());
		for (auto& v: other)
			pushBack(DataType(v.begin(), v.end()));
	}

	/// Destructor.
	constexpr ~List() {dump();}

	/// @brief Adds a new element to the end of the `List`. 
	/// @param value Element to add.
	/// @return Reference to self.
	constexpr SelfType& pushBack(DataType const& value) {
		if (count >= maximum)
			increase();
		MX::construct(contents+(count++), value);
		return *this;
	}

	/// @brief Removes an element from the end of the `List`.
	/// @return Value of the element removed.
	/// @throw OutOfBoundsException when `List` is empty.
	constexpr DataType popBack() {
		DataType value = back();
		count--;
		if (count) MX::destruct<DataType>(contents+count);
		return value;
	}

	/// @brief Inserts an element at a specified index in the `List`.
	/// @param value Value of the element to insert.
	/// @param index Index of which to insert in.
	/// @return Reference to self.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	/// @note If index is negative, it will be interpreted as starting from the end of the `List`.
	constexpr SelfType& insert(DataType const& value, IndexType index) {
		assertIsInBounds(index);
		wrapBounds(index, count);
		if (count >= maximum) increase();
		copy(&contents[index], &contents[index+1], count - index);
		MX::construct(contents+index, value);
		++count;
		return *this;
	}

	/// @brief Inserts a `List` of elements at a specified index in the `List`.
	/// @param other `List` to insert.
	/// @param index Index of which to insert in.
	/// @return Reference to self.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	/// @note If index is negative, it will be interpreted as starting from the end of the `List`.
	constexpr SelfType& insert(SelfType const& other, IndexType index) {
		assertIsInBounds(index);
		wrapBounds(index, count);
		expand(other.count);
		copy(&contents[index], &contents[index+other.count], count - index);
		copy(other.contents, &contents[index], other.count);
		count += other.count;
		return *this;
	}

	/// @brief Inserts a fixed array of elements at a specified index in the `List`.
	/// @tparam S Size of fixed array.
	/// @param values Array of element to insert.
	/// @param index Index of which to insert in.
	/// @return Reference to self.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	/// @note If index is negative, it will be interpreted as starting from the end of the `List`.
	template<SizeType S>
	constexpr SelfType& insert(Decay::AsType<ConstantType[S]> const& values, IndexType const& index) {
		return insert(SelfType(values), index);
	}

	/// @brief Inserts a given value, a given amount of times, at a specified index in the `List`.
	/// @param value Value to be inserted.
	/// @param count Amount of times to insert the element.
	/// @param index Index of which to insert in.
	/// @return Reference to self.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	/// @note If index is negative, it will be interpreted as starting from the end of the `List`.
	constexpr SelfType& insert(DataType const& value, SizeType const& count, IndexType const& index) {
		return insert(SelfType(count, value), index);
	}

	/// @brief Ensures the `List` can hold AT LEAST a given capacity.
	/// @param count Minimum size of the `List`.
	/// @return Reference to self.
	/// @note
	///		This guarantees the capacity will be AT LEAST `count`,
	/// 	but does not guarantee the capacity will be EXACTLY `count`.
	///		For that, use `resize`.
	constexpr SelfType& reserve(SizeType const& count) {
		while (count >= maximum)
			increase();
		return *this;
	}

	/// @brief Resizes the `List`, so the capacity is of a given size.
	/// @param newSize New `List` size.
	/// @return Reference to self.
	/// @note
	///		This guarantees the capacity will be EXACTLY of `newSize`.
	/// 	If you need the capacity to be AT LEAST `newSize`, use `reserve`.
	constexpr SelfType& resize(SizeType const& newSize) {
		if (!newSize) return clear();
		if (contents)	memresize(contents, newSize, maximum, count);
		else			contents = memcreate(newSize);
		maximum = newSize;
		if (count > newSize)
			count = newSize;
		recalculateMagnitude();
		return *this;
	}

	/// @brief Expands the `List`, such that it can hold AT LEAST `size()` + `count`.
	/// @param count Count to increase by.
	/// @return Reference to self.
	constexpr SelfType& expand(SizeType const& count) {
		if (!count) return *this;
		reserve(this->count + count);
		return *this;
	}

	/// @brief
	///		Ensures the `List` can hold AT LEAST a given capacity.
	/// @param count Minimum size of the `List`.
	/// @param fill Value to use as fill.
	/// @return Reference to self.
	/// @note
	///		If current size is smaller,
	///		then it fills the extra space added with the given `fill`,
	///		up to `count`, and sets current size to it.
	/// @note
	///		This guarantees the capacity will be AT LEAST `count`,
	/// 	but does not guarantee the capacity will be EXACTLY `count`.
	///		For that, use `resize`.
	constexpr SelfType& reserve(SizeType const& count, DataType const& fill) {
		reserve(count);
		if (count > this->count) {
			for (SizeType i = this->count; i < count; ++i)
				contents[i] = fill;
			this->count = count;
		}
		return *this;
	}

	/// @brief Resizes the `List`, so the capacity is of a given size, then sets current size to it.
	/// @param newSize New `List` size.
	/// @param fill Value to use as fill.
	/// @return Reference to self.
	///	@note
	///		If current size is smaller,
	///		then it fills the extra space added with the given `fill`.
	/// @note
	///		This guarantees the capacity will be EXACTLY of `newSize`.
	///		If you need the capacity to be AT LEAST `newSize`, use `reserve`.
	constexpr SelfType& resize(SizeType const& newSize, DataType const& fill) {
		if (!newSize) return clear();
		resize(newSize);
		if (newSize > count)
			for (SizeType i = count; i < newSize; ++i)
				contents[i] = fill;
		count = newSize;
		return *this;
	}

	/// @brief
	///		Expands the `List`, such that it can hold AT LEAST the current size,
	///		plus a given `count`.
	/// @param count Count to increase by.
	/// @param fill Value to use as fill.
	/// @return Reference to self.
	///	@note
	///		If current size is smaller,
	///		then it fills the extra space added with the given `fill`.
	constexpr SelfType& expand(SizeType count, DataType const& fill) {
		expand(count);
		while (count-- > 0) pushBack(fill);
		return *this;
	}
	
	/// @brief Ensures the current capacity is EXACTLY the current size.
	/// @return Reference to self. 
	constexpr SelfType& tighten() {
		resize(count);
		return *this;
	}

	/// @brief Reverses the `List`.
	/// @return Reference to self.
	constexpr SelfType& reverse() {
		::CTL::reverse(contents, count);
		return *this;
	}

	/// @brief Returns a reversed copy of the `List`.
	/// @return A reversed copy of the `List`.
	constexpr SelfType reversed() const {
		return SelfType(*this).reverse();
	}

	/// @brief Sorts the current `List`.
	/// @return Reference to self.
	/// @note This function only exists if the object type is a sortable type.
	constexpr SelfType& sort()
	requires Type::Algorithm::Sortable<DataType> {
		static_assert(Type::Algorithm::SortableIterator<IteratorType>);
		::CTL::sort(begin(), end());
		return *this;
	}

	/// @brief Returns a sorted copy of the `List`.
	/// @return Sorted copy of the `List`.
	/// @note This function only exists if the object type is a sortable type.
	constexpr SelfType sorted() const
	requires Type::Algorithm::Sortable<DataType> {
		static_assert(Type::Algorithm::SortableIterator<IteratorType>);
		return SelfType(*this).sort();
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

	/// @brief Removes an element at a given index.
	/// @param index Index of the element to remove.
	/// @return Reference to self.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	/// @note
	///		Does not resize `List`, merely moves it to the end, and destructs it.
	///		If you need the `List` size to change, use `erase`.
	constexpr SelfType& remove(IndexType index) {
		assertIsInBounds(index);
		wrapBounds(index, count);
		return squash(index);
	}

	/// @brief Removes elements that match a given value.
	/// @param value Value to match.
	/// @return Count of elements removed.
	/// @note
	///		Does not resize `List`, merely moves it to the end, and destructs it.
	///		If you need the `List` size to change, use `erase`.
	constexpr SizeType removeLike(DataType const& value)
	requires Type::Comparator::Equals<DataType, DataType> {
		if (empty()) return 0;
		SizeType removed = 0;
		SizeType const ocount = count;
		auto const start = begin();
		for(auto i = begin(); i < end();)
			if (ComparatorType::equals(*i, value)) {
				squash(i-start);
				++removed;
				--count;
			} else ++i;
		count = ocount;
		return removed;
	}

	/// @brief Removes elements that do not match a given value.
	/// @param value Value to match.
	/// @return Count of elements removed.
	/// @note
	///		Does not resize `List`, merely moves it to the end, and destructs it.
	///		If you need the `List` size to change, use `erase`. 
	constexpr SizeType removeUnlike(DataType const& value)
	requires Type::Comparator::Equals<DataType, DataType> {
		if (empty()) return 0;
		SizeType removed = 0;
		SizeType const ocount = count;
		auto const start = begin();
		for(auto i = begin(); i < end();)
			if (!ComparatorType::equals(*i, value)) {
				squash(i-start);
				++removed;
				--count;
			} else ++i;
		count = ocount;
		return removed;
	}

	/// @brief Removes elements that match a given predicate.
	/// @tparam TPredicate Predicate type.
	/// @param predicate Predicate to use as check.
	/// @return Count of objects elements.
	/// @note
	///		Does not resize `List`, merely moves it to the end, and destructs it.
	///		If you need the `List` size to change, use `erase`. 
	template<class TPredicate>
	constexpr SizeType removeIf(TPredicate const& predicate) {
		if (empty()) return 0;
		SizeType removed = 0;
		SizeType const ocount = count;
		auto const start = begin();
		for(auto i = begin(); i < end();)
			if (predicate(*i)) {
				squash(i-start);
				++removed;
				--count;
			} else ++i;
		count = ocount;
		return removed;
	}

	/// @brief Removes elements that do not match a given predicate.
	/// @tparam TPredicate Predicate type.
	/// @param predicate Predicate to use as check.
	/// @return Count of objects elements.
	/// @note
	///		Does not resize `List`, merely moves it to the end, and destructs it.
	///		If you need the `List` size to change, use `erase`. 
	template<class TPredicate>
	constexpr SizeType removeIfNot(TPredicate const& predicate) {
		if (empty()) return 0;
		SizeType removed = 0;
		SizeType const ocount = count;
		auto const start = begin();
		for(auto i = start; i < end();)
			if (!predicate(*i)) {
				squash(i-start);
				++removed;
				--count;
			} else ++i;
		count = ocount;
		return removed;
	}

	/// @brief Erases an element at a given index.
	/// @param index Index of the element to erase.
	/// @return Reference to self.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	/// @note
	///		Resizes the `List`.
	///		If you need the `List` size to remain the same, use `remove`. 
	constexpr SelfType& erase(IndexType const& index) {
		if (empty()) return 0;
		remove(index);
		count--;
		return *this;
	}

	/// @brief Erases elements that match a given value.
	/// @param value Value to match.
	/// @return Count of elements removed.
	/// @note
	///		Resizes the `List`.
	///		If you need the `List` size to remain the same, use `remove`. 
	constexpr SelfType& eraseLike(DataType const& value) {
		count -= removeLike(value);
		return *this;
	}

	/// @brief Erases elements that do not a given value.
	/// @param value Value to match.
	/// @return Count of elements removed.
	/// @note
	///		Resizes the `List`.
	///		If you need the `List` size to remain the same, use `remove`.
	constexpr SelfType& eraseUnlike(DataType const& value) {
		count -= removeUnlike(value);
		return *this;
	}

	/// @brief Erases elements that match a given predicate.
	/// @tparam TPredicate Predicate type.
	/// @param predicate Predicate to use as check.
	/// @note
	///		Resizes the `List`.
	///		If you need the `List` size to remain the same, use `remove`.
	template<class TPredicate>
	constexpr SelfType& eraseIf(TPredicate const& predicate) {
		count -= removeIf(predicate);
		return *this;
	}

	/// @brief Erases elements that do not match a given predicate.
	/// @tparam TPredicate Predicate type.
	/// @param predicate Predicate to use as check.
	/// @note
	///		Resizes the `List`.
	///		If you need the `List` size to remain the same, use `remove`.
	template<class TPredicate>
	constexpr SelfType& eraseIfNot(TPredicate const& predicate) {
		count -= removeIfNot(predicate);
		return *this;
	}

	/// @brief Returns a `List` containing all elements starting from a given index.
	/// @param start Starting index to copy from.
	/// @return `List` containing elements starting from the given `start`.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	/// @note If index is negative, it will be interpreted as starting from the end of the `List`.
	constexpr SelfType sliced(IndexType start) const {
		if (IndexType(count) < start) return SelfType();
		assertIsInBounds(start);
		wrapBounds(start, count);
		return SelfType(cbegin() + start, cend());
	}

	/// @brief Returns a `List` containing all elements located between two indices.
	/// @param start Starting index to copy from.
	/// @param stop End index to stop copying from.
	/// @return `List` containing elements between `start` and `stop`.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	/// @note If index is negative, it will be interpreted as starting from the end of the `List`.
	constexpr SelfType sliced(IndexType start, IndexType stop) const {
		if (IndexType(count) < start) return SelfType();
		assertIsInBounds(start);
		wrapBounds(start, count);
		if (IndexType(count) < stop) return sliced(start);
		wrapBounds(stop, count);
		if (stop < start) return SelfType();
		return SelfType(cbegin() + start, cbegin() + stop + 1);
	}

	/// @brief Returns the current `List`, divided at a given index.
	/// @param index The index to use as pivot.
	/// @return A `List` containing the two halves of this `List`.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	/// @note If index is negative, it will be interpreted as starting from the end of the `List`.
	constexpr List<SelfType, SizeType> divide(IndexType index) const {
		List<SelfType, SizeType> res;
		assertIsInBounds(res);
		wrapBounds(index, count);
		res.pushBack(sliced(0, index));
		res.pushBack(sliced(index+1));
		return res;
	}

	/// @brief Appends another `List` to the end of the `List`.
	/// @param other List to copy contents from.
	/// @return Reference to self.
	constexpr SelfType& appendBack(SelfType const& other) {
		expand(other.count);
		copy(other.contents, contents + count, other.count);
		count += other.count;
		return *this;
	}

	/// @brief Appends a list of elements to the end of the `List`.
	/// @param values List of elements to append.
	/// @return Reference to self.
	constexpr SelfType& appendBack(ArgumentListType const& values) {
		return appendBack(SelfType(values));
	}

	/// @brief Appends a quantity of elements of a given value to the end of the `List`.
	/// @param count Amount of elements to append.
	/// @param fill Value of the elements.
	/// @return Reference to self.
	constexpr SelfType& appendBack(SizeType const& count, DataType const& fill) {
		return expand(count, fill);
	}

	/// @brief Appends a range of elements to the end of the `List`.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator pointing to end of range.
	/// @return Reference to self.
	constexpr SelfType& appendBack(IteratorType const& begin, IteratorType const& end) {
		return appendBack(SelfType(begin, end));
	}

	/// @brief Appends a range of elements to the end of the `List`.
	/// @param begin Reverse iterator to beginning of range.
	/// @param end Reverse iterator pointing to end of range.
	/// @return Reference to self.
	constexpr SelfType& appendBack(ReverseIteratorType const& begin, ReverseIteratorType const& end) {
		return appendBack(SelfType(begin, end));
	}

	/// @brief Appends a fixed array of elements to the end of the `List`.
	/// @tparam S Size of the array.
	/// @param values Array of elements to append.
	/// @return Reference to self.
	template<SizeType S>
	constexpr SelfType& appendBack(Decay::AsType<DataType[S]> const& values) {
		expand(S);
		copy(values, contents + count, S);
		count += S;
		return *this;
	}

	/// @brief Clears the `List`.
	/// @return Reference to self.
	/// @note
	///		Does not free the underlying array held by the `List`.
	///		To actually free the underlying array, call `dispose`. 
	constexpr SelfType& clear() {
		memdestruct(contents, count);
		count = 0;
		return *this;
	}

	/// @brief Frees the underlying array held by the `List`.
	/// @return Reference to self.
	/// @note To not free the underlying array, call `clear`. 
	constexpr SelfType& dispose() {
		dump();
		recalculateMagnitude();
		return *this;
	}

	/// @brief Copy assignment operator.
	/// @param other Other `List`.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType const& other) {
		memdestruct(contents, count);
		resize(other.count);
		copy(other.contents, contents, other.count);
		count = other.count;
		return *this;
	}

	/// @brief Move assignment operator.
	/// @param other Other `List`.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType&& other) {
		memdestroy(contents, count);
		maximum			= CTL::move(other.maximum);
		contents		= CTL::move(other.contents);
		count			= CTL::move(other.count);
		magnitude		= CTL::move(other.magnitude);
		other.contents	= nullptr;
		return *this;
	}

	/// @brief Returns a pointer to the underlying array.
	/// @return Pointer to the underlying array.
	constexpr PointerType		data()			{return contents;	}
	/// @brief Returns a pointer to the underlying array.
	/// @return Pointer to the underlying array.
	constexpr ConstPointerType	data() const	{return contents;	}

	/// @brief Returns an iterator to the beginning of the `List`.
	/// @return Iterator to the beginning of the `List`.
	constexpr IteratorType		begin()			{return contents;		}
	/// @brief Returns an iterator to the end of the `List`.
	/// @return Iterator to the end of the `List`.
	constexpr IteratorType		end()			{return contents+count;	}
	/// @brief Returns an iterator to the beginning of the `List`.
	/// @return Iterator to the beginning of the `List`.
	constexpr ConstIteratorType	begin() const	{return contents;		}
	/// @brief Returns an iterator to the end of the `List`.
	/// @return Iterator to the end of the `List`.
	constexpr ConstIteratorType	end() const		{return contents+count;	}

	/// @brief Returns a reverse iterator to the beginning of the `List`.
	/// @return Reverse iterator to the beginning of the `List`.
	constexpr ReverseIteratorType		rbegin()		{return ReverseIteratorType(contents+count);		}
	/// @brief Returns a reverse iterator to the end of the `List`.
	/// @return Reverse iterator to the end of the `List`.
	constexpr ReverseIteratorType		rend()			{return ReverseIteratorType(contents);				}
	/// @brief Returns a reverse iterator to the beginning of the `List`.
	/// @return Reverse iterator to the beginning of the `List`.
	constexpr ConstReverseIteratorType	rbegin() const	{return ConstReverseIteratorType(contents+count);	}
	/// @brief Returns a reverse iterator to the end of the `List`.
	/// @return Reverse iterator to the end of the `List`.
	constexpr ConstReverseIteratorType	rend() const	{return ConstReverseIteratorType(contents);			}

	/// @brief Returns a pointer to the beginning of the `List`.
	/// @return Pointer to the beginning of the `List`.
	constexpr PointerType	cbegin()		{return contents;		}
	/// @brief Returns a pointer to the end of the `List`.
	/// @return Pointer to the end of the `List`.
	constexpr PointerType	cend()			{return contents+count;	}
	/// @brief Returns a pointer to the beginning of the `List`.
	/// @return Pointer to the beginning of the `List`.
	constexpr ConstantType*	cbegin() const	{return contents;		}
	/// @brief Returns a pointer to the end of the `List`.
	/// @return Pointer to the end of the `List`.
	constexpr ConstantType*	cend() const	{return contents+count;	}
	
	/// @brief Returns the value of the first element.
	/// @return Reference to the first element.
	/// @throw OutOfBoundsException when `List` is empty.
	constexpr ReferenceType		front()			{return at(0);			}
	/// @brief Returns the value of the last element.
	/// @return Reference to the last element.
	/// @throw OutOfBoundsException when `List` is empty.
	constexpr ReferenceType 	back()			{return at(count-1);	}
	/// @brief Returns the value of the first element.
	/// @return Value of the first element.
	/// @throw OutOfBoundsException when `List` is empty.
	constexpr DataType			front() const	{return at(0);			}
	/// @brief Returns the value of the last element.
	/// @return Value of the last element.
	/// @throw OutOfBoundsException when `List` is empty.
	constexpr DataType			back() const	{return at(count-1);	}

	/// @brief Returns the value of the element at a given index.
	/// @param index Index of the element.
	/// @return Reference to the element.
	/// @throw OutOfBoundsException when `List` is empty.
	constexpr DataType& at(IndexType index) {
		if (!count) emptyError();
		assertIsInBounds(index);
		wrapBounds(index, count);
		return contents[index];
	}

	/// @brief Returns the value of the element at a given index.
	/// @param index Index of the element.
	/// @return Value of the element.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	constexpr DataType at(IndexType index) const {
		if (!count) emptyError();
		assertIsInBounds(index);
		wrapBounds(index, count);
		return contents[index];
	}

	/// @brief Returns the value of the element at a given index.
	/// @param index Index of the element.
	/// @return Reference to the element.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	constexpr ReferenceType	operator[](IndexType index)			{return at(index);}
	/// @brief Returns the value of the element at a given index.
	/// @param index Index of the element.
	/// @return Value of the element.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	constexpr DataType operator[](IndexType const& index) const	{return at(index);}

	/// @brief Returns the current element count.
	/// @return Element count.
	constexpr SizeType size() const		{return count;		}
	/// @brief Returns the current size of the underlying array.
	/// @return Size of the underlying array.
	constexpr SizeType capacity() const	{return maximum;	}
	/// @brief Returns whether the array is empty.
	/// @return Whether the array is empty.
	constexpr SizeType empty() const	{return count == 0;	}

	/// @brief Equality operator.
	/// @param other Other `List` to compare with.
	/// @return Whether they're equal.
	/// @note Requires element type to be equally comparable.
	/// @sa Comparator::equals()
	constexpr bool operator==(SelfType const& other) const
	requires Type::Comparator::Equals<DataType, DataType> {
		return equals(other);
	}

	/// @brief Threeway comparison operator.
	/// @param other Other `List` to compare with.
	/// @return Order between both `List`s.
	/// @note Requires element type to be threeway comparable.
	/// @sa Comparator::compare()
	constexpr OrderType operator<=>(SelfType const& other) const
	requires Type::Comparator::Threeway<DataType, DataType> {
		return compare(other);
	}

	/// @brief Converts the `List` of an element type to a different `List` of a new element type.
	/// @tparam T2 New type.
	template <class T2>
	constexpr explicit operator List<T2, SizeType>() const
	requires (Type::Different<DataType, T2> && Type::Convertible<DataType, T2>) {
		List<T2, SizeType> result(count);
		for (usize i = 0; i < count; ++i)
			result[i] = T2(contents[i]);
		return result;
	}

	/// @brief Returns whether it is equal to another `List`.
	/// @param other Other `List` to compare with.
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

	/// @brief Returns the result of a threeway comparison with another `List`.
	/// @param other Other `List` to compare with.
	/// @return Order between both `List`s.
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

	/// @brief How different it is from another `List`.
	/// @param other Other `List` to compare with.
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

	/// @brief Apllies a procedure to all elements of the `List`.
	/// @tparam TProcedure Procedure type.
	/// @param fun Procedure to apply.
	/// @return Reference to self.
	template <class TProcedure>
	constexpr SelfType& transform(TProcedure const& fun) {
		for(DataType& v: *this)
			v = fun(v);
		return *this;
	}

	/// @brief Returns a `List` of `transform`ed elements.
	/// @tparam TProcedure Procedure type.
	/// @param fun Procedure to apply.
	/// @return List of transformed elements.
	template<class TProcedure>
	constexpr SelfType transformed(TProcedure const& fun) const {
		return SelfType(*this).transform(fun);
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

	/// @brief Removes all elements that do not match a given predicate.
	/// @tparam TPredicate Predicate type.
	/// @param filter Predicate to match.
	/// @return Reference to self.
	template<Type::Functional<PredicateType> TPredicate>
	constexpr SelfType& filter(TPredicate const& filter) {
		return eraseIfNot(filter);
	}

	/// @brief Removes all elements that fail a given comparison.
	/// @tparam TCompare Compare type.
	/// @param compare Comparison to make.
	/// @return Reference to self.
	template<Type::Functional<CompareType> TCompare>
	constexpr SelfType& filter(TCompare const& compare) {
		return *this = filtered(compare);
	}

	/// @brief Returns a `filter`ed `List` of elements.
	/// @tparam TPredicate Predicate type.
	/// @param filter Predicate to match.
	/// @return `filter`ed `List` of elements.
	template<Type::Functional<PredicateType> TPredicate>
	constexpr SelfType filtered(TPredicate const& filter) const {
		return SelfType(*this).eraseIfNot(filter);
	}

	/// @brief Returns a `filter`ed `List` of elements.
	/// @tparam TCompare Compare type.
	/// @param compare Comparison to make.
	/// @return `filter`ed `List` of elements.
	template<Type::Functional<CompareType> TCompare>
	constexpr SelfType filtered(TCompare const& compare) const {
		SelfType result;
		for (SizeType i = 0; i < count; ++i) {
			bool miss = false;
			for(SizeType j = count - 1; j >= 0; --j) {
				if (i == j) break;
				if ((miss = !compare(contents[i], contents[j])))
					break;
			}
			if (!miss) result.pushBack(contents[i]);
		}
		return result;
	}

	/// @brief Returns a `List` of all unique elements.
	/// @return `List` of unique elements.
	constexpr SelfType uniques() {
		return filtered([](ConstReferenceType a, ConstReferenceType b){return a != b;});
	}

	/// @brief Joins a `List` of `List`s with a given separator between them.
	/// @tparam T Element type.
	/// @param sep Separator value.
	/// @return Resulting joined `List`.
	template <Type::Equal<DataType> T>
	constexpr DataType join(typename T::DataType const& sep) const
	requires requires {
		requires Type::Container::List<DataType>;
		typename DataType::DataType;
		typename DataType::SizeType;
		requires Type::Equal<
			DataType,
			List<
				typename DataType::DataType,
				typename DataType::SizeType
			>
		>;
	} {
		DataType result = front();
		for (SizeType i = 1; i < count; ++i) {
			result.pushBack(sep);
			result.appendBack(contents[i]);
		}
		return result;
	}

	/// @brief Returns whether the current size matches the current capacity.
	/// @return Whether the current size matches the current capacity.
	constexpr bool tight() const {return count == maximum;}

	/// @brief Returns the associated allocator.
	/// @return `List` allocator.
	constexpr AllocatorType& allocator() {return alloc;}

	/// @brief `swap` algorithm for `List`.
	/// @param a `List` to swap.
	/// @param b `List` to swap with.
	friend constexpr void swap(SelfType& a, SelfType& b) noexcept {
		swap(a.contents, b.contents);
		swap(a.maximum, b.maximum);
		swap(a.count, b.count);
		swap(a.magnitude, b.magnitude);
	}

private:
	using Iteratable::wrapBounds;

	constexpr SelfType& squash(SizeType const& i) {
		if (!count) return *this;
		if (count > 1 && i < count-1)
			copy(contents + i + 1, contents + i, count-i-1);
		MX::destruct(contents+count-1);
		return *this;
	}

	constexpr void dump() {
		memdestroy(contents, count);
		contents	= nullptr;
		maximum		= 0;
		count		= 0;
	}

	constexpr static void memdestruct(PointerType const& p, SizeType const& sz) {
		if (!(sz && p)) return;
		if constexpr (!Type::Standard<DataType>) {
			for (auto i = p; i != (p+sz); ++i)
				MX::destruct(i);
		}
	}

	constexpr void memdestroy(PointerType const& p, SizeType const& sz) {
		if (!(sz && p)) return;
		memdestruct(p, sz);
		alloc.deallocate(p);
	}

	constexpr DataType* memcreate(SizeType const& sz) {
		return alloc.allocate(sz);
	}

	constexpr void memresize(DataType*& data, SizeType const& sz, SizeType const& oldsz, SizeType const& count) {
		if constexpr(Type::Standard<DataType>)
			alloc.resize(data, sz);
		else {
			DataType* ndata = alloc.allocate(sz);
			copy(data, ndata, count < sz ? count : sz);
			memdestroy(data, count);
			data = ndata;
		}
	}

	constexpr static void copy(ConstantType* src, DataType* dst, SizeType count) {
		if constexpr (Type::Standard<DataType>)
			MX::memmove<DataType>(dst, src, count);
		else MX::objcopy<DataType>(dst, src, count);
	}

	constexpr SelfType& invoke(SizeType const& size) {
		if (contents)	memresize(contents, size, maximum, count);
		else			contents = memcreate(size);
		maximum = size;
		recalculateMagnitude();
		return *this;
	}

	constexpr SelfType& recalculateMagnitude() {
		if (maximum == 0) {
			magnitude = 1;
			return *this;
		}
		magnitude = 0;
		SizeType const order = (sizeof(SizeType) * 8)-1;
		for (SizeType i = 1; i <= order; ++i) {
			magnitude = 1 << (order - i);
			if ((maximum >> (order - i)) & 1) {
				magnitude <<= 1;
				return *this;
			}
		}
		magnitude = 0;
		return *this;
	}

	constexpr SelfType& increase() {
		if (magnitude == 0) atItsLimitError();
		resize(magnitude);
		return *this;
	}

	constexpr SelfType& grow(SizeType const& count) {
		if (SizeType(this->count + count) < this->count)
			atItsLimitError();
		SizeType const newSize = this->count + count;
		resize(newSize);
		return *this;
	}

	void assertIsInBounds(IndexType const& index) const {
		if (index > IndexType(count-1)) outOfBoundsError();
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

	/// @brief Next underlying array size.
	SizeType		magnitude	= 1;
	/// @brief True underlying array size.
	SizeType		maximum		= 0;
	/// @brief Element count.
	SizeType		count		= 0;
	/// @brief Underlying array.
	DataType*		contents	= nullptr;

	AllocatorType	alloc;
};

/// @brief `List` analog for dynamic array of bytes.
/// @tparam TIndex Index type. 
/// @tparam TAlloc<class> Allocator type.
template <Type::Integer TIndex = usize, template <class> class TAlloc = HeapAllocator>
using BinaryData = List<uint8, TIndex, TAlloc>;

static_assert(Type::Container::List<List<int>>);

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_LIST_H
