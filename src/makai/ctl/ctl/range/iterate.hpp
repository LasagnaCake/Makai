#ifndef CTL_RANGE_ITERATE_H
#define CTL_RANGE_ITERATE_H

#include "../namespace.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Range-related facilites.
namespace Range {
	/// @brief Implementation details.
	namespace Impl {
		/// @brief Iterator wrapper.
		/// @tparam T Iterator type.
		template <class T>
		struct IteratorWrapper {
			/// @brief Constructs the iterator wrapper.
			/// @param begin Iterator to beginning of range.
			/// @param end Iterator to end of range.
			constexpr IteratorWrapper(T const& begin, T const& end): from(begin), to(end) {}
			/// @brief Returns the iterator to the beginning of the range.
			/// @return Iterator to beginning of range.
			constexpr T begin() const	{return from;	}
			/// @brief Returns the iterator to the end of the range.
			/// @return Iterator to beginning of range.
			constexpr T end() const		{return to;		}
		private:
			/// @brief Start of range.
			T const from;
			/// @brief End of range.
			T const to;
		};
	}

	/// @brief Wraps an iterator pair for easy range-based for loop iteration.
	/// @tparam T Iterator type.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator to end of range.
	/// @return Wrapped iterators.
	template<class T>
	constexpr Impl::IteratorWrapper<T> iterate(T const& begin, T const& end) {
		return {begin, end};
	}	
}

CTL_NAMESPACE_END

#endif