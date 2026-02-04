#ifndef CTL_CONTAINER_STRINGS_STRING_H
#define CTL_CONTAINER_STRINGS_STRING_H

#include <stdlib.h>
#include <string_view>
#include <string>

#include "../lists/list.hpp"
#include "../array.hpp"
#include "../pair.hpp"
#include "../../typeinfo.hpp"
#include "../../cpperror.hpp"
#include "../../io/stream.hpp"
#include "../../algorithm/aton.hpp"
#include "../../algorithm/transform.hpp"
#include "../../algorithm/validate.hpp"
#include "../../typetraits/traits.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Dynamic string of characters.
/// @tparam TChar Character type.
/// @tparam TIndex Index type.
/// @tparam TAlloc<class> Runtime allocator type. By default, it is `HeapAllocator`.
/// @tparam TConstAlloc<class> Compile-time allocator type. By default, it is `ConstantAllocator`.
template<
	Type::ASCII TChar,
	Type::Integer TIndex = usize,
	template <class> class TAlloc		= HeapAllocator,
	template <class> class TConstAlloc	= ConstantAllocator
>
struct BaseString:
	private List<TChar, TIndex, TAlloc, TConstAlloc>,
	public SelfIdentified<BaseString<TChar, TIndex, TAlloc, TConstAlloc>>,
	public Derived<List<TChar, TIndex, TAlloc, TConstAlloc>>,
	public CStringable<TChar>,
	public Streamable<TChar> {
public:
	using Iteratable		= ::CTL::Iteratable<TChar, TIndex>;
	using SelfIdentified	= ::CTL::SelfIdentified<BaseString<TChar, TIndex, TAlloc, TConstAlloc>>;
	using Derived			= ::CTL::Derived<List<TChar, TIndex, TAlloc, TConstAlloc>>;
	using Streamable		= ::CTL::Streamable<TChar>;
	using CStringable		= ::CTL::CStringable<TChar>;

	using typename Derived::BaseType;

	using typename BaseType::OrderType;

	using
		typename BaseType::PredicateType,
		typename BaseType::CompareType,
		typename BaseType::TransformType
	;

	using
		typename BaseType::DataType,
		typename BaseType::ConstantType,
		typename BaseType::PointerType,
		typename BaseType::ConstPointerType,
		typename BaseType::ReferenceType,
		typename BaseType::ConstReferenceType
	;

	using
		typename BaseType::IndexType,
		typename BaseType::SizeType
	;

	using BaseType::MAX_SIZE;

	using
		typename BaseType::IteratorType,
		typename BaseType::ConstIteratorType,
		typename BaseType::ReverseIteratorType,
		typename BaseType::ConstReverseIteratorType
	;

	using
		typename SelfIdentified::SelfType
	;

	using
		typename CStringable::CStringType
	;

	using
		typename Streamable::InputStreamType,
		typename Streamable::OutputStreamType
	;

	/// @brief STL library view analog.
	using STDViewType	= std::basic_string_view<DataType>;
	/// @brief STL library string analog.
	using STDStringType	= std::basic_string<DataType>;

	using
//		BaseType::BaseType,
		BaseType::allocator,
//		BaseType::clear,
//		BaseType::reserve,
//		BaseType::resize,
		BaseType::tight,
//		BaseType::tighten,
		BaseType::data,
		BaseType::cbegin,
//		BaseType::cend,
		BaseType::begin,
//		BaseType::end,
//		BaseType::rbegin,
		BaseType::rend,
		BaseType::front,
//		BaseType::back,
//		BaseType::transformed,
//		BaseType::validate,
//		BaseType::size,
//		BaseType::empty,
//		BaseType::find,
//		BaseType::rfind,
//		BaseType::bsearch,
//		BaseType::capacity,
//		BaseType::appendBack,
//		BaseType::pushBack,
//		BaseType::popBack,
		BaseType::toBytes
	;

	/// @brief Default constructor.
	constexpr BaseString(): BaseType() {
		BaseType::pushBack('\0');
	}

	/// @brief Constructs the `BaseString` with a preallocated capacity.
	/// @param size Size to preallocate.
	constexpr explicit BaseString(SizeType const size): BaseType(size+1) {}

	/// @brief Constructs a `BaseString` of a given size and a given fill.
	/// @param size Size to allocate.
	/// @param fill Value to set for each character.
	constexpr explicit BaseString(SizeType const size, DataType const& fill): BaseType(size+1, fill) {
		BaseType::back() = '\0';
	}

	/// @brief Constructs the `BaseString` from a fixed array of characters.
	/// @tparam S Size of array.
	/// @param values Characters to add to `BaseString`.
	template<SizeType S>
	constexpr BaseString(As<ConstantType[S]> const& values) {
		BaseType::resize(values[S-1] == '\0' ? S : S+1);
		BaseType::appendBack(values);
		if (values[S-1] != '\0') BaseType::pushBack('\0');
	}

	/// @brief Copy constructor.
	/// @param other `BaseString` to copy from.
	constexpr BaseString(SelfType const& other) {
		BaseType::resize(other.size()+1);
		BaseType::appendBack(other.begin(), other.end());
		BaseType::pushBack('\0');
	}

	/// @brief Move constructor.
	/// @param other `BaseString` to move from.
	constexpr BaseString(SelfType&& other) {
		BaseType::resize(other.size()+1);
		BaseType::appendBack(other.begin(), other.end());
		BaseType::pushBack('\0');
	}

	/// @brief Constructs a `BaseString` from a range of characters.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator to end of range.
	constexpr BaseString(ConstIteratorType const& begin, ConstIteratorType const& end): BaseType() {
		if (end <= begin) return;
		BaseType::resize(end - begin + (*(end-1) == '\0' ? 1 : 2));
		BaseType::appendBack(begin, end);
		if (BaseType::back() != '\0')
			BaseType::pushBack('\0');
	}

	/// @brief Constructs a `BaseString` from a range of characters.
	/// @param begin Reverse iterator to beginning of range.
	/// @param end Reverse iterator to end of range.
	constexpr BaseString(ConstReverseIteratorType const& begin, ConstReverseIteratorType const& end): BaseType() {
		if (end <= begin) return;
		BaseType::resize(end - begin + (*(end-1) == '\0' ? 1 : 2));
		BaseType::appendBack(begin, end);
		if (BaseType::back() != '\0')
			BaseType::pushBack('\0');
	}

	/// @brief Constructs a `BaseString` from a "C-style" range of characters.
	/// @param start Start of range.
	/// @param size Size of range.
	constexpr explicit BaseString(ConstPointerType const& start, SizeType const size): BaseType(size) {
		BaseType::appendBack(start, start + size);
		if (BaseType::back() != '\0')
			BaseType::pushBack('\0');
	}

	/// @brief Constructs a `BaseString`, from a ranged object of (non-subclass) type T.
	/// @tparam T Ranged type.
	/// @param other Object to copy from.
	template<Type::Container::Ranged<IteratorType, ConstIteratorType> T>
	constexpr BaseString(T const& other)
	requires (!Type::Subclass<T, SelfType>):
		BaseString(other.begin(), other.end()) {}

	/// @brief Constructs a `BaseString`, from a bounded object of (non-list) type T.
	/// @tparam T Ranged type.
	/// @param other Object to copy from.
	template<Type::Container::Bounded<PointerType, SizeType> T>
	constexpr explicit BaseString(T const& other)
	requires (
		!Type::Container::List<T>
	&&	!Type::Container::Ranged<T, IteratorType, ConstIteratorType>
	): BaseString(other.data(), other.size()) {}

	/// @brief Destructor.
	constexpr ~BaseString() {}

	/// @brief Constructs a `BaseString` from a null-terminated string.
	/// @param v String to copy from.
	constexpr BaseString(CStringType const& v) {
		SizeType len = 0;
		while (v[len++] != '\0' && len <= MAX_SIZE);
		BaseType::reserve(len);
		BaseType::appendBack(v, v+len);
	}

	/// @brief Constructs a `BaseString` from a series of arguments.
	/// @tparam ...Args Argument types.
	/// @param ...args Arguments.
	template<class... Args>
	constexpr BaseString(Args const&... args)
	requires (... && Type::Equal<Args, SelfType>) {
		(*this) += (... + args);
	}

	/// @brief Constructos a `BaseString` from a STL view analog.
	/// @param str View to copy from.
	constexpr BaseString(STDViewType const& str):	BaseString(&*str.begin(), &*str.end())	{}
	/// @brief Constructos a `BaseString` from a STL string analog.
	/// @param str View to copy from.
	constexpr BaseString(STDStringType const& str):	BaseString(&*str.begin(), &*str.end())	{}

	/// @brief Adds a new character to the end of the `BaseString`.
	/// @param value Character to add.
	/// @return Reference to self.
	constexpr SelfType& pushBack(DataType const& value) {
		BaseType::back() = value;
		BaseType::pushBack('\0');
		return *this;
	}

	/// @brief Removes an character from the end of the `BaseString`.
	/// @return Removed character.
	/// @throw OutOfBoundsException when `BaseString` is empty.
	constexpr DataType popBack() {
		if (empty()) emptyError();
		BaseType::popBack();
		DataType value = BaseType::back();
		BaseType::back() = '\0';
		return value;
	}

	/// @brief Inserts a character at a specified index in the `BaseString`.
	/// @param value Character to insert.
	/// @param index Index of which to insert in.
	/// @return Reference to self.
	/// @throw OutOfBoundsException when index is bigger than `BaseString` size.
	/// @note If index is negative, it will be interpreted as starting from the end of the `BaseString`.
	constexpr SelfType& insert(DataType const& value, IndexType index) {
		assertIsInBounds(index);
		wrapBounds(index);
		BaseType::insert(value, index);
		return *this;
	}

	/// @brief Inserts a `BaseString` at a specified index in the `BaseString`.
	/// @param other `BaseString` to insert.
	/// @param index Index of which to insert in.
	/// @return Reference to self.
	/// @throw OutOfBoundsException when index is bigger than `BaseString` size.
	/// @note If index is negative, it will be interpreted as starting from the end of the `BaseString`.
	constexpr SelfType& insert(SelfType const& other, IndexType index) {
		assertIsInBounds(index);
		wrapBounds(index);
		BaseType::insert(BaseType(other.begin(), other.end()), index);
		return *this;
	}

	/// @brief Inserts a given character, a given amount of times, at a specified index in the `BaseString`.
	/// @param value Character to be inserted.
	/// @param count Amount of times to insert the character.
	/// @param index Index of which to insert in.
	/// @return Reference to self.
	/// @throw OutOfBoundsException when index is bigger than `BaseString` size.
	/// @note If index is negative, it will be interpreted as starting from the end of the `BaseString`.
	constexpr SelfType& insert(DataType const& value, SizeType const count, IndexType index) {
		assertIsInBounds(index);
		wrapBounds(index);
		BaseType::insert(count, value, index);
		return *this;
	}

	/// @brief Inserts a fixed array of characters at a specified index in the `BaseString`.
	/// @tparam S Size of fixed array.
	/// @param values Characters to insert.
	/// @param index Index of which to insert in.
	/// @return Reference to self.
	/// @throw OutOfBoundsException when index is bigger than `BaseString` size.
	/// @note If index is negative, it will be interpreted as starting from the end of the `BaseString`.
	template<SizeType S>
	constexpr SelfType& insert(As<ConstantType[S]> const& values, IndexType index) {
		assertIsInBounds(index);
		wrapBounds(index);
		BaseType::insert(values, index);
		return *this;
	}

	/// @brief Ensures the `BaseString` can hold AT LEAST a given capacity.
	/// @param count Minimum size of the `BaseString`.
	/// @return Reference to self.
	/// @note
	///		This guarantees the capacity will be AT LEAST `count`,
	/// 	but does not guarantee the capacity will be EXACTLY `count`.
	///		For that, use `resize`.
	constexpr SelfType& reserve(SizeType const count) {
		BaseType::reserve(count + 1);
		return *this;
	}

	/// @brief Resizes the `BaseString`, so the capacity is of a given size.
	/// @param newSize New `BaseString` size.
	/// @return Reference to self.
	/// @note
	///		This guarantees the capacity will be EXACTLY of `newSize`.
	/// 	If you need the capacity to be AT LEAST `newSize`, use `reserve`.
	constexpr SelfType& resize(SizeType const newSize) {
		BaseType::resize(newSize + 1);
		return *this;
	}

	/// @brief Expands the `BaseString`, such that it can hold AT LEAST `size()` + `count`.
	/// @param count Count to increase by.
	/// @return Reference to self.
	constexpr SelfType& expand(SizeType const count) {
		BaseType::expand(count + 1);
		return *this;
	}

	/// @brief
	///		Ensures the `BaseString` can hold AT LEAST a given capacity.
	/// @param count Minimum size of the `BaseString`.
	/// @param fill Character to use as fill.
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
		BaseType::back() = fill;
		BaseType::reserve(count + 1, fill);
		BaseType::back() = '\0';
		return *this;
	}

	/// @brief Resizes the `BaseString`, so the capacity is of a given size, then sets current size to it.
	/// @param newSize New `BaseString` size.
	/// @param fill Character to use as fill.
	/// @return Reference to self.
	///	@note
	///		If current size is smaller,
	///		then it fills the extra space added with the given `fill`.
	/// @note
	///		This guarantees the capacity will be EXACTLY of `newSize`.
	///		If you need the capacity to be AT LEAST `newSize`, use `reserve`.
	constexpr SelfType& resize(SizeType const newSize, DataType const& fill) {
		BaseType::back() = fill;
		BaseType::resize(newSize + 1, fill);
		BaseType::back() = '\0';
		return *this;
	}

	/// @brief
	///		Expands the `BaseString`, such that it can hold AT LEAST the current size,
	///		plus a given `count`.
	/// @param count Count to increase by.
	/// @param fill Character to use as fill.
	/// @return Reference to self.
	///	@note
	///		If current size is smaller,
	///		then it fills the extra space added with the given `fill`.
	constexpr SelfType& expand(SizeType const count, DataType const& fill) {
		BaseType::back() = fill;
		BaseType::expand(count + 1, fill);
		BaseType::back() = '\0';
		return *this;
	}

	/// @brief Ensures the current capacity is EXACTLY the current size.
	/// @return Reference to self.
	constexpr SelfType& tighten() {
		BaseType::tighten();
		return *this;
	}

	/// @brief Reverses the `BaseString`.
	/// @return Reference to self.
	constexpr SelfType& reverse() {
		::CTL::reverse(data(), size());
		return *this;
	}

	/// @brief Returns a reversed copy of the `BaseString`.
	/// @return A reversed copy of the `BaseString`.
	constexpr SelfType reversed() const {
		return SelfType(*this).reverse();
	}

	/// @brief Finds the the position of the first character that matches a value.
	/// @param value Value to search for.
	/// @return The index of the value, or -1 if not found.
	constexpr IndexType find(DataType const& value) const {
		IndexType location = BaseType::find(value);
		return (location == IndexType(size())) ? -1 : location;
	}

	/// @brief Finds the the position of the last character that matches a value.
	/// @param value Value to search for.
	/// @return The index of the value, or -1 if not found.
	constexpr IndexType rfind(DataType const& value) const {
		if (empty()) return -1;
		auto const start = rbegin(), stop = rend();
		for (auto i = start; i != stop; ++i)
			if (BaseType::ComparatorType::equals(*i, value))
				return size()-(i-start)-1;
		return -1;
	}

	/// @brief Performs a binary search to find the index of a character that matches a value.
	/// @param value Value to search for.
	/// @return The index of the value, or -1 if not found.
	/// @note Requires the string to be sorted.
	constexpr IndexType bsearch(DataType const& value) const {
		IndexType location = BaseType::find(value);
		return (location == IndexType(size())) ? -1 : location;
	}

	/// @brief Removes a character at a given index.
	/// @param index Index of the character to remove.
	/// @return Reference to self.
	/// @throw OutOfBoundsException when index is bigger than `BaseString` size.
	/// @note
	///		Does not resize `BaseString`, merely moves it to the end, and destructs it.
	///		If you need the `BaseString` size to change, use `erase`.
	constexpr SelfType& remove(IndexType index) {
		assertIsInBounds(index);
		wrapBounds(index);
		return BaseType::remove(index);
	}

	/// @brief Removes characters that match a given character.
	/// @param value Character to match.
	/// @return Count of characters removed.
	/// @note
	///		Does not resize `BaseString`, merely moves it to the end, and destructs it.
	///		If you need the `BaseString` size to change, use `erase`.
	constexpr SizeType removeLike(DataType const& value) {
		SizeType count = BaseType::removeLike(value) - (value == '\0');
		if (value == '\0') BaseType::back() = '\0';
		return count;
	}

	/// @brief Removes characters that do not match a given character.
	/// @param value Character to match.
	/// @return Count of characters removed.
	/// @note
	///		Does not resize `BaseString`, merely moves it to the end, and destructs it.
	///		If you need the `BaseString` size to change, use `erase`.
	constexpr SizeType removeUnlike(DataType const& value) {
		SizeType count = BaseType::removeUnlike(value) - (value == '\0');
		if (value == '\0') BaseType::back() = '\0';
		return count;
	}

	/// @brief Removes characters that match a given predicate.
	/// @tparam TPredicate Predicate type.
	/// @param predicate Predicate to use as check.
	/// @return Count of characters removed.
	/// @note
	///		Does not resize `BaseString`, merely moves it to the end, and destructs it.
	///		If you need the `BaseString` size to change, use `erase`.
	template<class TPredicate>
	constexpr SizeType removeIf(TPredicate const& predicate) {
		SizeType count = BaseType::removeIf(predicate);
		if (predicate(BaseType::back()))
			--count;
		return count;
	}

	/// @brief Removes characters that do not match a given predicate.
	/// @tparam TPredicate Predicate type.
	/// @param predicate Predicate to use as check.
	/// @return Count of characters removed.
	/// @note
	///		Does not resize `BaseString`, merely moves it to the end, and destructs it.
	///		If you need the `BaseString` size to change, use `erase`.
	template<class TPredicate>
	constexpr SizeType removeIfNot(TPredicate const& predicate) {
		SizeType count = BaseType::removeIfNot(predicate);
		if (!predicate(BaseType::back()))
			--count;
		return count;
	}

	/// @brief Erases a character at a given index.
	/// @param index Index of the character to erase.
	/// @return Reference to self.
	/// @throw OutOfBoundsException when index is bigger than `BaseString` size.
	/// @note
	///		Resizes the `BaseString`.
	///		If you need the `BaseString` size to remain the same, use `remove`.
	constexpr SelfType& erase(IndexType const index) {
		assertIsInBounds(index);
		wrapBounds(index);
		return BaseType::erase(index);
	}

	/// @brief Erases characters that match a given value.
	/// @param value Value to match.
	/// @return Count of characters removed.
	/// @note
	///		Resizes the `BaseString`.
	///		If you need the `BaseString` size to remain the same, use `remove`.
	constexpr SelfType& eraseLike(DataType const& value) {
		resize(removeLike(value));
		return *this;
	}

	/// @brief Erases characters that do not a given value.
	/// @param value Value to match.
	/// @return Count of characters removed.
	/// @note
	///		Resizes the `BaseString`.
	///		If you need the `BaseString` size to remain the same, use `remove`.
	constexpr SelfType& eraseUnlike(DataType const& value) {
		resize(removeUnlike(value));
		return *this;
	}

	/// @brief Erases characters that match a given predicate.
	/// @tparam TPredicate Predicate type.
	/// @param predicate Predicate to use as check.
	/// @note
	///		Resizes the `BaseString`.
	///		If you need the `BaseString` size to remain the same, use `remove`.
	template<Type::Functional<PredicateType> TPredicate>
	constexpr SelfType& eraseIf(TPredicate const& predicate) {
		resize(removeIf(predicate));
		return *this;
	}

	/// @brief Erases characters that do not match a given predicate.
	/// @tparam TPredicate Predicate type.
	/// @param predicate Predicate to use as check.
	/// @note
	///		Resizes the `BaseString`.
	///		If you need the `BaseString` size to remain the same, use `remove`.
	template<Type::Functional<PredicateType> TPredicate>
	constexpr SelfType& eraseIfNot(TPredicate const& predicate) {
		resize(removeIfNot(predicate));
		return *this;
	}

	/// @brief Returns a substring starting from a given index.
	/// @param start Starting index to copy from.
	/// @return Substring.
	/// @throw OutOfBoundsException when index is bigger than `BaseString` size.
	/// @note If index is negative, it will be interpreted as starting from the end of the `BaseString`.
	constexpr SelfType sliced(IndexType start) const {
		if (IndexType(size()) < start) return SelfType();
		wrapBounds(start);
		return SelfType(begin() + start, end());
	}

	/// @brief Returns a substring ranging from between two indices.
	/// @param start Starting index to copy from.
	/// @param stop End index to stop copying from.
	/// @return Substring.
	/// @throw OutOfBoundsException when index is bigger than `BaseString` size.
	/// @note If index is negative, it will be interpreted as starting from the end of the `BaseString`.
	constexpr SelfType sliced(IndexType start, IndexType stop) const {
		if (IndexType(size()) < start) return SelfType();
		wrapBounds(start);
		if (IndexType(size()) < stop) return sliced(start);
		wrapBounds(stop);
		if (stop < start) return SelfType();
		return SelfType(begin() + start, begin() + stop + 1);
	}

	/// @brief Appends another `BaseString` to the end of the `BaseString`.
	/// @param other `BaseString` to copy contents from.
	/// @return Reference to self.
	constexpr SelfType& appendBack(SelfType const& other) {
		expand(other.size());
		if (BaseType::size())
			BaseType::popBack();
		BaseType::appendBack(other.begin(), other.end());
		BaseType::pushBack('\0');
		return *this;
	}

	/// @brief Appends a character several times to the end of the `BaseString`.
	/// @param count Amount of character to append.
	/// @param fill Value of the characters.
	/// @return Reference to self.
	constexpr SelfType& appendBack(SizeType const count, DataType const& fill) {
		return expand(count, fill);
	}

	/// @brief Appends a range of characters to the end of the `BaseString`.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator pointing to end of range.
	/// @return Reference to self.
	constexpr SelfType& appendBack(ConstIteratorType const& begin, ConstIteratorType const& end) {
		expand(end - begin);
		if (BaseType::size())
			BaseType::popBack();
		BaseType::appendBack(begin, end);
		BaseType::pushBack('\0');
		return *this;
	}

	/// @brief Appends a range of characters to the end of the `BaseString`.
	/// @param begin Reverse iterator to beginning of range.
	/// @param end Reverse iterator pointing to end of range.
	/// @return Reference to self.
	constexpr SelfType& appendBack(ConstReverseIteratorType const& begin, ConstReverseIteratorType const& end) {
		expand(end - begin);
		if (BaseType::size())
			BaseType::popBack();
		BaseType::appendBack(begin, end);
		BaseType::pushBack('\0');
		return *this;
	}

	/// @brief Appends a fixed array of characters to the end of the `BaseString`.
	/// @tparam S Size of the array.
	/// @param values Characters to append.
	/// @return Reference to self.
	template<SizeType S>
	constexpr SelfType& appendBack(As<DataType[S]> const& values) {
		if (values[S-1] == '\0') {
			expand(S);
			if (BaseType::size())
				BaseType::popBack();
			BaseType::appendBack(values);
		} else {
			expand(S+1);
			if (BaseType::size())
				BaseType::popBack();
			BaseType::appendBack(values);
			BaseType::pushBack('\0');
		}
		return *this;
	}

	/// @brief Clears the `BaseString`.
	/// @return Reference to self.
	/// @note
	///		Does not free the underlying character array held by the `BaseString`.
	///		To actually free the underlying character array, call `dispose`.
	constexpr SelfType& clear() {
		BaseType::clear();
		BaseType::pushBack('\0');
		return *this;
	}

	/// @brief Frees the underlying character array held by the `BaseString`.
	/// @return Reference to self.
	/// @note To not free the underlying character array, call `clear`.
	constexpr SelfType& dispose() {
		BaseType::dump();
		return *this;
	}

	/// @brief Returns an iterator to the end of the `BaseString`.
	/// @return Iterator to the end of the `BaseString`.
	constexpr IteratorType				end()			{return data()+size();								}
	/// @brief Returns an iterator to the end of the `BaseString`.
	/// @return Iterator to the end of the `BaseString`.
	constexpr ConstIteratorType			end() const		{return data()+size();								}
	/// @brief Returns a reverse iterator to the end of the `BaseString`.
	/// @return Reverse iterator to the end of the `BaseString`.
	constexpr PointerType				cend()			{return data()+size();								}
	/// @brief Returns a reverse iterator to the end of the `BaseString`.
	/// @return Reverse iterator to the end of the `BaseString`.
	constexpr ConstPointerType			cend() const	{return data()+size();								}
	/// @brief Returns a reverse iterator to the beginning of the `BaseString`.
	/// @return Reverse iterator to the beginning of the `BaseString`.
	constexpr ReverseIteratorType		rbegin()		{return ReverseIteratorType(data()+size());			}
	/// @brief Returns a reverse iterator to the beginning of the `BaseString`.
	/// @return Reverse iterator to the beginning of the `BaseString`.
	constexpr ConstReverseIteratorType	rbegin() const	{return ConstReverseIteratorType(data()+size());	}

	/// @brief Returns the last character.
	/// @return Last character.
	/// @throw OutOfBoundsException when `BaseString` is empty.
	constexpr ReferenceType 	back()			{return at(size()-1);	}
	/// @brief Returns the last character.
	/// @return Last character.
	/// @throw OutOfBoundsException when `BaseString` is empty.
	constexpr DataType 			back() const	{return at(size()-1);	}

	/// @brief Returns the character at a given index.
	/// @param index Index of the character.
	/// @return Reference to the character.
	/// @throw OutOfBoundsException when `BaseString` is empty.
	constexpr DataType& at(IndexType index) {
		assertIsInBounds(index);
		wrapBounds(index);
		return BaseType::at(index);
	}

	/// @brief Returns the character at a given index.
	/// @param index Index of the character.
	/// @return Character.
	/// @throw OutOfBoundsException when `BaseString` is empty.
	constexpr DataType at(IndexType index) const {
		assertIsInBounds(index);
		wrapBounds(index);
		return BaseType::at(index);
	}

	/// @brief Returns the character at a given index.
	/// @param index Index of the character.
	/// @return Reference to the character.
	/// @throw OutOfBoundsException when `BaseString` is empty.
	constexpr ReferenceType	operator[](IndexType const index)	{return at(index);}
	/// @brief Returns the character at a given index.
	/// @param index Index of the character.
	/// @return Character.
	/// @throw OutOfBoundsException when `BaseString` is empty.
	constexpr DataType operator[](IndexType const index) const	{return at(index);}

	/// @brief Returns the current size of the underlying character array.
	/// @return Size of the underlying character array.
	constexpr SizeType capacity() const	{return BaseType::capacity() - 1;	}
	/// @brief Returns whether the string is empty.
	/// @return Whether the string is empty.
	constexpr SizeType empty() const	{return size() == 0;				}

	/// @brief Equality operator.
	/// @param other Other `BaseString` to compare with.
	/// @return Whether they're equal.
	constexpr bool operator==(SelfType const& other) const {
		return BaseType::equals(other);
	}

	/// @brief Threeway comparison operator.
	/// @param other Other `BaseString` to compare with.
	/// @return Order between both `BaseString`s.
	constexpr OrderType operator<=>(SelfType const& other) const {
		return compare(other);
	}

	/// @brief Returns whether this `BaseString` is equal to another `BaseString`.
	/// @param other Other `BaseString` to compare with.
	/// @return Whether they're equal.
	constexpr SizeType equals(SelfType const& other) const {
		return BaseType::equals(other);
	}

	/// @brief Returns the result of a threeway comparison with another `BaseString`.
	/// @param other Other `BaseString` to compare with.
	/// @return Order between both `BaseString`s.
	constexpr OrderType compare(SelfType const& other) const {
		return BaseType::compare(other);
	}

	/// @brief Returns how different this `BaseString` is from another `BaseString`.
	/// @param other Other `BaseString` to compare with.
	/// @return How different it is.
	constexpr SizeType disparity(SelfType const& other) const {
		return BaseType::disparity(other);
	}

	/// @brief Apllies a procedure to all characters in the `BaseString`.
	/// @tparam TProcedure Procedure type.
	/// @param fun Procedure to apply.
	/// @return Reference to self.
	template <Type::Functional<TransformType> TProcedure>
	constexpr SelfType& transform(TProcedure const& fun) {
		for(DataType& v: *this)
			v = fun(v);
		return *this;
	}

	/// @brief Returns a `transform`ed `BaseString`.
	/// @tparam TProcedure Procedure type.
	/// @param fun Procedure to apply.
	/// @return Transformed string.
	template<Type::Functional<TransformType> TProcedure>
	constexpr SelfType transformed(TProcedure const& fun) const {
		return SelfType(*this).transform(fun);
	}

	/// @brief Apllies a procedure to all characters in the `BaseString`.
	/// @tparam TProcedure Procedure type.
	/// @param fun Procedure to apply.
	/// @return Reference to self.
	template <Type::Functional<TransformType> TProcedure>
	constexpr SelfType& operator|=(TProcedure const& fun) {
		return transform(fun);
	}

	/// @brief Returns a `transform`ed `BaseString`.
	/// @tparam TProcedure Procedure type.
	/// @param fun Procedure to apply.
	/// @return Transformed string.
	template <Type::Functional<TransformType> TProcedure>
	constexpr SelfType operator|(TProcedure const& fun) const {
		return transformed(fun);
	}

	/// @brief Apllies a procedure to all characters in the `BaseString`.
	/// @tparam TProcedure Procedure type.
	/// @param fun Procedure to apply.
	/// @return Reference to self.
	template <Type::Functional<SelfType&(SelfType&)> TProcedure>
	constexpr SelfType& operator|=(TProcedure const& fun) {
		return fun(*this);
	}

	/// @brief Returns a copy of the `BaseString`, with the given procedure applied to it.
	/// @tparam TProcedure Procedure type.
	/// @param fun Procedure to apply.
	/// @return Transformed string.
	template <Type::Functional<SelfType(SelfType const&)> TProcedure>
	constexpr SelfType operator|(TProcedure const& fun) const {
		return fun(*this);
	}

	/// @brief Returns whether all characters match a given predicate.
	/// @tparam TPredicate Predicate type.
	/// @param cond Predicate to match.
	/// @return Whether all characters match.
	template<class TPredicate>
	constexpr bool validate(TPredicate const& cond) const {
		if (empty()) return false;
		for (DataType const& c: *this)
			if (!cond(c))
				return false;
		return true;
	}

	/// @brief Removes all characters that do not match a given predicate.
	/// @tparam TPredicate Predicate type.
	/// @param filter Predicate to match.
	/// @return Reference to self.
	template<Type::Functional<PredicateType> TPredicate>
	constexpr SelfType& filter(TPredicate const& filter) {
		return eraseIfNot(filter);
	}

	/// @brief Removes all characters that fail a given comparison.
	/// @tparam TCompare Compare type.
	/// @param compare Comparison to make.
	/// @return Reference to self.
	template<Type::Functional<CompareType> TCompare>
	constexpr SelfType& filter(TCompare const& compare) {
		return *this = filtered(compare);
	}

	/// @brief Returns a `filter`ed `BaseString`.
	/// @tparam TPredicate Predicate type.
	/// @param filter Predicate to match.
	/// @return `filter`ed `BaseString`.
	template<Type::Functional<PredicateType> TPredicate>
	constexpr SelfType filtered(TPredicate const& filter) const {
		return SelfType(*this).eraseIfNot(filter);
	}

	/// @brief Returns a `filter`ed `BaseString`.
	/// @tparam TCompare Compare type.
	/// @param compare Comparison to make.
	/// @return `filter`ed `BaseString`.
	template<Type::Functional<CompareType> TCompare>
	constexpr SelfType filtered(TCompare const& compare) const {
		SelfType result;
		for (SizeType i = 0; i < size(); ++i) {
			bool miss = false;
			for(SizeType j = size() - 1; j >= 0; --j) {
				if (i == j) break;
				if ((miss = !compare(data()[i], data()[j])))
					break;
			}
			if (!miss) result.pushBack(data()[i]);
		}
		return result;
	}

	/// @brief Returns a `BaseString` containing only unique characters.
	/// @return `BaseString` containing only unique characters.
	constexpr SelfType uniques() {
		return filtered([](ConstReferenceType a, ConstReferenceType b){return a != b;});
	}

	/// @brief Returns the current `BaseString`, divided at a given index.
	/// @param index The index to use as pivot.
	/// @return A `List`  containing the two halves of this `BaseString`.
	/// @throw OutOfBoundsException when index is bigger than `BaseString` size.
	/// @note If index is negative, it will be interpreted as starting from the end of the `BaseString`.
	constexpr List<SelfType, SizeType> divide(IndexType index) const {
		List<SelfType, SizeType> res;
		assertIsInBounds(res);
		wrapBounds(index);
		res.pushBack(sliced(0, index));
		res.pushBack(sliced(index+1));
		return res;
	}

	/// @brief Returns a string without any leading whitespace characters.
	/// @return String without leading whitespace.
	constexpr SelfType stripped() const {
		auto start	= begin();
		auto stop	= end()-1;
		while (start != stop && isNullOrSpaceChar(*start))
			++start;
		while (stop != start && isNullOrSpaceChar(*stop))
			--stop;
		if (start > stop) return SelfType();
		return SelfType(start, stop+1);
	}

	/// @brief Strips the string of any leading whitespace characters.
	/// @return reference to self.
	constexpr SelfType& strip() {
		*this = stripped();
		return *this;
	}

	/// @brief Splits the string by a separator.
	/// @param sep Separator.
	/// @return List of split strings.
	constexpr List<SelfType, SizeType> split(DataType const& sep) const {
		List<SelfType, SizeType> res;
		SelfType buf;
		for (ConstReferenceType v : *this) {
			if (v == sep) {
				res.pushBack(buf);
				buf.clear();
				continue;
			}
			buf += v;
		}
		res.pushBack(buf);
		if (res.empty()) res.pushBack(*this);
		return res;
	}

	/// @brief Splits the string by a series of separators.
	/// @param seps Separators.
	/// @return List of split strings.
	constexpr List<SelfType, SizeType> split(BaseType const& seps) const {
		List<SelfType, SizeType> res;
		SelfType buf;
		for (ConstReferenceType v : *this) {
			if (seps.find(v)) {
				res.pushBack(buf);
				buf.clear();
				continue;
			}
			buf.pushBack(v);
		}
		res.pushBack(buf);
		if (res.empty()) res.pushBack(*this);
		return res;
	}

	/// @brief Splits the string at the first character that matches the separator.
	/// @param sep Separator.
	/// @return List of split strings.
	constexpr List<SelfType, SizeType> splitAtFirst(DataType const& sep) const {
		List<SelfType, SizeType> res;
		IndexType idx = find(sep);
		if (idx < 0) res.pushBack(*this);
		else {
			res.pushBack(sliced(0, idx-1));
			res.pushBack(sliced(idx+1));
		}
		return res;
	}

	/// @brief Splits the string at the first character that matches one of the separators.
	/// @param seps Separators.
	/// @return List of split strings.
	constexpr List<SelfType, SizeType> splitAtFirst(BaseType const& seps) const {
		List<SelfType, SizeType> res;
		IndexType idx = -1;
		for(ConstReferenceType sep: seps) {
			IndexType i = find(sep);
			if (i > -1 && i < idx)
				idx = i;
		}
		if (idx < 0)	res.pushBack(*this);
		else {
			res.pushBack(sliced(0, idx-1));
			res.pushBack(sliced(idx+1));
		}
		return res;
	}

	/// @brief Splits the string at the last character that matches the separator.
	/// @param sep Separator.
	/// @return List of split strings.
	constexpr List<SelfType, SizeType> splitAtLast(DataType const& sep) const {
		List<SelfType, SizeType> res;
		IndexType idx = rfind(sep);
		if (idx < 0) res.pushBack(*this);
		else {
			res.pushBack(sliced(0, idx-1));
			res.pushBack(sliced(idx+1));
		}
		return res;
	}

	/// @brief Splits the string at the last character that matches one of the separators.
	/// @param seps Separators.
	/// @return List of split strings.
	constexpr List<SelfType, SizeType> splitAtLast(BaseType const& seps) const {
		List<SelfType, SizeType> res;
		IndexType idx = -1;
		for(ConstReferenceType sep: seps) {
			IndexType i = rfind(sep);
			if (i > -1 && i > idx)
				idx = i;
		}
		if (idx < 0) res.pushBack(*this);
		else {
			res.pushBack(sliced(0, idx-1));
			res.pushBack(sliced(idx+1));
		}
		return res;
	}

	/// @brief Replaces any character that matches, with the replacement.
	/// @param val Character to match.
	/// @param rep Replacement.
	/// @return Reference to self.
	constexpr SelfType& replace(DataType const& val, DataType const& rep) {
		for (DataType& v: *this)
			if (v == val) v = rep;
		return *this;
	}

	/// @brief Replaces any character that matches the set, with the replacement.
	/// @param values Characters to match.
	/// @param rep Replacement.
	/// @return Reference to self.
	constexpr SelfType& replace(BaseType const& values, DataType const& rep) {
		for (DataType const& val: values)
			replace(val, rep);
		return *this;
	}

	/// @brief Character replacement rule.
	struct Replacement {
		/// @brief Characters to replace.
		BaseType	targets;
		/// @brief Character to replace with.
		DataType	replacement;
	};

	/// @brief Replaces characters, acoording to a given replacement rule.
	/// @param rep Replacement rule.
	/// @return Reference to self.
	constexpr SelfType& replace(Replacement const& rep) {
		replace(rep.targets, rep.replacement);
		return *this;
	}

	/// @brief Replaces characters, acoording to a given list of rules.
	/// @param reps Replacement rules.
	/// @return Reference to self.
	constexpr SelfType& replace(List<Replacement, SizeType> const& reps) {
		for (Replacement const& rep: reps)
			replace(rep);
		return *this;
	}

	/// @brief Returns a string with any character that matches replaced.
	/// @param val Character to match.
	/// @param rep Replacement.
	/// @return Resulting string.
	constexpr SelfType replaced(DataType const& val, DataType const& rep) const				{return SelfType(*this).replace(val, rep);		}
	/// @brief Returns a string with any character that matches replaced.
	/// @param values Characters to match.
	/// @param rep Replacement.
	/// @return Resulting string.
	constexpr SelfType replaced(BaseType const& values, DataType const& rep) const			{return SelfType(*this).replace(values, rep);	}
