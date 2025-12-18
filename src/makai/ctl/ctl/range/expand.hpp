#ifndef CTL_RANGE_EXPAND_H
#define CTL_RANGE_EXPAND_H

#include "../namespace.hpp"
#include "../typetraits/traits.hpp"
#include "iterate.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Range-related facilites.
namespace Range {
	/// @brief Implementation details.
	namespace Impl {
		/// @brief Expansion iterator.
		/// @tparam T Underlying iterator type.
		template <class T>
		struct ExpansionIterator {
			/// @brief Expansion.
			struct Expansion {
				/// @brief Value.
				AsIteratorValue<T>	value;
				/// @brief Index.
				usize				index;
			};

			/// @brief Constructs the expansion iterator.
			/// @param iter Underlying reference iterator.
			constexpr ExpansionIterator(T const& iter): iter(iter), current(0) {}

			/// @brief Pre-increment operator overloading.
			constexpr ExpansionIterator& operator++()						{++iter; ++current; return *this;	}
			/// @brief Dereference operator overloading.
			constexpr Expansion operator*() const							{return {*iter, current};			}
			/// @brief Equality operator overloading.
			constexpr bool operator==(ExpansionIterator const& other) const	{return iter == other.iter;			}
			/// @brief Inequality operator overloading.
			constexpr bool operator!=(ExpansionIterator const& other) const	{return iter != other.iter;			}

		private:
			/// @brief Start of range.
			T iter;
			/// @brief Current index.
			usize current = 0;
		};

		/// @brief Expansion iterator wrapper alias.
		/// @tparam T Iterator type.
		template <class T>
		using ExpansionWrapper = IteratorWrapper<ExpansionIterator<T>>;
	}

	/// @brief Expands a pair of iterators with additional iteration information.
	/// @tparam T Iterator type.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator to end of range.
	/// @return Expanded iterators.
	template<class T>
	constexpr Impl::ExpansionWrapper<T> expand(T const& begin, T const& end) {
		return iterate<Impl::ExpansionIterator<T>>(begin, end);
	}

	/// @brief Expands a range with additional iteration information.
	/// @tparam T Iterator type.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator to end of range.
	/// @return Expanded iterators.
	template<Type::Iteratable T>
	constexpr auto expand(T& range) {
		return expand(range.begin(), range.end());
	}
	/// @brief Expands a range with additional iteration information.
	/// @tparam T Iterator type.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator to end of range.
	/// @return Expanded iterators.
	template<Type::CIteratable T>
	constexpr auto expand(T& range)
	requires (!Type::Iteratable<T>) {
		return expand(range.data(), range.data() + range.size());
	}
}

CTL_NAMESPACE_END

#endif