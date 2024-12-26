#ifndef CTL_CONTAINER_ITERATOR_H
#define CTL_CONTAINER_ITERATOR_H

#include <iterator>

#include "../namespace.hpp"
#include "../order.hpp"
#include "../ctypes.hpp"
#include "../templates.hpp"
#include "../cpperror.hpp"
#include "../typetraits/traits.hpp"
#include "../meta/logic.hpp"

CTL_NAMESPACE_BEGIN

/*template<class TData, bool R = false, Type::Integer TIndex = usize>
struct STLIteratorCompatible {
	using iterator_category = Meta::DualType<R, std::bidirectional_iterator_tag>;
    using value_type = TData;
    using difference_type = AsUnsigned<TIndex>;
    using pointer = TData*;
    using reference = TData&;
};*/

/// @brief Iterator.
/// @tparam TData Type of object the iterator is pointing to.
/// @tparam R Whether it is a reverse (`R = true`) or forward (`R = false`) iterator.
/// @tparam TIndex Index type.
template<class TData, bool R = false, Type::Integer TIndex = usize>
struct Iterator:
//	STLIteratorCompatible<TData, R, TIndex>,
	Typed<TData>,
	Indexed<TIndex>,
	Ordered,
	SelfIdentified<Iterator<TData, R, TIndex>> {
public:
	/// @brief Whether the `Iterator` is a reverse (`R = true`) or forward (`R = false`) iterator.
	constexpr static bool REVERSE = R;

	using Typed				= ::CTL::Typed<TData>;
	using Indexed			= ::CTL::Indexed<TIndex>;
	using Ordered			= ::CTL::Ordered;
	using SelfIdentified	= ::CTL::SelfIdentified<Iterator<TData, REVERSE, TIndex>>;

	using
		typename Typed::DataType,
		typename Typed::ConstantType,
		typename Typed::PointerType,
		typename Typed::ConstPointerType,
		typename Typed::ReferenceType,
		typename Typed::ConstReferenceType
	;

	using
		typename Indexed::IndexType,
		typename Indexed::SizeType
	;

	using
		typename SelfIdentified::SelfType
	;

	using typename Ordered::OrderType;

	using STDForwardIterator	= PointerType;
	using STDReverseIterator	= std::reverse_iterator<PointerType>;

	using STDConstForwardIterator	= ConstPointerType;
	using STDConstReverseIterator	= std::reverse_iterator<ConstPointerType>;

	/// @brief Empty constructor.
	constexpr Iterator() {}

	/// @brief Copy constructor.
	/// @param value Pointer to iterate over.
	constexpr explicit(REVERSE) Iterator(PointerType const& value): iterand(value)			{}
	/// @brief Move constructor.
	/// @param value Pointer to iterate over.
	constexpr explicit(REVERSE) Iterator(PointerType&& value): iterand(CTL::move(value))	{}

	/// @brief Copy constructor.
	/// @param other `Iterator` to copy from.
	constexpr Iterator(SelfType const& other): iterand(other.iterand)		{}
	/// @brief Move constructor.
	/// @param other `Iterator` to move from.
	constexpr Iterator(SelfType&& other): iterand(CTL::move(other.iterand))	{}

	/// @brief Returns the underlying pointer.
	/// @return Underlying pointer.
	constexpr PointerType raw() const {return iterand;}

	/// @brief Dereferences the operator.
	/// @return Reference to underlying object.
	constexpr ReferenceType operator*()	{return *iter();	}
	/// @brief Dereferences the operator.
	/// @return Pointer to underlying object.
	constexpr PointerType operator->()	{return iter();		}

	/// @brief Dereferences the operator.
	/// @return Const reference to underlying object.
	constexpr ConstReferenceType operator*() const	{return *iter();	}
	/// @brief Dereferences the operator.
	/// @return Pointer to underlying constant.
	constexpr ConstPointerType operator->() const	{return iter();		}

	/// @brief Pre-increments the operator.
	/// @return Reference to self.
	SelfType& operator++()		{step(); return *this;							}
	/// @brief Post-increments the operator.
	/// @return Copy of self pre-increment.
	SelfType operator++(int)	{SelfType copy = *this; step(); return copy;	}

	/// @brief Pre-decrements the operator.
	/// @return Reference to self.
	SelfType& operator--()		{rstep(); return *this;							}
	/// @brief Post-decrements the operator.
	/// @return Copy of self pre-decrement.
	SelfType operator--(int)	{SelfType copy = *this; rstep(); return copy;	}

	/// @brief Pointer-to-Object type conversion.
	constexpr operator PointerType() const		{return iter();	}

	/// @brief Equality operator.
	/// @param other Other `Iterator` to compare with.
	/// @return Whether they're equal.
	constexpr bool operator==(SelfType const& other) const			{return iterand == other.iterand;	}
	/// @brief Threeway comparison operator.
	/// @param other Other `Iterator` to compare with.
	/// @return Order between both `Iterator`s.
	constexpr OrderType operator<=>(SelfType const& other) const	{return compare(other.iterand);		}

	/// @brief Returns the difference from the `Iterator` with another.
	/// @param other Other `Iterator` to subtract with.
	/// @return Difference between both operators.
	constexpr IndexType operator-(SelfType const& other) const	{return difference(other.iterand);	}
	/// @brief Subtracts an offset from the `Iterator`.
	/// @param value The offset to subtract from.
	/// @return Offset Resulting offset `Iterator`.
	constexpr SelfType operator-(IndexType const& value) const	{return offset(-value);				}
	/// @brief Adds an offset to the `Iterator`.
	/// @param value The offset to add.
	/// @return Offset Resulting offset `Iterator`.
	constexpr SelfType operator+(IndexType const& value) const	{return offset(value);				}

	/// @brief `std::reverse_iterator` type conversion.
	constexpr operator STDReverseIterator() requires(REVERSE)				{return iterand;}
	/// @brief `std::reverse_iterator` type conversion.
	constexpr operator STDConstReverseIterator() const requires(REVERSE)	{return iterand;}
private:
	constexpr PointerType iter() const {
		PointerType it = iterand;
		if constexpr (REVERSE)	return it-1;
		else					return it;
	}

	constexpr void step() {
		if constexpr (REVERSE)	--iterand;
		else					++iterand;
	}

	constexpr void rstep() {
		if constexpr (REVERSE)	++iterand;
		else					--iterand;
	}

	constexpr OrderType compare(ConstPointerType const& other) const {
		if constexpr (REVERSE)	return other <=> iterand;
		else					return iterand <=> other;
	}

	constexpr IndexType difference(ConstPointerType const& other) const {
		if constexpr (REVERSE)	return other - iterand;
		else					return iterand - other;
	}

	constexpr ConstPointerType offset(IndexType const& value) const {
		if constexpr (REVERSE)	return iterand + value;
		else					return iterand - value;
	}

	constexpr PointerType offset(IndexType const& value) {
		if constexpr (REVERSE)	return iterand + value;
		else					return iterand - value;
	}

	/// @brief The underlying iterand.
	PointerType iterand = nullptr;
};

