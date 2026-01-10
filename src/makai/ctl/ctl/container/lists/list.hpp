#ifndef CTL_CONTAINER_LISTS_LIST_H
#define CTL_CONTAINER_LISTS_LIST_H

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

#include <iostream>

CTL_NAMESPACE_BEGIN

template<
	class TData,
	Type::Integer TIndex = usize,
	template <class> class TAlloc		= HeapAllocator,
	template <class> class TConstAlloc	= ConstantAllocator
>
struct List;

/// @brief Container-specific type constraints.
namespace Type::Container {
	/// @brief Implementation of type constraints.
	namespace Impl {
		template<class T>
		struct IsList;

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
		struct IsList<T0<T1, T2, T3, T4>>: BooleanConstant<Type::Equal<T0<T1, T2, T3, T4>, ::CTL::List<T1, T2, T3, T4>>> {};
	}

	/// Type must be `List`.
	template<class T>
	concept List = Impl::IsList<T>::value; 
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
struct List:
	Iteratable<TData, TIndex>,
	SelfIdentified<List<TData, TIndex, TAlloc, TConstAlloc>>,
	ContextAwareAllocatable<TData, TAlloc>,
	Ordered {
public:
	using Iteratable				= ::CTL::Iteratable<TData, TIndex>;
	using SelfIdentified			= ::CTL::SelfIdentified<List<TData, TIndex, TAlloc, TConstAlloc>>;
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

	/// Default constructor.
	constexpr List(): count(0) {invoke(1);}

	/// @brief Constructs the `List` with a preallocated capacity.
	/// @param size Size to preallocate.
	constexpr explicit List(SizeType const size) {
		invoke(size);
	}

	/// @brief Constructs a `List` of a given size and a given fill.
	/// @param size Size to allocate.
	/// @param fill Value to set for elements.
	constexpr explicit List(SizeType const size, DataType const& fill) {
		invoke(size);
		for (usize i = 0; i < size; ++i)
			contents[i] = fill;
		count = size;
	}
	
	/// @brief Constructs the `List` with a parameter pack.
	/// @tparam ...Args Parameter pack.
	/// @param ...args Pack elements.
	template<typename... Args>
	constexpr List(Args const&... args)
	requires (
	#ifndef __clang__
		(sizeof...(Args) > 0)
	&&	(... && (Type::Different<Args, SelfType> && Type::CanBecome<Args, DataType>))
	#else
		(sizeof...(Args) > 1) 
	&&	(
		(... && (Type::Standard<Args> && Type::CanBecome<Args, DataType>))
	||	(... && (!Type::Standard<Args> && Type::Different<Args, SelfType>))
	)
	#endif
	) {
		invoke(sizeof...(Args));
		(..., pushBack(args));
	}

	/// @brief Constructs the `List` from a fixed array of elements.
	/// @tparam S Size of array.
	/// @param values Elements to add to `List`.
	template<SizeType S>
	constexpr List(As<ConstantType[S]> const& values) {
		invoke(S);
		copy(values, contents.data(), S);
		count = S;
	}

	/// @brief Copy constructor.
	/// @param other `List` to copy from.
	constexpr List(SelfType const& other) {
		invoke(other.contents.size());
		copy(other.contents.data(), contents.data(), other.count);
		count = other.count;
	}

	/// @brief Move constructor.
	/// @param other `List` to move from.
	constexpr List(SelfType&& other) {
		contents	= ::CTL::move(other.contents);
		count		= ::CTL::move(other.count);
		magnitude	= ::CTL::move(other.magnitude);
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
		copy(begin, contents.data(), end - begin);
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
	constexpr explicit List(ConstIteratorType const& begin, ConstIteratorType const& end) {
		if (end <= begin) return;
		invoke(end - begin + 1);
		copy(begin, contents.data(), end - begin);
		count = end - begin;
	}

	/// @brief Constructs a `List` from a range of elements.
	/// @param begin Reverse iterator to beginning of range.
	/// @param end Reverse iterator to end of range.
	constexpr explicit List(ConstReverseIteratorType const& begin, ConstReverseIteratorType const& end) {
		if (end <= begin) return;
		invoke(end - begin + 1);
		for (auto i = begin; i != end; ++i)
			pushBack(*i);
		count = end - begin;
	}

	/// @brief Constructs a `List` from a "C-style" range of elements.
	/// @param start Start of range.
	/// @param size Size of range.
	constexpr explicit List(ConstPointerType const& start, SizeType const size): List(start, start + size) {}

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

	/// @brief Constructs and adds new element to the end of the `List`. 
	/// @tparam ...Args Argument types.
	/// @param ...args Values to pass to constructor.
	/// @return Reference to self.
	template<class... Args>
	constexpr SelfType& constructBack(Args... args) {
		if (count >= contents.size())
			increase();
		MX::construct(contents.data()+(count++), args...);
		return *this;
	}

	/// @brief Adds a new element to the end of the `List`. 
	/// @param value Element to add.
	/// @return Reference to self.
	constexpr SelfType& pushBack(DataType const& value) {
		if (count >= contents.size())
			increase();
		MX::construct(contents.data()+(count++), value);
		return *this;
	}

	/// @brief Removes an element from the end of the `List`.
	/// @return Value of the element removed.
	/// @throw OutOfBoundsException when `List` is empty.
	constexpr DataType popBack() {
		if (empty()) emptyError();
		DataType value = back();
		count--;
		if (count) MX::destruct<DataType>(contents.data()+count);
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
		if (count >= contents.size()) increase();
		copy(contents.data() + index, contents.data() + index + 1, count - index);
		MX::construct(contents.data()+index, value);
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
		copy(contents.data() + index, contents.data() + index+other.count, count - index);
		copy(other.contents.data(), contents.data() + index, other.count);
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
	constexpr SelfType& insert(As<ConstantType[S]> const& values, IndexType const index) {
		return insert(SelfType(values), index);
	}

	/// @brief Inserts a given value, a given amount of times, at a specified index in the `List`.
	/// @param value Value to be inserted.
	/// @param count Amount of times to insert the element.
	/// @param index Index of which to insert in.
	/// @return Reference to self.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	/// @note If index is negative, it will be interpreted as starting from the end of the `List`.
	constexpr SelfType& insert(DataType const& value, SizeType const count, IndexType const index) {
		return insert(SelfType(count, value), index);
	}

	/// @brief Ensures the `List` can hold AT LEAST a given capacity.
	/// @param count Minimum size of the `List`.
	/// @return Reference to self.
	/// @note
	///		This guarantees the capacity will be AT LEAST `count`,
	/// 	but does not guarantee the capacity will be EXACTLY `count`.
	///		For that, use `resize`.
	constexpr SelfType& reserve(SizeType const count) {
		while (contents.size() < count)
			increase();
		return *this;
	}

	/// @brief Resizes the `List`, so the capacity is of a given size.
	/// @param newSize New `List` size.
	/// @return Reference to self.
	/// @note
	///		This guarantees the capacity will be EXACTLY of `newSize`.
	/// 	If you need the capacity to be AT LEAST `newSize`, use `reserve`.
	constexpr SelfType& resize(SizeType const newSize) {
		if (!newSize) return clear();
		if (contents.size())	remake(newSize); 
		else					contents.create(newSize);
		if (count > newSize)
			count = newSize;
		recalculateMagnitude();
		return *this;
	}

	/// @brief Expands the `List`, such that it can hold AT LEAST `size()` + `count`.
	/// @param count Count to increase by.
	/// @return Reference to self.
	constexpr SelfType& expand(SizeType const count) {
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
	constexpr SelfType& reserve(SizeType const count, DataType const& fill) {
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
	constexpr SelfType& resize(SizeType const newSize, DataType const& fill) {
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
		::CTL::reverse(begin(), end());
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
		return ::CTL::fsearch(begin(), end(), value);
	}

	/// @brief Finds the the position of the last element that matches a value.
	/// @param value Value to search for.
	/// @return The index of the value, or -1 if not found.
	constexpr IndexType rfind(DataType const& value) const
	requires Type::Comparator::Equals<DataType, DataType> {
		return ::CTL::rsearch(begin(), end(), value);
	}

	/// @brief Performs a binary search to find the index of an element that matches a value.
	/// @param value Value to search for.
	/// @return The index of the value, or -1 if not found.
	/// @note Requires the array to be sorted.
	constexpr IndexType bsearch(DataType const& value) const
	requires (Type::Comparator::Threeway<DataType, DataType>) {
		return ::CTL::bsearch(begin(), end(), value);
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
	/// @return Count of elements removed.
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
	/// @return Count of elements removed.
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

	/// @brief Removes elements from a specified range.
	/// @param start Starting index to start removing from.
	/// @param stop End index to stop removing from.
	/// @return Count of elements removed.
	/// @note
	///		Does not resize `List`, merely moves it to the end, and destructs it.
	///		If you need the `List` size to change, use `erase`. 
	constexpr SizeType removeRange(IndexType start, IndexType stop = -1) {
		if (empty()) return 0;
		assertIsInBounds(start);
		wrapBounds(start, count);
		if (stop < 0) wrapBounds(stop, count);
		if (stop < start) return 0;
		if (SizeType(stop) > count) stop = count;
		squashRange(start, stop - start);
		return stop - start;
	}

	/// @brief Erases an element at a given index.
	/// @param index Index of the element to erase.
	/// @return Reference to self.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	/// @note
	///		Resizes the `List`.
	///		If you need the `List` size to remain the same, use `remove`. 
	constexpr SelfType& erase(IndexType const index) {
		if (empty()) return *this;
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

	/// @brief Erases elements between a given range.
	/// @param start Starting index to start removing from.
	/// @param stop End index to stop removing from.
	/// @note
	///		Resizes the `List`.
	///		If you need the `List` size to remain the same, use `remove`.
	constexpr SelfType& eraseRange(IndexType const start, IndexType const stop) {
		count -= removeRange(start, stop);
		return *this;
	}

	/// @brief Returns a `List` containing all elements EXCLUDING the ones located between two indices.
	/// @param start Starting index to start removing from.
	/// @param stop End index to stop removing from.
	/// @return `List` containing elements between `start` and `stop`.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	/// @note If index is negative, it will be interpreted as starting from the end of the `List`.
	constexpr SelfType withoutRange(IndexType const start, IndexType const stop) const {
		return sliced(0, start).appendBack(sliced(stop));
	}

	
	/// @brief Replaces any element that matches, with the replacement.
	/// @param val Element to match.
	/// @param rep Replacement.
	/// @return Reference to self.
	constexpr SelfType& replace(DataType const& val, DataType const& rep) {
		for (DataType& v: *this)
			if (v == val) v = rep;
		return *this;
	}

	/// @brief Replaces any element that matches the set, with the replacement.
	/// @param values Element to match.
	/// @param rep Replacement.
	/// @return Reference to self.
	constexpr SelfType& replace(SelfType const& values, DataType const& rep) {
		for (DataType const& val: values)
			replace(val, rep);
		return *this;
	}

	/// @brief Returns a list with any element that matches the given one replaced.
	/// @param val Element to match.
	/// @param rep Replacement.
	/// @return Resulting list.
	constexpr SelfType replaced(DataType const& val, DataType const& rep) const		{return SelfType(*this).replace(val, rep);		}
	/// @brief Returns a list with any element that matches the given ones replaced.
	/// @param values Elements to match.
	/// @param rep Replacement.
	/// @return Resulting list.
	constexpr SelfType replaced(SelfType const& values, DataType const& rep) const	{return SelfType(*this).replace(values, rep);	}

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
		assertIsInBounds(stop);
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
	/// @param other `List` to copy contents from.
	/// @return Reference to self.
	constexpr SelfType& appendBack(SelfType const& other) {
		return appendBack(other.begin(), other.end());
	}

	/// @brief Appends a quantity of elements of a given value to the end of the `List`.
	/// @param count Amount of elements to append.
	/// @param fill Value of the elements.
	/// @return Reference to self.
	constexpr SelfType& appendBack(SizeType const count, DataType const& fill) {
		return expand(count, fill);
	}

	/// @brief Appends a range of elements to the end of the `List`.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator pointing to end of range.
	/// @return Reference to self.
	constexpr SelfType& appendBack(ConstIteratorType const& begin, ConstIteratorType const& end) {
		expand(end - begin);
		copy(begin, contents.data() + count, end - begin);
		count += (end - begin);
		return *this;
	}

	/// @brief Appends a range of elements to the end of the `List`.
	/// @param begin Reverse iterator to beginning of range.
	/// @param end Reverse iterator pointing to end of range.
	/// @return Reference to self.
	constexpr SelfType& appendBack(ConstReverseIteratorType const& begin, ConstReverseIteratorType const& end) {
		return appendBack(SelfType(begin, end));
	}

	/// @brief Appends a fixed array of elements to the end of the `List`.
	/// @tparam S Size of the array.
	/// @param values Array of elements to append.
	/// @return Reference to self.
	template<SizeType S>
	constexpr SelfType& appendBack(As<DataType[S]> const& values) {
		expand(S);
		copy(values, contents.data() + count, S);
		count += S;
		return *this;
	}

	/// @brief Clears the `List`.
	/// @return Reference to self.
	/// @note
	///		Does not free the underlying array held by the `List`.
	///		To actually free the underlying array, call `dispose`. 
	constexpr SelfType& clear() {
		MX::objclear(contents.data(), count);
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
		clear();
		resize(other.count);
		copy(other.contents.data(), contents.data(), other.count);
		count = other.count;
		return *this;
	}

	/// @brief Move assignment operator.
	/// @param other Other `List`.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType&& other) {
		if (inCompileTime()) {
			clear();
			resize(other.count);
			copy(other.contents.data(), contents.data(), other.count);
			count = other.count;
		} else {
			destroy(count);
			contents	= CTL::move(other.contents);
			count		= CTL::move(other.count);
			magnitude	= CTL::move(other.magnitude);
		}
		return *this;
	}

	/// @brief Returns a pointer to the underlying array.
	/// @return Pointer to the underlying array.
	constexpr PointerType		data()			{return contents.data();	}
	/// @brief Returns a pointer to the underlying array.
	/// @return Pointer to the underlying array.
	constexpr ConstPointerType	data() const	{return contents.data();	}

	/// @brief Returns an iterator to the beginning of the `List`.
	/// @return Iterator to the beginning of the `List`.
	constexpr IteratorType		begin()			{return contents.data();		}
	/// @brief Returns an iterator to the end of the `List`.
	/// @return Iterator to the end of the `List`.
	constexpr IteratorType		end()			{return contents.data()+count;	}
	/// @brief Returns an iterator to the beginning of the `List`.
	/// @return Iterator to the beginning of the `List`.
	constexpr ConstIteratorType	begin() const	{return contents.data();		}
	/// @brief Returns an iterator to the end of the `List`.
	/// @return Iterator to the end of the `List`.
	constexpr ConstIteratorType	end() const		{return contents.data()+count;	}

	/// @brief Returns a reverse iterator to the beginning of the `List`.
	/// @return Reverse iterator to the beginning of the `List`.
	constexpr ReverseIteratorType		rbegin()		{return ReverseIteratorType(contents.data()+count);			}
	/// @brief Returns a reverse iterator to the end of the `List`.
	/// @return Reverse iterator to the end of the `List`.
	constexpr ReverseIteratorType		rend()			{return ReverseIteratorType(contents.data());				}
	/// @brief Returns a reverse iterator to the beginning of the `List`.
	/// @return Reverse iterator to the beginning of the `List`.
	constexpr ConstReverseIteratorType	rbegin() const	{return ConstReverseIteratorType(contents.data()+count);	}
	/// @brief Returns a reverse iterator to the end of the `List`.
	/// @return Reverse iterator to the end of the `List`.
	constexpr ConstReverseIteratorType	rend() const	{return ConstReverseIteratorType(contents.data());			}

	/// @brief Returns a pointer to the beginning of the `List`.
	/// @return Pointer to the beginning of the `List`.
	constexpr PointerType	cbegin()			{return contents.data();		}
	/// @brief Returns a pointer to the end of the `List`.
	/// @return Pointer to the end of the `List`.
	constexpr PointerType	cend()				{return contents.data()+count;	}
	/// @brief Returns a pointer to the beginning of the `List`.
	/// @return Pointer to the beginning of the `List`.
	constexpr ConstPointerType	cbegin() const	{return contents.data();		}
	/// @brief Returns a pointer to the end of the `List`.
	/// @return Pointer to the end of the `List`.
	constexpr ConstPointerType	cend() const	{return contents.data()+count;	}
	
	/// @brief Returns the value of the first element.
	/// @return Reference to the first element.
	/// @throw OutOfBoundsException when `List` is empty.
	constexpr ReferenceType			front()			{return at(0);			}
	/// @brief Returns the value of the last element.
	/// @return Reference to the last element.
	/// @throw OutOfBoundsException when `List` is empty.
	constexpr ReferenceType 		back()			{return at(count-1);	}
	/// @brief Returns the value of the first element.
	/// @return Value of the first element.
	/// @throw OutOfBoundsException when `List` is empty.
	constexpr ConstReferenceType	front() const	{return at(0);			}
	/// @brief Returns the value of the last element.
	/// @return Value of the last element.
	/// @throw OutOfBoundsException when `List` is empty.
	constexpr ConstReferenceType	back() const	{return at(count-1);	}

	/// @brief Returns the value of the element at a given index.
	/// @param index Index of the element.
	/// @return Reference to the element.
	/// @throw OutOfBoundsException when `List` is empty.
	constexpr ReferenceType at(IndexType index) {
		if (!count) emptyError();
		assertIsInBounds(index);
		wrapBounds(index, count);
		return contents[index];
	}

	/// @brief Returns the value of the element at a given index.
	/// @param index Index of the element.
	/// @return Value of the element.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	constexpr ConstReferenceType at(IndexType index) const {
		if (!count) emptyError();
		assertIsInBounds(index);
		wrapBounds(index, count);
		return contents[index];
	}

	/// @brief Returns the value of the element at a given index.
	/// @param index Index of the element.
	/// @return Reference to the element.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	constexpr ReferenceType	operator[](IndexType index)						{return at(index);}
	/// @brief Returns the value of the element at a given index.
	/// @param index Index of the element.
	/// @return Value of the element.
	/// @throw OutOfBoundsException when index is bigger than `List` size.
	constexpr ConstReferenceType operator[](IndexType const index) const	{return at(index);}

	/// @brief Returns the current element count.
	/// @return Element count.
	constexpr SizeType size() const		{return count;				}
	/// @brief Returns the current size of the underlying array.
	/// @return Size of the underlying array.
	constexpr SizeType capacity() const	{return contents.size();	}
	/// @brief Returns whether the list is empty.
	/// @return Whether the array is list.
	constexpr SizeType empty() const	{return count == 0;			}

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

	/// @brief Returns whether this `List` is equal to another `List`.
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

	/// @brief Returns how different this `List` is from another `List`.
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
	template <Type::Functional<TransformType> TProcedure>
	constexpr SelfType& operator|=(TProcedure const& fun) {
		return transform(fun);
	}

	/// @brief Returns a `List` of `transform`ed elements.
	/// @tparam TProcedure Procedure type.
	/// @param fun Procedure to apply.
	/// @return List of transformed elements.
	template <Type::Functional<TransformType> TProcedure>
	constexpr SelfType operator|(TProcedure const& fun) const {
		return transformed(fun);
	}

	/// @brief Apllies a procedure to the `List`.
	/// @tparam TProcedure Procedure type.
	/// @param fun Procedure to apply.
	/// @return Reference to self.
	template <Type::Functional<SelfType&(SelfType&)> TProcedure>
	constexpr SelfType& operator|=(TProcedure const& fun) {
		return fun(*this);
	}
	
	/// @brief Returns a copy of the list, with the given procedure applied to it.
	/// @tparam TProcedure Procedure type.
	/// @param fun Procedure to apply.
	/// @return Transformed list.
	template <Type::Functional<SelfType(SelfType const&)> TProcedure>
	constexpr SelfType operator|(TProcedure const& fun) const {
		return fun(*this);
	}

	/// @brief Apllies a procedure to all elements of the `List`.
	/// @tparam TProcedure Procedure type.
	/// @param fun Procedure to apply.
	/// @return Reference to self.
	template <Type::Functional<TransformType> TProcedure>
	constexpr SelfType& transform(TProcedure const& fun) {
		for(DataType& v: *this)
			v = fun(v);
		return *this;
	}

	/// @brief Returns a `List` of `transform`ed elements.
	/// @tparam TProcedure Procedure type.
	/// @param fun Procedure to apply.
	/// @return List of transformed elements.
	template<Type::Functional<TransformType> TProcedure>
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

	/// @brief Joins a `List` of ranged elements with a given separator between them.
	/// @param sep Separator.
	/// @return Resulting joined element.
	template<Type::Equal<DataType> T = DataType>
	constexpr DataType join(typename T::DataType const& sep) const {
		if (!count) return DataType();
		DataType result = front();
		for (SizeType i = 1; i < count; ++i) {
			result.pushBack(sep);
			result.appendBack(contents[i]);
		}
		return result;
	}

	/// @brief Joins a `List` of ranged elements with a given separator between them.
	/// @param sep Separator.
	/// @return Resulting joined element.
	template<Type::Convertible<DataType> T = DataType>
	constexpr DataType join(T const& sep) const {
		if (!count) return DataType();
		DataType result = front();
		for (SizeType i = 1; i < count; ++i) {
			result.appendBack(sep);
			result.appendBack(contents[i]);
		}
		return result;
	}

	/// @brief Joins a `List` of ranged elements.
	/// @return Resulting joined element.
	constexpr DataType join() const {
		if (!count) return DataType();
		DataType result = front();
		for (SizeType i = 1; i < count; ++i) {
			result.appendBack(contents[i]);
		}
		return result;
	}

	template<class T>
	constexpr DataType join(T const&) const = delete;

	/// @brief Returns whether the current size matches the current capacity.
	/// @return Whether the current size matches the current capacity.
	constexpr bool tight() const {return count == contents.size();}

	/// @brief `swap` algorithm for `List`.
	/// @param a `List` to swap.
	/// @param b `List` to swap with.
	friend constexpr void swap(SelfType& a, SelfType& b) noexcept {
		swap(a.contents, b.contents);
		swap(a.count, b.count);
		swap(a.magnitude, b.magnitude);
	}

	/// @brief Returns the associated allocator.
	/// @return Allocator.
	constexpr auto allocator() const	{return contents.allocator();}

	/// @brief Constructs a list from a series of values.
	/// @tparam Types... Element types.
	/// @param values... Values to construct from.
	/// @return Constructed list.
	template<class... Types>
	constexpr static SelfType from(Types const&... values) {
		SelfType result;
		(..., result.pushBack(values));
		return result;
	}

	constexpr List<byte, TIndex, TAlloc, TConstAlloc> toBytes() const
	requires (Type::Equal<DataType, AsNonReference<DataType>>) {
		auto const start = reinterpret_cast<ref<byte const>>(data());
		return List<byte, TIndex, TAlloc, TConstAlloc>(
			start,
			start + (size() * sizeof(DataType))
		);
	}
	
	constexpr List<sbyte, TIndex, TAlloc, TConstAlloc> toSignedBytes() const
	requires (Type::Equal<DataType, AsNonReference<DataType>>) {
		auto const start = reinterpret_cast<ref<sbyte const>>(data());
		return List<sbyte, TIndex, TAlloc, TConstAlloc>(
			start,
			start + (size() * sizeof(DataType))
		);
	}
	
	template <class TNew>
	constexpr List<TNew, TIndex, TAlloc, TConstAlloc> toList() const 
	requires (Type::CanBecome<DataType, TNew> || Type::Constructible<TNew, DataType>) {
		List<TNew, TIndex, TAlloc, TConstAlloc> result;
		for (auto const& elem: *this)
			result.pushBack(TNew(elem));
		return result;
	}

	template <class TNew, Type::Functional<TNew(DataType const&)> TConvert>
	constexpr List<TNew, TIndex, TAlloc, TConstAlloc> toList(TConvert const& conv) const {
		List<TNew, TIndex, TAlloc, TConstAlloc> result;
		for (auto const& elem: *this)
			result.pushBack(conv(elem));
		return result;
	}

private:
	using Iteratable::wrapBounds;

	ContextAllocatorType alloc;

	constexpr SelfType& squash(SizeType const i) {
		CTL_DEVMODE_FN_DECL;
		if (!count) return *this;
		if (count > 1 && i < count-1)
			copy(contents.data() + i + 1, contents.data() + i, count-i-1);
		MX::destruct(contents.data()+count-1);
		return *this;
	}

	constexpr SelfType& squashRange(SizeType const start, SizeType const amount) {
		CTL_DEVMODE_FN_DECL;
		if (!count) return *this;
		if (count > 1 && start < count-1)
			copy(contents.data() + start + amount, contents.data() + start, count-(start+amount));
		MX::objclear(contents.data()+start, amount);
		return *this;
	}

	constexpr void dump() {
		CTL_DEVMODE_FN_DECL;
		if (!count) return;
		destroy(count);
		count = 0;
	}

	constexpr void destroy(SizeType const count) {
		CTL_DEVMODE_FN_DECL;
		if (contents.empty()) return;
		auto const sz = (count < contents.size()) ? count : contents.size();
		if constexpr (!Type::Standard<DataType>) {
			for (usize i = 0; i < sz; ++i)
				MX::destruct(contents.data() + i);
		}
		contents.free();
	}

	constexpr void remake(usize const newSize) {
		CTL_DEVMODE_FN_DECL;
		auto const newCount = (count < newSize) ? count : newSize;
		if (!newSize)		dump();
		else if (!count)	contents.resize(newSize);
		else {
			StorageType buffer;
			buffer.create(newSize);
			copy(contents.data(), buffer.data(), newCount);
			destroy(count);
			swap(contents, buffer);
		}
		count = newCount;
	}

	constexpr static void copy(ref<ConstantType> src, ref<DataType> dst, SizeType count) {
		CTL_DEVMODE_FN_DECL;
		if (!count) return;
		if (Type::Standard<DataType> && inRunTime())
			MX::memmove<DataType>(dst, src, count);
		else MX::objcopy<DataType>(dst, src, count);
	}

	constexpr SelfType& invoke(SizeType const size) {
		CTL_DEVMODE_FN_DECL;
		if (contents.size()) return *this;
		else contents.create(size);
		recalculateMagnitude();
		return *this;
	}

	constexpr SelfType& recalculateMagnitude() {
		CTL_DEVMODE_FN_DECL;
		constexpr usize ONE = static_cast<SizeType>(1);
		if (contents.size() == 0) {
			magnitude = 1;
			return *this;
		}
		magnitude = 0;
		SizeType const order = (sizeof(SizeType) * 8)-1;
		for (SizeType i = 1; i <= order; ++i) {
			magnitude = ONE << (order - i);
			if ((contents.size() >> (order - i)) & 1) {
				magnitude <<= ONE;
				return *this;
			}
		}
		magnitude = 0;
		return *this;
	}

	constexpr SelfType& increase() {
		CTL_DEVMODE_FN_DECL;
		if (magnitude == 0) atItsLimitError();
		resize(magnitude);
		return *this;
	}

	constexpr SelfType& grow(SizeType const count) {
		CTL_DEVMODE_FN_DECL;
		if (SizeType(this->count + count) < this->count)
			atItsLimitError();
		SizeType const newSize = this->count + count;
		resize(newSize);
		return *this;
	}

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

	/// @brief Next underlying array size.
	SizeType	magnitude	= 1;
	/// @brief Element count.
	SizeType	count		= 0;
	/// @brief Underlying storage.
	StorageType	contents;
};

//static_assert(List<int>().empty());

/// @brief `List` analog for dynamic array of bytes.
/// @tparam TIndex Index type. 
/// @tparam TAlloc<class> Allocator type.
/// @tparam TConstAlloc<class> Constant allocator type.
template <Type::Integer TIndex = usize, template <class> class TAlloc = HeapAllocator, template <class> class TConstAlloc = ConstantAllocator>
using BinaryData = List<byte, TIndex, TAlloc, TConstAlloc>;

/// @brief `List` analog for dynamic array of bytes.
/// @tparam TIndex Index type. 
/// @tparam TAlloc<class> Allocator type.
/// @tparam TConstAlloc<class> Constant allocator type.
template <Type::Integer TIndex = usize, template <class> class TAlloc = HeapAllocator, template <class> class TConstAlloc = ConstantAllocator>
using ByteList = BinaryData<TIndex, TAlloc, TConstAlloc>;

/// @brief `List` analog for dynamic array of bytes.
/// @tparam TIndex Index type. 
/// @tparam TAlloc<class> Allocator type.
/// @tparam TConstAlloc<class> Constant allocator type.
template <Type::Integer TIndex = usize, template <class> class TAlloc = HeapAllocator, template <class> class TConstAlloc = ConstantAllocator>
using Binary = BinaryData<TIndex, TAlloc, TConstAlloc>;

static_assert(Type::Container::List<List<int>>);

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_LIST_H
