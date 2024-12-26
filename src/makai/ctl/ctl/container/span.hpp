#ifndef CTL_CONTAINER_SPAN_H
#define CTL_CONTAINER_SPAN_H

#include "../ctypes.hpp"
#include "../templates.hpp"
#include "../typetraits/traits.hpp"
#include "../namespace.hpp"
#include "../cpperror.hpp"
#include "../adapter/comparator.hpp"
#include "iterator.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Span extent size deduction.
enum class ExtentSize {
	CES_AUTO,
	CES_STATIC,
	CES_DYNAMIC
};

/// @brief Integer value representing dynamic size.
constexpr usize DYNAMIC_SIZE = -1;

template<class TData, usize S = DYNAMIC_SIZE, Type::Integer TIndex = usize, ExtentSize EXTENT = ExtentSize::CES_AUTO>
struct Span;

/// @brief Fixed-size, or variable-size, view of a span of elements.
/// @tparam TData Element type.
/// @tparam S Span size.
/// @tparam TIndex Index size.
/// @tparam EXTENT Extent size deduction.
/// @note Even if `S` is specified, the actual `Span` size is not guaranteed to be `S`.
template<class TData, usize S, Type::Integer TIndex, ExtentSize EXTENT>
struct Span:
	Iteratable<TData, TIndex>,
	SelfIdentified<Span<TData, S, TIndex, EXTENT>>,
	Ordered {

	using Iteratable		= ::CTL::Iteratable<TData, TIndex>;
	using SelfIdentified	= ::CTL::SelfIdentified<Span<TData, S, TIndex, EXTENT>>;

	using
		typename Iteratable::IteratorType,
		typename Iteratable::ConstIteratorType,
		typename Iteratable::ReverseIteratorType,
		typename Iteratable::ConstReverseIteratorType,
		typename Iteratable::IndexType,
		typename Iteratable::SizeType
	;

	using
		typename Iteratable::DataType,
		typename Iteratable::ReferenceType,
		typename Iteratable::PointerType,
		typename Iteratable::ConstPointerType,
		typename Iteratable::ConstantType,
		typename Iteratable::ConstReferenceType
	;

	using typename SelfIdentified::SelfType;

	using Ordered::OrderType;

	constexpr static bool STATIC	= (S != DYNAMIC_SIZE || EXTENT == ExtentSize::CES_STATIC);
	constexpr static bool DYNAMIC	= (S == DYNAMIC_SIZE && EXTENT != ExtentSize::CES_STATIC);

	using ComparatorType = SimpleComparator<DataType>;

	/// @brief Empty constructor.
	constexpr Span() noexcept: contents(nullptr), count(0) {}

	/// @brief Copy constructor (`Span`).
	/// @param other Other `Span` to copy from.
	constexpr Span(SelfType const& other) noexcept: contents(other.contents), count(other.count)					{}
	/// @brief Move constructor (`Span`).
	/// @param other Other `Span` to move.
	constexpr Span(SelfType&& other) noexcept: contents(CTL::move(other.contents)), count(CTL::move(other.count))	{}

	/// @brief Constructs a `Span` from a pointer to a range of elements.
	/// @param data Elements to view.
	/// @note Becomes explicit if span is dymamic.
	constexpr explicit(DYNAMIC) Span(PointerType const& data): contents(data)													{}
	/// @brief Constructs a `Span` from an iterator to the beginning of a span of elements.
	/// @param data Elements to view.
	/// @note Becomes explicit if span is dymamic.
	constexpr explicit(DYNAMIC) Span(IteratorType const& begin): contents(begin)												{}

	/// @brief Constructs a `Span` from a "C-style" span of elements.
	/// @param data Start of span.
	/// @param size Size of span.
	/// @note Becomes explicit if span is static.
	constexpr explicit(STATIC) Span(PointerType const& data, SizeType const& size): contents(data), count(size)					{}
	/// @brief Constructs a `Span` from a span of elements.
	/// @param begin Iterator to beginning of span.
	/// @param end Iterator to end of span.
	/// @note Becomes explicit if span is static.
	constexpr explicit(STATIC) Span(IteratorType const& begin, IteratorType const& end): contents(begin), count(end - begin)	{}

	/// @brief Constructs a `Span` from a ranged object.
	/// @tparam T Ranged object type.
	/// @param other Object to view from.
	template<Type::Container::Ranged<IteratorType, ConstIteratorType> T>
	constexpr explicit Span(T const& other)
	requires requires {
		requires !Type::Derived<T, SelfType>;
	}: Span(other.begin(), other.end())	{}
	/// @brief Constructs a `Span` from a bounded object.
	/// @tparam T Bounded object type.
	/// @param other Object to view from.
	template<Type::Container::Bounded<PointerType, SizeType> T>
	constexpr explicit Span(T const& other)
	requires requires {
		requires !Type::Derived<T, SelfType>;
		requires !Type::Container::Ranged<T, IteratorType, ConstIteratorType>;
	}: Span(other.data(), other.size())	{}

	/// @brief Returns the value of the element at a given index.
	/// @param index Index of the element.
	/// @return Reference to the element.
	/// @throw OutOfBoundsException when index is bigger than `Span` size.
	/// @throw NonexistentValueException when no span is bound, or `Span` is empty.
	constexpr ReferenceType at(IndexType index) {
		assertExists();
		wrapBounds(index, count);
		return contents[index];
	}

	/// @brief Returns the value of the element at a given index.
	/// @param index Index of the element.
	/// @return Const reference to the element.
	/// @throw OutOfBoundsException when index is bigger than `Span` size.
	/// @throw NonexistentValueException when no span is bound, or `Span` is empty.
	constexpr ConstReferenceType at(IndexType index) const {
		assertExists();
		wrapBounds(index, count);
		return contents[index];
	}

	/// @brief Returns the value of the element at a given index.
	/// @param index Index of the element.
	/// @return Reference to the element.
	/// @throw OutOfBoundsException when index is bigger than `Span` size.
	/// @throw NonexistentValueException when no span is bound, or `Span` is empty.
	constexpr ReferenceType operator[](IndexType index)				{return at(index);	}

	/// @brief Returns the value of the element at a given index.
	/// @param index Index of the element.
	/// @return Const reference to the element.
	/// @throw OutOfBoundsException when index is bigger than `Span` size.
	/// @throw NonexistentValueException when no span is bound, or `Span` is empty.
	constexpr ConstReferenceType operator[](IndexType index) const	{return at(index);	}

	/// @brief Returns a pointer to the beginning of the span.
	/// @return Pointer to beginning of span.
	constexpr PointerType		data()			{return contents;	}
	/// @brief Returns a pointer to the beginning of the span.
	/// @return Pointer to beginning of span.
	constexpr ConstPointerType	data() const	{return contents;	}


	/// @brief Returns an iterator to the beginning of the `Span`.
	/// @return Iterator to the beginning of the `Span`.
	constexpr IteratorType		begin()			{return contents;		}
	/// @brief Returns an iterator to the end of the `Span`.
	/// @return Iterator to the end of the `Span`.
	constexpr IteratorType		end()			{return contents+count;	}
	/// @brief Returns an iterator to the beginning of the `Span`.
	/// @return Iterator to the beginning of the `Span`.
	constexpr ConstIteratorType	begin() const	{return contents;		}
	/// @brief Returns an iterator to the end of the `Span`.
	/// @return Iterator to the end of the `Span`.
	constexpr ConstIteratorType	end() const		{return contents+count;	}

	/// @brief Returns a reverse iterator to the beginning of the `Span`.
	/// @return Reverse iterator to the beginning of the `Span`.
	constexpr ReverseIteratorType		rbegin()		{return ReverseIteratorType(contents+count);		}
	/// @brief Returns a reverse iterator to the end of the `Span`.
	/// @return Reverse iterator to the end of the `Span`.
	constexpr ReverseIteratorType		rend()			{return ReverseIteratorType(contents);				}
	/// @brief Returns a reverse iterator to the beginning of the `Span`.
	/// @return Reverse iterator to the beginning of the `Span`.
	constexpr ConstReverseIteratorType	rbegin() const	{return ConstReverseIteratorType(contents+count);	}
	/// @brief Returns a reverse iterator to the end of the `Span`.
	/// @return Reverse iterator to the end of the `Span`.
	constexpr ConstReverseIteratorType	rend() const	{return ConstReverseIteratorType(contents);			}

	/// @brief Returns a pointer to the beginning of the `Span`.
	/// @return Pointer to the beginning of the `Span`.
	constexpr PointerType	cbegin()		{return contents;		}
	/// @brief Returns a pointer to the end of the `Span`.
	/// @return Pointer to the end of the `Span`.
	constexpr PointerType	cend()			{return contents+count;	}
	/// @brief Returns a pointer to the beginning of the `Span`.
	/// @return Pointer to the beginning of the `Span`.
	constexpr ConstantType*	cbegin() const	{return contents;		}
	/// @brief Returns a pointer to the end of the `Span`.
	/// @return Pointer to the end of the `Span`.
	constexpr ConstantType*	cend() const	{return contents+count;	}
	
	/// @brief Returns the value of the first element.
	/// @return Reference to the first element.
	/// @throw NonexistentValueException when no span is bound, or `Span` is empty.
	constexpr ReferenceType		front()			{return at(0);			}
	/// @brief Returns the value of the last element.
	/// @return Reference to the last element.
	/// @throw NonexistentValueException when no span is bound, or `Span` is empty.
	constexpr ReferenceType 	back()			{return at(count-1);	}
	/// @brief Returns the value of the first element.
	/// @return Value of the first element.
	/// @throw NonexistentValueException when no span is bound, or `Span` is empty.
	constexpr DataType			front() const	{return at(0);			}
	/// @brief Returns the value of the last element.
	/// @return Value of the last element.
	/// @throw NonexistentValueException when no span is bound, or `Span` is empty.
	constexpr DataType			back() const	{return at(count-1);	}

	/// @brief Returns the size of the span.
	/// @return Span size.
	constexpr SizeType size() const	{return count;		}
	/// @brief Returns whether the span is empty.
	/// @return Whether the span is empty.
	constexpr bool empty() const	{return count == 0;	}

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

	/// @brief Equality operator.
	/// @param other Other `Span` to compare with.
	/// @return Whether they're equal.
	/// @note Requires element type to be equally comparable.
	/// @sa Comparator::equals()
	constexpr bool operator==(SelfType const& other) const
	requires Type::Comparator::Equals<DataType, DataType> {
		return equals(other);
	}

	/// @brief Threeway comparison operator.
	/// @param other Other `Span` to compare with.
	/// @return Order between both `Span`s.
	/// @note Requires element type to be threeway comparable.
	/// @sa Comparator::compare()
	constexpr OrderType operator<=>(SelfType const& other) const
	requires Type::Comparator::Threeway<DataType, DataType> {
		return compare(other);
	}

	/// @brief Returns whether it is equal to another `Span`.
	/// @param other Other `Span` to compare with.
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

	/// @brief Returns the result of a threeway comparison with another `Span`.
	/// @param other Other `Span` to compare with.
	/// @return Order between both `Span`s.
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
	}
	

private:
	constexpr void assertExists() {
		if (!(contents && count))
			throw NonexistentValueException("No element range bound to span!");
	}

	using Iteratable::wrapBounds;

	DataType*	contents	= nullptr;
	usize		count		= S;
};

/// @brief `Span` analog for a viewable set of bytes.
/// @tparam S Span size.
/// @tparam TIndex Index type.
/// @tparam EXTENT Extent size deduction.
template<usize S = DYNAMIC_SIZE, Type::Integer TIndex = usize, ExtentSize EXTENT = ExtentSize::CES_AUTO>
using ByteSpan = Span<uint8, S, TIndex, EXTENT>;

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_SPAN_H