//	constexpr SelfType replaced(ArgumentListType const& values, DataType const& rep) const	{return SelfType(*this).replace(values, rep);	}

	/// @brief Returns a string with any rule that matches replaced.
	/// @param rep Replacement rule.
	/// @return Resulting string.
	constexpr SelfType replaced(Replacement const& rep) const					{return SelfType(*this).replace(rep);	}
	/// @brief Returns a string with any rule that matches replaced.
	/// @param reps Replacement rules.
	/// @return Resulting string.
	constexpr SelfType replaced(List<Replacement, SizeType> const& reps) const	{return SelfType(*this).replace(reps);	}
//	constexpr SelfType replaced(Arguments<Replacement> const& reps) const		{return SelfType(*this).replace(reps);	}

	/// @brief Stream insertion operator.
	constexpr OutputStreamType& operator<<(OutputStreamType& o) const	{if (!empty()) o << cstr(); return o;}
	/// @brief Stream insertion operator.
	constexpr OutputStreamType& operator<<(OutputStreamType& o)			{if (!empty()) o << cstr(); return o;}

	/// @brief Stream insertion operator.
	friend constexpr OutputStreamType& operator<<(OutputStreamType& o, SelfType& self)			{if (!self.empty()) o << self.cstr(); return o;}
	/// @brief Stream insertion operator.
	friend constexpr OutputStreamType& operator<<(OutputStreamType& o, SelfType const& self)	{if (!self.empty()) o << self.cstr(); return o;}

	/// @brief Reads from an input stream until a character is reached.
	/// @param i Input stream.
	/// @param stop Stop character.
	/// @return Input stream.
	constexpr InputStreamType& readFrom(InputStreamType& i, DataType const& stop) {
		DataType buf[32];
		while(i.getline(buf, 32, stop))
			appendBack(buf, i.gcount());
		return i;
	}

	/// @brief Reads from an input stream until a null character is reached.
	/// @param i Input stream.
	/// @return Input stream.
	constexpr InputStreamType& readFrom(InputStreamType& i) {
		return readFromStream(i, '\0');
	}

	/// @brief Copy assignment operator (`BaseString`).
	/// @param other `BaseString` to copy from.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType const& other)			{BaseType::operator=(other); return *this;				}
	/// @brief Move assignment operator (`BaseString`).
	/// @param other `BaseString` to move.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType&& other)					{BaseType::operator=(CTL::move(other)); return *this;	}
	/// @brief Copy assignment operator (null-terminated string).
	/// @param other String to copy from.
	/// @return Reference to self.
	constexpr SelfType& operator=(CStringType const& other)			{BaseType::operator=(SelfType(other)); return *this;	}

	/// @brief Insertion operator (`BaseString`).
	/// @param other `Basestring` to insert into.
	/// @return Const reference to self.
	constexpr SelfType const& operator<<(SelfType& other) const	{other.appendBack(*this); return *this;}
	/// @brief Insertion operator (`BaseString`).
	/// @param other `Basestring` to insert into.
	/// @return Reference to self.
	constexpr SelfType& operator<<(SelfType& other)				{other.appendBack(*this); return *this;}

	/// @brief Extraction operator (`BaseString`).
	/// @param other `Basestring` to extract from.
	/// @return Reference to self.
	constexpr SelfType& operator>>(SelfType const& other)	{appendBack(other); return other;}

	/// @brief String concatenation operator (character).
	/// @param value Character to concatenate.
	/// @return Resulting concatenated string.
	constexpr SelfType operator+(DataType const& value) const	{return SelfType(*this).pushBack(value);	}
	/// @brief String concatenation operator (`BaseString`).
	/// @param value `BaseString` to concatenate.
	/// @return Resulting concatenated string.
	constexpr SelfType operator+(SelfType const& other) const	{return SelfType(*this).appendBack(other);	}

	/// @brief String concatenation operator (null-terminated string).
	/// @param value String to concatenate.
	/// @return Resulting concatenated string.
	constexpr SelfType operator+(CStringType const& str) const			{return (*this) + SelfType(str);}
	/// @brief String concatenation operator (char array).
	/// @tparam S Array size.
	/// @param value Char array to concatenate.
	/// @return Resulting concatenated string.
	template<SizeType S>
	constexpr SelfType operator+(As<ConstantType[S]> const& str) const	{return (*this) + SelfType(str);}

	/// @brief String concatenation operator (character).
	/// @param value Character to concatenate.
	/// @param self `BaseString` to concatenate with.
	/// @return Resulting concatenated string.
	friend constexpr SelfType operator+(DataType const& value, SelfType const& self)	{return SelfType().pushBack(value) + self;	}

	/// @brief String concatenation operator (null-terminated string).
	/// @param value String to concatenate.
	/// @param self `BaseString` to concatenate with.
	/// @return Resulting concatenated string.
	friend constexpr SelfType operator+(CStringType const& str, SelfType const& self)			{return SelfType(str) + (self);}
	/// @brief String concatenation operator (char array).
	/// @tparam S Array size.
	/// @param value Char array to concatenate.
	/// @param self `BaseString` to concatenate with.
	/// @return Resulting concatenated string.
	template<SizeType S>
	friend constexpr SelfType operator+(As<ConstantType[S]> const& str, SelfType const& self)	{return SelfType(str) + (self);}

	/// @brief String appending operator (character).
	/// @param value Caracter to append.
	/// @return Reference to self.
	constexpr SelfType& operator+=(DataType const& value)				{pushBack(value); return *this;				}
	/// @brief String appending operator (`BaseString`).
	/// @param value `BaseString` to append.
	/// @return Reference to self.
	constexpr SelfType& operator+=(SelfType const& other)				{appendBack(other); return *this;			}
	/// @brief String appending operator (null-terminated string).
	/// @param value String to append.
	/// @return Reference to self.
	constexpr SelfType& operator+=(CStringType const& str)				{appendBack(SelfType(str)); return *this;	}
	/// @brief String appending operator (char array).
	/// @tparam S Array size.
	/// @param value Char array to append.
	/// @return Reference to self.
	template<SizeType S>
	constexpr SelfType& operator+=(As<ConstantType[S]> str)				{appendBack(str); return *this;				}

	/// @brief Returns a string, repeated a given amount of times.
	/// @param times Amount of times to repeat.
	/// @return Resulting string.
	constexpr SelfType operator*(IndexType const times) const {
		if (times < 1) return SelfType();
		if (times == 1) return *this;
		SelfType result(size() * static_cast<usize>(times));
		for (SizeType i = 0; i < static_cast<usize>(times); ++i)
			result.appendBack(*this);
		return result;
	}

	/// @brief Repeats the string a given amount of times.
	/// @param times Amount of times to repeat.
	/// @return Reference to self.
	constexpr SelfType& operator*=(IndexType const times) {
		if (times < 1) return SelfType();
		if (times == 1) return *this;
		SelfType copy = *this;
		for (SizeType i = 0; i < times; ++i)
			appendBack(copy);
		return *this;
	}

	/// @brief Equality comparison operator (char array).
	/// @tparam S Array size.
	/// @param str Array to compare with.
	/// @return Whether they're equal.
	template<SizeType S>
	constexpr bool operator==(As<ConstantType[S]> const& str) const	{return *this == SelfType(str);			}
	/// @brief Equality comparison operator (null-terminated string).
	/// @param str String to compare with.
	/// @return Whether they're equal.
	constexpr bool operator==(CStringType const& str) const			{return *this == SelfType(str);			}
	/// @brief Threeway comparison operator (char array).
	/// @tparam S Array size.
	/// @param str Char array to compare with.
	/// @return Order between objects.
	template<SizeType S>
	constexpr OrderType operator<=>(As<ConstantType[S]> const& str) const	{return *this <=> SelfType(str);		}
	/// @brief Threeway comparison operator (null-terminated string).
	/// @param str String to compare with.
	/// @return Order between objects.
	constexpr OrderType operator<=>(CStringType const& str) const			{return *this <=> SelfType(str);		}

	/// @brief Returns this string as a "C-style" string.
	constexpr explicit operator const char*() const {return cstr();}

	/// @brief Converts the `BaseString` to a `BaseString` of a different character type.
	/// @tparam C Character type.
	/// @note Character type must be different from this `BaseString`'s character type.
	template <Type::ASCII C>
	constexpr operator BaseString<C, IndexType>() const
	requires Type::Different<DataType, C> {
		BaseString<C, IndexType> result(size(), '\0');
		for (SizeType i = 0; i < size(); ++i)
			result[i] = data()[i];
		return result;
	}

	/// @brief Returns a substring, starting at a given point.
	/// @param start Start of new string.
	/// @return Resulting substring.
	SelfType substring(IndexType const start) const {
		return sliced(start);
	}

	/// @brief Returns a substring, starting at a given point, and going for a given size.
	/// @param start Start of new string.
	/// @param length Length of new string.
	/// @return Resulting substring.
	SelfType substring(IndexType start, SizeType const length) const {
		assertIsInBounds(start);
		wrapBounds(start);
		while (start < 0) start += size();
		return sliced(start, start + length);
	}

	/// @brief Returns the string as a "c-style" string.
	/// @return Pointer to beginning of the "c-style" string.
	constexpr CStringType cstr() const {
		return data();
	}

	/// @brief Returns the string's size.
	/// @return Size of string.
	constexpr SizeType size() const {
		if (BaseType::empty()) return 0;
		return BaseType::size() - 1;
	}


	/// @brief Returns the string as lowercase.
	/// @return Lowercase string.
	constexpr SelfType lower() const {return transformed(toLowerImpl);	}
	/// @brief Returns the string as uppercase.
	/// @return Uppercase string.
	constexpr SelfType upper() const {return transformed(toUpperImpl);	}

	/// @brief Returns whether the string represents a hex value.
	/// @return Whether the string represents a hex value.
	constexpr bool isHex() const			{return validate(isHexImpl);			}
	/// @brief Returns whether the string is composed fully of null or whitespace characters.
	/// @return Whether the string is null or entirely whitespace.
	constexpr bool isNullOrSpaces() const	{return validate(isNullOrSpaceImpl);	}

	// Most likely wrong
	/// @brief Converts the string to a `char` string.
	/// @return String as `char` string.
	constexpr BaseString<char, SizeType> toString() const
	requires Type::Equal<DataType, wchar> {
		BaseString<char, SizeType> result;
		for (DataType const& c: *this)
			result.pushBack(char(c));
		return result;
	}

	/// @brief Converts the string to a `char` string.
	/// @return String as `char` string.
	constexpr SelfType toString() const
	requires Type::Equal<DataType, char> {return *this;}

	// Most likely wrong
	/// @brief Converts the string to a `wchar` string.
	/// @return String as `wchar` string.
	constexpr BaseString<wchar, SizeType> toWideString() const
	requires Type::Equal<DataType, char> {
		BaseString<wchar, SizeType> result;
		for (DataType const& c: *this)
			result.pushBack(wchar(c));
		return result;
	}

	/// @brief Converts the string to a `wchar` string.
	/// @return String as `wchar` string.
	constexpr SelfType toWideString() const
	requires Type::Equal<DataType, wchar> {return *this;}

	/// @brief String-to-boolean conversion.
	///@tparam T Boolean type (`bool`).
	/// @param str String to convert.
	/// @return Converted value as a boolean.
	template<Type::Equal<bool> T>
	constexpr static T toNumber(SelfType const& str) {
		if (str == "true") return true;
		if (str == "false") return false;
		return toNumber<uint8>(str);
	}

	/// @brief String-to-integer conversion.
	///@tparam T Integer type.
	/// @param str String to convert.
	/// @param base Base of the string. Will be used, if non-zero.
	/// @return Converted value as number.
	/// @throw FailedActionException if conversion fails.
	template<Type::Integer T>
	constexpr static T toNumber(SelfType const& str, T const& base = 0)
	requires Type::Different<T, bool> {
		T val = T();
		if (!atoi<T, DataType>(str.data(), str.size(), val, base))
			throw FailedActionException("String-to-Integer conversion failure!");
		return val;
	}

	/// @brief String-to-float conversion.
	///@tparam T Floating point type.
	/// @param str String to convert.
	/// @param base Base of the string. Will be used, if non-zero.
	/// @return Converted value as number.
	/// @throw FailedActionException if conversion fails.
	template<Type::Real T>
	constexpr static T toNumber(SelfType const& str, usize const base = 0) {
		T val = T();
		if (!atof<T, DataType>(str.data(), str.size(), val, base))
			throw FailedActionException("String-to-Float conversion failure!");
		return val;
	}

	/// @brief Boolean-to-string conversion.
	///@tparam T Boolean type (`bool`).
	/// @param val Value to convert.
	/// @param text Whether to convert as text ("true" or "false"), or a number (0 or 1).
	/// @return Converted value as string.
	template<Type::Equal<bool> T>
	constexpr static SelfType fromNumber(T const& val, bool const text = false) {
		if (text) return val ? "true" : "false";
		return val ? "1" : "0";
	}

	/// @brief Integer-to-string conversion.
	/// @tparam T Integer type.
	/// @param val Value to convert.
	/// @param base Base to convert as. By default, it is base 10.
	/// @return Converted value as string.
	/// @throw FailedActionException if conversion fails.
	template<Type::Integer T>
	constexpr static SelfType fromNumber(T const& val, T const& base = 10)
	requires Type::Different<T, bool> {
		SelfType result(sizeof(T)*4, '\0');
		ssize sz = itoa<T, DataType>(val, result.data(), result.size(), base);
		if (sz < 0) throw FailedActionException("Integer-to-String conversion failure!");
		result.resize(sz);
		return result;
	}

	/// @brief Floating-point-to-string conversion.
	/// @tparam T Floating point type.
	/// @param val Value to convert.
	/// @param
	///		precision Amount of decimal spaces to include.
	///		By default, it is equal to double the byte size of the floating point type.
	/// @return Converted value as string.
	/// @throw FailedActionException if conversion fails.
	/// @note
	///		Default value of `precision` for:
	///
	///		- `float`s: 8 decimal spaces.
	///
	///		- `double`s: 16 decimal spaces.
	///
	///		- `long double`s: 32 decimal spaces.
	template<Type::Real T>
	constexpr static SelfType fromNumber(T const& val, usize const precision = sizeof(T)*2) {
		SelfType result(sizeof(T)*4, '\0');
		ssize sz = ftoa<T, DataType>(val, result.data(), result.size(), precision);
		if (sz < 0) throw FailedActionException("Float-to-String conversion failure!");
		result.resize(sz);
		return result;
	}

	/// @brief Swap algorithm for `BaseString`.
	/// @param a `BaseString` to swap.
	/// @param b `BaseString` to swap with.
	friend constexpr void swap(SelfType& a, SelfType& b) noexcept {
		swap<BaseType>(a, b);
	}

	/// @brief Returns an STL view for the string.
	/// @return STL view for the string.
	constexpr STDViewType stdView() const		{return STDViewType(cbegin(), cend());		}
	/// @brief Returns the string as an STL string.
	/// @return String as an STL string.
	constexpr STDStringType std() const			{return STDStringType(cbegin(), cend());	}
	/// @brief Returns the string as an STL string.
	/// @return String as an STL string.
	constexpr operator STDStringType() const	{return std();								}