/// @brief `Iterator` analog for a forward iterator.
/// @tparam TData Type of object the iterator is pointing to.
/// @tparam TIndex Index type.
template<class TData, Type::Integer TIndex = usize>
using ForwardIterator = Iterator<TData, false, TIndex>;
/// @brief `Iterator` analog for a reverse iterator.
/// @tparam TData Type of object the iterator is pointing to.
/// @tparam TIndex Index type.
template<class TData, Type::Integer TIndex = usize>
using ReverseIterator = Iterator<TData, true, TIndex>;

/// @brief Container-specific type constraints.
namespace Type::Container {
	/// @brief Type must be a valid iterator type.
	template<class T>
	concept Iterator = requires {
		typename T::DataType;
		typename T::SizeType;
	} && (
		Derived<T, ::CTL::Iterator<typename T::DataType, true, typename T::SizeType>>
	||	Derived<T, ::CTL::Iterator<typename T::DataType, false, typename T::SizeType>>
	);

	/// @brief Type must have both `begin()` and `end()` functions that returns either `TIterator` or `TConstIterator`.
	template<class T, class TIterator, class TIteratorConst>
	concept Ranged = requires (T t) {
		{t.begin()} -> Type::OneOf<TIterator, TIteratorConst>;
		{t.end()} -> Type::OneOf<TIterator, TIteratorConst>;
	};

	/// @brief Type must have both `data()` and `size()` functions, that return `TPointer` and `TSize`, respectively.
	template<class T, class TPointer, class TSize>
	concept Bounded = requires (T t) {
		{t.data()} -> Type::EqualOrConst<TPointer>;
		{t.size()} -> Type::Equal<TSize>;
	};

	/**
	 * @brief Type must fullfill the following requirements:
	 * 
	 * 1. Have both `data()` and `size()` functions.
	 * 
	 * 2. The result of dereferencing the result of `data()` must be convertible to `TData`.
	 * 
	 * 3. The result of `size()` must be convertible to `TSize`.
	*/
	template<class T, class TData, class TSize>
	concept FuzzilyBounded = requires (T t) {
		{*t.data()} -> Type::Convertible<TData>;
		{t.size()} -> Type::Convertible<TSize>;
	};
}

static_assert(Type::Container::Iterator<Iterator<int>>);

/// @brief Tags the deriving class as iteratable.
/// @tparam TData Element type.
/// @tparam TIndex Index type.
template<class TData, Type::Integer TIndex>
struct Iteratable: Typed<TData>, Indexed<TIndex> {
	using Indexed	= ::CTL::Indexed<TIndex>;
	using Typed		= ::CTL::Typed<TData>;

	using
		typename Typed::DataType,
		typename Typed::ConstantType,
		typename Typed::PointerType,
		typename Typed::ConstPointerType
	;

	using
		typename Indexed::IndexType,
		typename Indexed::SizeType
	;

	using Indexed::MAX_SIZE;

	/// @brief Forward iterator type.
	typedef ForwardIterator<DataType, SizeType>		IteratorType;
	/// @brief Constant forward iterator type.
	typedef ForwardIterator<ConstantType, SizeType>	ConstIteratorType;
	/// @brief Reverse iterator type.
	typedef ReverseIterator<DataType, SizeType>		ReverseIteratorType;
	/// @brief Constant reverse iterator type.
	typedef ReverseIterator<ConstantType, SizeType>	ConstReverseIteratorType;

protected:
	/// @brief Ensures a given index, when negative, is between the bounds of the iteratable class.
	/// @param index Index to wrap.
	/// @param count Size of the iteratable range.
	constexpr static void wrapBounds(IndexType& index, SizeType const& size) {
		while (index < 0) index += size;
	}

	/// @brief Throws an OutOfBoundsException when called.
	/// @throw OutOfBoundsException when called.
	[[noreturn]] constexpr static void outOfBoundsError() {
		throw OutOfBoundsException("Index is bigger than iteratable size!");
	}
};

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_ITERATOR_H
