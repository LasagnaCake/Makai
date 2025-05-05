#ifndef CTL_CONTAINER_ID_SSUID_H
#define CTL_CONTAINER_ID_SSUID_H

#include "../../namespace.hpp"
#include "../../templates.hpp"
#include "../../order.hpp"
#include "../../memory/core.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Identifiers.
namespace ID {

/// @brief Static Size Unique IDentifier.
/// @tparam N Identifier size.
template <usize N>
struct SSUID: Ordered, SelfIdentified<SSUID<N>> {
	using Ordered			= ::CTL::Ordered;
	using SelfIdentified	= ::CTL::SelfIdentified<SSUID<N>>;
	
	using typename Ordered::OrderType;

	using typename SelfIdentified::SelfType;

	/// @brief Identifier size.
	constexpr static usize SIZE = N;
	/// @brief Identifier storage container type.
	using InternalType = As<usize[SIZE]>;

	/// @brief Addition operator overloading.
	/// @param other `SSUID` to add.
	/// @return Result of the operation.
	constexpr SelfType	operator+(SelfType const& other) const	{return SelfType(*this).increment(other);	}
	/// @brief Addition operator overloading.
	/// @param other `SSUID` to subtract.
	/// @return Result of the operation.
	constexpr SelfType	operator-(SelfType const& other) const	{return SelfType(*this).decrement(other);	}
	/// @brief Addition assignment operator overloading.
	/// @param other `SSUID` to add.
	/// @return Reference to self.
	constexpr SelfType&	operator+=(SelfType const& other)		{return increment(other);					}
	/// @brief Addition assignment operator overloading.
	/// @param other `SSUID` to subtract.
	/// @return Reference to self.
	constexpr SelfType&	operator-=(SelfType const& other)		{return decrement(other);					}

	/// @brief Postfix increment iterator.
	/// @return Previous value.
	constexpr SelfType	operator++(int)	{SelfType self = *this; increment(); return self;	}
	/// @brief Postfix decrement iterator.
	/// @return Previous value.
	constexpr SelfType	operator--(int)	{SelfType self = *this; decrement(); return self;	}
	/// @brief Prefix increment iterator.
	/// @return Reference to self.
	constexpr SelfType&	operator++()	{increment(); return *this;							}
	/// @brief Prefix decrement iterator.
	/// @return Reference to self.
	constexpr SelfType&	operator--()	{decrement(); return *this;							}

	/// @brief Returns whether the `SSUID` is zero.
	constexpr operator bool() const	{return equals({});}

	/// @brief Equality comparison operator.
	/// @param other `SSUID` to compare to.
	/// @return Whether both `SSUID`s are equal.
	constexpr bool operator==(SelfType const& other) const			{return equals(other);							}
	/// @brief Threeway comparison operator.
	/// @param other `SSUID` to compare to.
	/// @return Order between `SSUID`s.
	constexpr OrderType operator<=>(SelfType const& other) const	{return compare(other);							}

	/// @brief Compares the `SSUID` with another.
	/// @param other `SSUID` to compare to.
	/// @return Whether both `SSUID`s are equal.
	constexpr bool equals(SelfType const& other) const				{return MX::memcmp(id, other.id, SIZE) == 0;	}
	/// @brief Gets the order between the `SSUID` with another.
	/// @param other `SSUID` to compare to.
	/// @return Order between `SSUID`s.
	constexpr OrderType compare(SelfType const& other) const		{return MX::memcmp(id, other.id, SIZE);			}

private:
	constexpr SelfType& increment(SelfType const& other) {
		for (usize i = 0; i < SIZE; ++i)
			increment(other.id[i], i);
		return *this;
	}

	constexpr SelfType& decrement(SelfType const& other) {
		for (usize i = 0; i < SIZE; ++i)
			decrement(other.id[i], i);
		return *this;
	}

	constexpr SelfType& increment(usize amount, usize const start = 0) {
		usize index = start;
		if (id[start] > (static_cast<usize>(-1) - amount))
			while (index < SIZE && (id[index++] == static_cast<usize>(-1)))
				++id[index];
		id[start] += amount;
		return *this;
	}

	constexpr SelfType& decrement(usize const amount, usize const start = 0) {
		usize index = start;
		if (id[start] < amount)
			while (index < SIZE && (id[index++] == 0))
				--id[index];
		id[start] -= amount;
		return *this;
	}

	constexpr SelfType& increment() {
		usize index = 0;
		while (index < SIZE && id[index++] == static_cast<usize>(-1))
			++id[index];
		++id[0];
		return *this;
	}

	constexpr SelfType& decrement() {
		usize index = 0;
		while (index < SIZE && id[index++] == 0)
			--id[index];
		--id[0];
		return *this;
	}

	/// @brief Underlying ID container type.
	InternalType id = {0};
};

/// @brief Large Unique IDentifier.
using LUID	= SSUID<2>;
/// @brief Very Large Unique IDentifier.
using VLUID	= SSUID<4>;
/// @brief Extremely Large Unique IDentifier.
using ELUID	= SSUID<8>;

}

CTL_NAMESPACE_END

#endif