private:
	constexpr static DataType toLowerImpl(DataType const& c)	{return toLowerChar<DataType>(c);		}
	constexpr static DataType toUpperImpl(DataType const& c)	{return toUpperChar<DataType>(c);		}
	constexpr static bool isHexImpl(DataType const& c)			{return isHexChar<DataType>(c);			}
	constexpr static bool isNullOrSpaceImpl(DataType const& c)	{return isNullOrSpaceChar<DataType>(c);	}

	[[noreturn]] void invalidNumberError(CStringType const& v) const {
		throw InvalidValueException("Not a valid number!");
	}

	constexpr void assertIsInBounds(IndexType const index) const {
		if (index >= 0 && usize(index) > (size()-1)) outOfBoundsError(index);
	}

	constexpr void wrapBounds(IndexType& index) const {
		Iteratable::wrapBounds(index, size());
	}

	[[noreturn]] constexpr void outOfBoundsError(IndexType const index) const {
		throw OutOfBoundsException("Index is out of bounds!");
	}

	[[noreturn]] constexpr static void emptyError() {
		throw OutOfBoundsException("String is empty!");
	}
};

/// @brief Stream extraction operator overloading.
/// @tparam TChar Character type.
/// @tparam TIndex Index type.
/// @param stream Stream to extract from.
/// @param string `BaseString` to extract to.
/// @return Input stream.
template<Type::ASCII TChar, Type::Integer TIndex = usize>
constexpr typename BaseString<TChar, TIndex>::InputStreamType& operator>>(
	typename BaseString<TChar, TIndex>::InputStreamType& stream,
	BaseString<TChar, TIndex>& string
) {
	return string.readFrom(stream);
}

