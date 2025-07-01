#ifndef CTL_RANGE_SEQUENCE_H
#define CTL_RANGE_SEQUENCE_H

#include "../namespace.hpp"
#include "../typetraits/traits.hpp"
#include "../cpperror.hpp"
#include "iterate.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Range-lated facilites.
namespace Range {
	/// @brief Implementation details.
	namespace Impl {
		/// @brief Sequence iterator.
		/// @tparam T Number type.
		template<Type::Number T>
		struct SequenceIterator {
			/// @brief Number type.
			using DataType			= T;
			/// @brief Number difference type.
			using DifferenceType	= Meta::DualType<Type::Integer<DataType>, AsSigned<DataType>, DataType>;

			/// @brief Constructs the iterator with a starting value, and a stride.
			/// @param value Starting value.
			/// @param stride Stride between steps.
			constexpr SequenceIterator(DataType const value, DifferenceType const& stride):
				value(value),
				stride(stride),
				decrementing(stride < 0) {
					if (!stride) throw InvalidValueException("Step cannot be zero!");
				}

			/// @brief Dereference operator overloading.
			constexpr DataType operator*() const			{return value;					}
			/// @brief Pre-increment operator overloading.
			/// @return Reference to self.
			constexpr SequenceIterator& operator++() const	{value += stride; return *this;	}
			/// @brief Pre-decrement operator overloading.
			/// @return Reference to self.
			constexpr SequenceIterator& operator--() const	{value -= stride; return *this;	}

			/// @brief Returns whether one iterator has overshot another iterator.
			/// @return Whether overshot happened.
			constexpr bool operator!=(SequenceIterator const& other) const {
				return decrementing
				?	(value > other.value || (value - stride) > other.value)
				:	(value < other.value || (value - stride) < other.value)
				;
			}

		private:
			/// @brief Current number.
			DataType				value;
			/// @brief Stride between steps.
			DifferenceType const	stride;
			/// @brief Whether iterator is incrementing or decrementing.
			bool const				decrementing;
		};

		/// @brief Wrapper for `IteratorSequence`.
		/// @tparam T Number type.
		template<Type::Number T>
		using SequenceWrapper = IteratorWrapper<SequenceIterator<T>>;
	}

	/// @brief returns a numeric sequence from `begin` to `end` with steps of `step`.
	/// @tparam T Number type.
	/// @param begin Starting value.
	/// @param end End value.
	/// @param step Stride between steps.
	/// @return Sequence.
	template<Type::Number T>
	constexpr Impl::SequenceWrapper<T> sequence(T const begin, T const end, T const step) {
		if (!step) throw InvalidValueException("Step cannot be zero!");
		return {{begin, step}, {end, step}};
	}

	/// @brief returns a numeric sequence from `begin` to `end`.
	/// @tparam T Number type.
	/// @param begin Starting value.
	/// @param end End value.
	/// @return Sequence.
	template<Type::Number T>
	constexpr Impl::SequenceWrapper<T> sequence(T const begin, T const end) {
		if (begin < end)
			return sequence<T>(begin, end, -1);
		return sequence<T>(begin, end, 1);
	}

	/// @brief returns a numeric sequence from `0` to `end`.
	/// @tparam T Number type.
	/// @param end End value.
	/// @return Sequence.
	template<Type::Number T>
	constexpr Impl::SequenceWrapper<T> sequence(T const end) {
		return sequence<T>(0, end);
	}
}

/// @brief returns a numeric sequence from `begin` to `end` with steps of `step`.
/// @tparam T Number type.
/// @param begin Starting value.
/// @param end End value.
/// @param step Stride between steps.
/// @return Sequence.
template<Type::Number T>
constexpr auto range(T const begin, T const end, T const step) {
	return Range::sequence<T>(begin, end, step);
}

/// @brief returns a numeric sequence from `begin` to `end`.
/// @tparam T Number type.
/// @param begin Starting value.
/// @param end End value.
/// @return Sequence.
template<Type::Number T>
constexpr auto range(T const begin, T const end) {
	return Range::sequence<T>(begin, end);
}

/// @brief returns a numeric sequence from `0` to `end`.
/// @tparam T Number type.
/// @param end End value.
/// @return Sequence.
template<Type::Number T>
constexpr auto range(T const end) {
	return Range::sequence<T>(0, end);
}

CTL_NAMESPACE_END

#endif