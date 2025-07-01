#ifndef CTL_RANGE_SEQUENCE_H
#define CTL_RANGE_SEQUENCE_H

#include "../namespace.hpp"
#include "../typetraits/traits.hpp"
#include "../cpperror.hpp"
#include "iterate.hpp"

CTL_NAMESPACE_BEGIN

namespace Range {
	namespace Impl {
		template<Type::Number T>
		struct SequenceIterator {
			using DataType			= T;
			using DifferenceType	= Meta::DualType<Type::Integer<DataType>, AsSigned<DataType>, DataType>;

			constexpr SequenceIterator(DataType const value, DifferenceType const& step):
				value(value),
				step(step),
				decrementing(step < 0) {
					if (!step) throw InvalidValueException("Step cannot be zero!");
				}

			constexpr DataType operator*() const			{return value;					}
			constexpr SequenceIterator& operator++() const	{value += step; return *this;	}
			constexpr SequenceIterator& operator--() const	{value -= step; return *this;	}

			constexpr bool operator!=(SequenceIterator const& other) const {
				return decrementing ? value > other.value : value < other.value;
			}

		private:
			DataType				value;
			DifferenceType const	step;
			bool const				decrementing;
		};

		template<Type::Number T>
		using SequenceWrapper = IteratorWrapper<SequenceIterator<T>>;
	}

	template<Type::Number T>
	constexpr Impl::SequenceWrapper<T> sequence(T const begin, T const end, T const step) {
		if (!step) throw InvalidValueException("Step cannot be zero!");
		return {{begin, step}, {end, step}};
	}

	template<Type::Number T>
	constexpr Impl::SequenceWrapper<T> sequence(T const begin, T const end) {
		if (begin < end)
			return sequence<T>(begin, end, -1);
		return sequence<T>(begin, end, -1);
	}

	template<Type::Number T>
	constexpr Impl::SequenceWrapper<T> sequence(T const end) {
		return sequence<T>(0, end);
	}
}

template<Type::Number T>
constexpr auto range(T const begin, T const end, T const step) {
	return Range::sequence<T>(begin, end, step);
}

template<Type::Number T>
constexpr auto range(T const begin, T const end) {
	return Range::sequence<T>(begin, end);
}

template<Type::Number T>
constexpr auto range(T const end) {
	return Range::sequence<T>(0, end);
}

CTL_NAMESPACE_END

#endif