/// @brief String concatenation operator.
/// @tparam TChar Character type.
/// @tparam TIndex Index type.
/// @param str String to append.
/// @param self `BaseString` to append to.
/// @return Resulting concatenated string.
template<Type::ASCII TChar, Type::Integer TIndex = usize>
constexpr BaseString<TChar, TIndex>
operator+(
	typename BaseString<TChar, TIndex>::CStringType const& str,
	BaseString<TChar, TIndex> const& self
) {
	return BaseString<TChar, TIndex>(str) + self;
}

/// @brief String concatenation operator.
/// @tparam TChar Character type.
/// @tparam TIndex Index type.
/// @tparam S Array size.
/// @param str Char array to append.
/// @param self `BaseString` to append to.
/// @return Resulting concatenated string.
template<Type::ASCII TChar, Type::Integer TIndex = usize, AsUnsigned<TIndex> S>
constexpr BaseString<TChar, TIndex>
operator+(
	As<typename BaseString<TChar, TIndex>::ConstantType[S]> const& str,
	BaseString<TChar, TIndex> const& self
) {
	return BaseString<TChar, TIndex>(str) + self;
}

/// `BaseString` analog for a `char` string.
typedef BaseString<char>	String;
/// `BaseString` analog for a `wchar` string.
typedef BaseString<wchar_t>	WideString;

