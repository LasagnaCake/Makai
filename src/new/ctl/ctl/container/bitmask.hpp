#ifndef CTL_CONTAINER_BITMASK_H
#define CTL_CONTAINER_BITMASK_H

#include "../templates.hpp"
#include "../typetraits/traits.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Bit mask.
/// @tparam TData Underlying storage type.
/// @tparam S Length of the bit mask.
/// @tparam I Initial state.
/// @note Size of the bitmask (in bytes) will be `sizeof(TData) * S`.
template<Type::Integer TData, usize S, bool I = true>
struct BitMask:
	Typed<TData>,
	SelfIdentified<BitMask<TData, S, I>> {
	/// @brief Size of the underlying bit mask.
	constexpr static usize SIZE = S;
	/// @brief Byte size of the underlying bit mask.
	constexpr static usize BYTE_SIZE = sizeof(TData) * SIZE;
	/// @brief Bit size of mask element.
	constexpr static usize ELEMENT_BIT_SIZE = (sizeof(TData) * 8);
	/// @brief Initial state of the underlying bit mask.
	/// @note `true` = all bits set. `false` = all bits unset.
	constexpr static bool INITIAL_STATE = I;

	using Typed				= ::CTL::Typed<TData>;
	using SelfIdentified	= ::CTL::SelfIdentified<BitMask<TData, S, I>>;

	using typename Typed::DataType;
	using typename SelfIdentified::SelfType;

	using MaskType = Decay::AsType<DataType[SIZE]>;

	constexpr static DataType ALL_ENABLED = TypeInfo<DataType>::HIGHEST;

	using OtherType = BitMask<TData, S, !I>;

	/// @brief Empty constructor.
	constexpr BitMask() {}

	/// @brief Constructs a `BitMask` with all bits set to a given state.
	/// @param state Initial state of the mask.
	constexpr explicit BitMask(bool const state) {
		for (usize i = 0; i < SIZE; ++i)
			mask[i] =  (state ? ALL_ENABLED : 0);
	}

	/// @brief Constructs a `BitMask` from a given mask.
	/// @param mask Mask to copy from.
	constexpr BitMask(MaskType const& mask) {
		for (usize i = 0; i < SIZE; ++i)
			this->mask[i] = mask[i];
	}

	/// @brief Copy constructor.
	/// @param other Other `BitMask`.	
	constexpr BitMask(SelfType const& other): BitMask(other.mask)	{}
	/// @brief Copy constructor for `BitMask` with different initial state.
	/// @param other Other `BitMask` with different initial state.
	constexpr BitMask(OtherType const& other): BitMask(other.mask)	{}

	/// @brief Returns all elements AND'd into a single value.
	/// @return All elements AND'd.
	constexpr DataType reduce() const {
		DataType result{1};
		for (usize i = 0; i < SIZE; ++i)
			result &= mask[i];
		return result;
	}

	/// @brief Returns all elements OR'd into a single value.
	/// @return All elements OR'd.
	constexpr DataType overlap() const {
		DataType result{0};
		for (usize i = 0; i < SIZE; ++i)
			result |= mask[i];
		return result;
	}

	/// @brief Returns this `BitMask` `AND`ed with another.
	/// @param other `BitMask` to `AND` with.
	/// @return This `BitMask` `AND`ed with it.
	constexpr SelfType match(SelfType const& other) const {
		SelfType result;
		for (usize i = 0; i < SIZE; ++i)
			result.mask[i] = other.mask[i] & mask[i];
		return result;
	}

	/// @brief Returns this `BitMask` `OR`ed with another.
	/// @param other `BitMask` to `OR` with.
	/// @return This `BitMask` `OR`ed with it.
	constexpr SelfType join(SelfType const& other) const {
		SelfType result;
		for (usize i = 0; i < SIZE; ++i)
			result.mask[i] = other.mask[i] | mask[i];
		return result;
	}

	/// @brief Returns this `BitMask` `XOR`ed with another.
	/// @param other `BitMask` to `XOR` with.
	/// @return This `BitMask` `XOR`ed with it.
	constexpr SelfType exclude(SelfType const& other) const {
		SelfType result;
		for (usize i = 0; i < SIZE; ++i)
			result.mask[i] = other.mask[i] ^ mask[i];
		return result;
	}

	/// @brief Returns the inverse of the `BitMask`.
	/// @return The inverse of the `BitMask`.
	constexpr SelfType inverse() const {
		SelfType result;
		for (usize i = 0; i < SIZE; ++i)
			result.mask[i] = ~mask[i];
		return result;
	}

	/// @brief Returns this `BitMask` `AND`ed with another.
	/// @param other `BitMask` to `AND` with.
	/// @return This `BitMask` `AND`ed with it.
	constexpr SelfType operator&(SelfType const& other) const {return matched(other);	}
	/// @brief Returns this `BitMask` `OR`ed with another.
	/// @param other `BitMask` to `OR` with.
	/// @return This `BitMask` `OR`ed with it.
	constexpr SelfType operator|(SelfType const& other) const {return joined(other);	}
	/// @brief Returns this `BitMask` `XOR`ed with another.
	/// @param other `BitMask` to `XOR` with.
	/// @return This `BitMask` `XOR`ed with it.
	constexpr SelfType operator^(SelfType const& other) const {return excluded(other);	}

	/// @brief Returns the inverse of the `BitMask`.
	/// @return The inverse of the `BitMask`.
	constexpr SelfType operator~() const {return inverse();}

	/// @brief Returns the state of a bit at a given index.
	/// @param index Index of the bit.
	/// @return Whether the bit is set or not.
	/// @throw OutOfBoundsException when index is bigger than max possible bit range.
	constexpr bool operator[](ssize index) {
		if (index >= BYTE_SIZE * 8) throw OutOfBoundsException("Index is bigger than possible bit range!");
		while (index < 0) index += BYTE_SIZE * 8;
		usize i = 0;
		while (index > ELEMENT_BIT_SIZE) {
			index -= ELEMENT_BIT_SIZE;
			++i;
		}
		return (mask[i] >> index) & 1;
	}

	/// @brief Underlying bit mask.
	MaskType mask = {INITIAL_STATE ? ALL_ENABLED : 0};
};

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_BITMASK_H
