#ifndef CTL_CONTAINER_ARRAY_H
#define CTL_CONTAINER_ARRAY_H

#include "../templates.hpp"
#include "../typeinfo.hpp"
#include "../ctypes.hpp"
#include "../cpperror.hpp"
#include "../typetraits/traits.hpp"
#include "iterator.hpp"
#include "arguments.hpp"
#include "../algorithm/sort.hpp"
#include "../algorithm/reverse.hpp"
#include "../memory/memory.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Static array of objects.
/// @tparam TData Element type.
/// @tparam N Array size.
/// @tparam TIndex Index type.
template<class TData, usize N, Type::Integer TIndex = usize>
struct Array:
	Iteratable<TData, TIndex>,
	SelfIdentified<Array<TData, N, TIndex>>,
	Ordered {
private:
public:
	using Iteratable		= ::CTL::Iteratable<TData, TIndex>;
	using SelfIdentified	= ::CTL::SelfIdentified<Array<TData, N, TIndex>>;

	using
		typename Iteratable::IteratorType,
		typename Iteratable::ReverseIteratorType,
		typename Iteratable::ConstIteratorType,
		typename Iteratable::ConstReverseIteratorType
	;

	using
		typename Iteratable::DataType,
		typename Iteratable::ConstantType,
		typename Iteratable::PointerType,
		typename Iteratable::ConstPointerType,
		typename Iteratable::ReferenceType,
		typename Iteratable::ConstReferenceType
	;

	using 
		typename Iteratable::SizeType,
		typename Iteratable::IndexType
	;
	
	using 
		typename SelfIdentified::SelfType
	;

	using ComparatorType = SimpleComparator<DataType>;

	using Iteratable::MAX_SIZE;

	/// @brief `Array` size.
	constexpr static SizeType SIZE = N;

	/// @brief Underlying array type.
	typedef Decay::AsType<DataType[SIZE]> ArrayType;

	static_assert(N <= MAX_SIZE, "Array size must not be bigger than highest SizeType!");

	/// @brief Empty constructor.
	constexpr Array()	{}
	/// @brief Fixed array constructor.
	/// @param arr Array to initialize with.
	template<usize AS>
	constexpr Array(Decay::AsType<DataType[AS]> const& arr)
	requires (AS == SIZE)											{copy(arr, contents, SIZE);				}
	/// @brief Copy constructor.
	/// @brief v `Array` to copy from.
	constexpr Array(SelfType const& other)							{copy(other.contents, contents, SIZE);	}
	/// @brief Move constructor.
	/// @brief v `Array` to move.
	constexpr Array(SelfType&& other)								{copy(other.contents, contents, SIZE);	}
	
	/// @brief Fill constructor.
	/// @brief v Value to fill with.
	constexpr explicit  Array(ConstReferenceType v): contents{v}	{}

	/// @brief Argument list constructor.
	/// @tparam ...Args Argument types.
	/// @param ...args Values to construct from.
	template<typename... Args>
	constexpr Array(Args const&... rest)
	requires (sizeof...(Args) == N): contents{DataType(rest)...} {}

	/// @brief Returns the element at the given index.
	/// @param index Index of the element.
	/// @return Reference to the element.
	/// @throw OutOfBoundsException when index is bigger than `Array` size.
	constexpr ReferenceType operator[](IndexType index) {
		if (index > IndexType(SIZE-1) || SIZE == 0)
			throw OutOfBoundsException("Index is bigger than array size!");
		wrapBounds(index, SIZE);
		return contents[index];
	}

	/// @brief Returns the element at the given index.
	/// @param index Index of the element.
	/// @return Const reference to the element.
	/// @throw OutOfBoundsException when index is bigger than `Array` size.
	constexpr ConstReferenceType operator[](IndexType index) const {
		if (index > IndexType(SIZE-1) || SIZE == 0)
			throw OutOfBoundsException("Index is bigger than array size!");
		wrapBounds(index, SIZE);
		return contents[index];
	}

	/// @brief Equality operator.
	/// @param other Other `Array` to compare with.
	/// @return Whether they're equal.
	/// @note Requires element type to be equally comparable.
	/// @sa Comparator::equals()
	constexpr bool operator==(SelfType const& other) const
	requires Type::Comparator::Equals<DataType, DataType> {
		return equals(other);
	}

	/// @brief Threeway comparison operator.
	/// @param other Other `Array` to compare with.
	/// @return Order between both `Array`s.
	/// @note Requires element type to be threeway comparable.
	/// @sa Comparator::compare()
	constexpr OrderType operator<=>(SelfType const& other) const
	requires Type::Comparator::Threeway<DataType, DataType> {
		return compare(other);
	}

	/// @brief Returns whether it is equal to another `Array`.
	/// @param other Other `Array` to compare with.
	/// @return Whether they're equal.
	/// @note Requires element type to be equally comparable.
	/// @sa Comparator::equals()
	constexpr SizeType equals(SelfType const& other) const
	requires Type::Comparator::Equals<DataType, DataType> {
		bool result = true;
		for (SizeType i = 0; i < SIZE; ++i)
			result = ComparatorType::equals(contents[i], other.contents[i]);
		return result;
	}

	/// @brief Returns the result of a threeway comparison with another `Array`.
	/// @param other Other `Array` to compare with.
	/// @return Order between both `Array`s.
	/// @note Requires element type to be threeway comparable.
	/// @sa Comparator::compare()
	constexpr OrderType compare(SelfType const& other) const
	requires Type::Comparator::Threeway<DataType, DataType> {
		OrderType result = Order::EQUAL;
		for (SizeType i = 0; i < SIZE; ++i) {
			result = ComparatorType::equals(contents[i], other.contents[i]);
			if (result != Order::EQUAL) break;
		}
		return result;
	}

	/// @brief Fixed array assignment operator.
	/// @param arr Array to assign.
	/// @return Reference to self.
	template<usize AS>
	constexpr Array& operator=(Decay::AsType<DataType[AS]> const& arr)
	requires (AS == SIZE)								{copy(arr, contents, SIZE); return *this;				}
	/// @brief Copy assignment operator.
	/// @brief v `Array` to copy from.
	/// @return Reference to self.
	constexpr Array& operator=(SelfType const& other)	{copy(other.contents, contents, SIZE); return *this;	}
	/// @brief Move assignment operator.
	/// @brief v `Array` to move.
	/// @return Reference to self.
	constexpr Array& operator=(SelfType&& other)		{copy(other.contents, contents, SIZE); return *this;	}
	
	/// @brief Returns the `Array` size.
	/// @return Size of the `Array`.
	constexpr SizeType size() const			{return SIZE;		}
	/// @brief Returns a pointer to the uderlying array.
	/// @return Pointer to the uderlying array.
	constexpr ConstPointerType data() const	{return contents;	}
	/// @brief Returns a pointer to the uderlying array.
	/// @return Pointer to the uderlying array.
	constexpr PointerType data() 			{return contents;	}

	/// @brief Returns an iterator to the beginning of the `Array`.
	/// @return Iterator to the beginning of the `Array`.
	constexpr IteratorType		begin()			{return contents;		}
	/// @brief Returns an iterator to the end of the `Array`.
	/// @return Iterator to the end of the `Array`.
	constexpr IteratorType		end()			{return contents+SIZE;	}
	/// @brief Returns an iterator to the beginning of the `Array`.
	/// @return Iterator to the beginning of the `Array`.
	constexpr ConstIteratorType	begin() const	{return contents;		}
	/// @brief Returns an iterator to the end of the `Array`.
	/// @return Iterator to the end of the `Array`.
	constexpr ConstIteratorType	end() const		{return contents+SIZE;	}

	/// @brief Returns a reverse iterator to the beginning of the `Array`.
	/// @return Reverse iterator to the beginning of the `Array`.
	constexpr ReverseIteratorType		rbegin()		{return contents+SIZE;	}
	/// @brief Returns a reverse iterator to the end of the `Array`.
	/// @return Reverse iterator to the end of the `Array`.
	constexpr ReverseIteratorType		rend()			{return contents;		}
	/// @brief Returns a reverse iterator to the beginning of the `Array`.
	/// @return Reverse iterator to the beginning of the `Array`.
	constexpr ConstReverseIteratorType	rbegin() const	{return contents+SIZE;	}
	/// @brief Returns a reverse iterator to the end of the `Array`.
	/// @return Reverse iterator to the end of the `Array`.
	constexpr ConstReverseIteratorType	rend() const	{return contents;		}

	/// @brief Returns a pointer to the beginning of the `Array`.
	/// @return Pointer to the beginning of the `Array`.
	constexpr PointerType		cbegin()		{return contents;		}
	/// @brief Returns a pointer to the end of the `Array`.
	/// @return Pointer to the end of the `Array`.
	constexpr PointerType		cend()			{return contents+SIZE;	}
	/// @brief Returns a pointer to the beginning of the `Array`.
	/// @return Pointer to the beginning of the `Array`.
	constexpr ConstPointerType	cbegin() const	{return contents;		}
	/// @brief Returns a pointer to the end of the `Array`.
	/// @return Pointer to the end of the `Array`.
	constexpr ConstPointerType	cend() const	{return contents+SIZE;	}

	/// @brief Returns the first element.
	/// @return Reference to the first element.
	constexpr ReferenceType			front()			{return contents[0];		}
	/// @brief Returns the last element.
	/// @return Reference to the last element.
	constexpr ReferenceType 		back()			{return contents[SIZE-1];	}
	/// @brief Returns the first element.
	/// @return Const reference to the first element.
	constexpr ConstReferenceType	front() const	{return contents[0];		}
	/// @brief Returns the last element.
	/// @return Const reference to the last element.
	constexpr ConstReferenceType	back() const	{return contents[SIZE-1];	}

private:
	using Iteratable::wrapBounds;

	constexpr static void copy(ConstantType* src, DataType* dst, SizeType count) {
		if constexpr (Type::Standard<DataType>)
			MX::memmove<DataType>(dst, src, count);
		else MX::objcopy<DataType>(dst, src, count);
	}

	ArrayType contents;
};

namespace Impl {
	template<typename T, Type::Integer TIndex = usize>
	struct FromCArray;

	template<usize N, class TData, Type::Integer TIndex>
	struct FromCArray<TData[N], TIndex> {
		typedef Array<TData, N, TIndex> Type;
	};
}

template<typename T, Type::Integer TIndex = usize>
using FromCArray = Impl::FromCArray<T, TIndex>::Type;

namespace {
	template<usize N>
	constexpr bool isValidArray(Array<usize, N> const& arr) {
		return (arr.SIZE == N);
	}
}

static_assert(isValidArray(Array<usize, 10>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_ARRAY_H