//static_assert(String("Compile-time Magics!").size());

/// @brief Static string of characters.
/// @tparam TChar Character type.
/// @tparam N String size.
/// @tparam TIndex Index type.
template<Type::ASCII TChar, usize N, Type::Integer TIndex = usize>
struct BaseStaticString:
	Array<TChar, N, TIndex>,
	CStringable<TChar>,
	SelfIdentified<BaseStaticString<TChar, N, TIndex>>,
	Derived<Array<TChar, N, TIndex>> {
public:
	using Derived			= Derived<Array<TChar, N, TIndex>>;
	using SelfIdentified	= SelfIdentified<BaseStaticString<TChar, N, TIndex>>;
	using CStringable	= CStringable<TChar>;

	using typename Derived::BaseType;

	using
		typename SelfIdentified::SelfType
	;

	using
		typename BaseType::DataType,
		typename BaseType::ConstPointerType,
		typename BaseType::SizeType,
		typename BaseType::IndexType
	;

	using
		BaseType::BaseType,
		BaseType::data,
		BaseType::cbegin,
		BaseType::cend,
		BaseType::begin,
		BaseType::end,
		BaseType::rbegin,
		BaseType::rend
	;

	using BaseType::SIZE, BaseType::MAX_SIZE;

	using typename CStringable::CStringType;

private:
	constexpr static IndexType wrapAround(IndexType value) {
		while (value < 0) value += SIZE;
		return value;
	}

public:
	/// @brief Constructs the `BaseStaticString` from a null-terminated string.
	/// @param str String to copy from.
	constexpr BaseStaticString(CStringType const& str) {
		SizeType len = 0;
		while (str[len++] != '\0' && len <= MAX_SIZE);
		SizeType const sz = (len < SIZE ? len : SIZE);
		if (inCompileTime())
			for (SizeType i = 0; i < sz; ++i)
				data()[i] = str[i];
		else MX::memcpy(str, data(), sz);
	}

	/// @brief Returns a static substring, starting at a given point, and going for a given size.
	/// @tparam BEGIN Start of new string.
	/// @tparam S Length of new string.
	/// @return Resulting substring.
	template<IndexType BEGIN, SizeType S = SIZE>
	constexpr auto substring() const {
		constexpr SizeType start	= wrapAround(BEGIN);
		constexpr SizeType stop		= ((start + S) < SIZE) ? start + S : SIZE;
		BaseStaticString<TChar, stop - start + 1, TIndex> result('\0');
		SizeType const sz = stop - start;
		if (inCompileTime())
			for (SizeType i = 0; i < sz; ++i)
				data()[i] = result.data()[i];
		else MX::memcpy(result.data(), data() + start, sz);
		return result;
	}

	/// @brief Returns the static string as a "c-style" string.
	/// @return Static string as a "c-style" string.
	constexpr CStringType cstr() const {
		return data();
	}

	/// @brief Returns the static string as a dynamic string.
	/// @return Static string as dynamic string.
	constexpr BaseString<TChar, TIndex> toString() const {
		return BaseString<TChar, TIndex>(begin(), end());
	}
};

/// @brief List of strings.
typedef List<String>			StringList;
/// @brief Pair of strings.
typedef Pair<String, String>	StringPair;

/// @brief `BaseStaticString` analog for a `char` static string.
/// @tparam N Static string size.
template<usize N> using StaticString		= BaseStaticString<char,	N>;
/// @brief `BaseStaticString` analog for a `wchar` static string.
/// @tparam N Static string size.
template<usize N> using StaticWideString	= BaseStaticString<wchar_t,	N>;

#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wliteral-suffix"
#else
#pragma GCC diagnostic ignored "-Wuser-defined-literals"
#endif
/// @brief String literals.
namespace Literals::Text {
	/// @brief CTL `String` literal.
	constexpr String operator "" s		(cstring cstr, litsize sz)	{return String(cstr, sz);					}
	/// @brief CTL `String` literal.
	constexpr String operator "" s		(cwstring cstr, litsize sz)	{return WideString(cstr, sz).toString();	}
	/// @brief CTL `WideString` literal.
	constexpr WideString operator "" ws	(cstring cstr, litsize sz)	{return String(cstr, sz).toWideString();	}
	/// @brief CTL `WideString` literal.
	constexpr WideString operator "" ws	(cwstring cstr, litsize sz)	{return WideString(cstr, sz);				}
}
#pragma GCC diagnostic pop

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_STRING_H
