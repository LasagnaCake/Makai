#ifndef CTL_ALGORITHM_SEARCH_H
#define CTL_ALGORITHM_SEARCH_H

#include "sort.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Search-specific type constraints.
namespace Type::Algorithm::Search {
	/// @brief Type must be a valid comparator that supports equality comparison on `TData`.
	template<class TCompare, class TData>
	concept EqualityComparator = requires (TData a, TData b) {
		{TCompare::equals(a, b)} -> Type::Equal<bool>;
	};

	/// @brief Type must be a valid comparator that supports threeway comparison on `TData`.
	template<class TCompare, class TData>
	concept ThreewayComparator = requires (TData a, TData b) {
		{TCompare::compare(a, b)} -> Type::Equal<ValueOrder>;
	};

	/// @brief Type must be a valid comparator that supports both threeway and equality comparison on `TData`.
	template<class TCompare, class TData>
	concept FullHouseComparator = 
		EqualityComparator<TCompare, TData>
	&&	ThreewayComparator<TCompare, TData>
	;
}

/// @brief Forward searches through a range of elements.
/// @tparam T Iterator type.
/// @tparam TIndex Index type.
/// @tparam TData Element type.
/// @tparam TCompare Comparator type.
/// @param begin Iterator to beginning of range.
/// @param end Iterator to end of range.
/// @param value Value to search for.
/// @return Index of value, or `-1` if not found.
template<
	Type::Container::Iterator T,
	Type::Signed TIndex = ssize,
	class TData = typename T::DataType,
	Type::Algorithm::Search::EqualityComparator<TData> TCompare = SimpleComparator<TData>
>
constexpr TIndex fsearch(T begin, T const& end, TData const& value) {
	if (end <= begin) return -1;
	while (begin < end) {
		if (TCompare::equals(*begin, value))
			return end-begin;
		++begin;
	}
	return -1;
}

/// @brief Reverse searches through a range of elements.
/// @tparam T Iterator type.
/// @tparam TIndex Index type.
/// @tparam TData Element type.
/// @tparam TCompare Comparator type.
/// @param begin Iterator to beginning of range.
/// @param end Iterator to end of range.
/// @param value Value to search for.
/// @return Index of value, or `-1` if not found.
template<
	Type::Container::Iterator T,
	Type::Signed TIndex = ssize,
	class TData = typename T::DataType,
	Type::Algorithm::Search::EqualityComparator<TData> TCompare = SimpleComparator<TData>
>
constexpr TIndex rsearch(T const& begin, T end, TData const& value) {
	if (end <= begin) return -1;
	while (--end >= begin)
		if (TCompare::equals(*end, value))
			return end-begin;
	return -1;
}

/// @brief Performs a binary search through a range of elements.
/// @tparam T Iterator type.
/// @tparam TIndex Index type.
/// @tparam TData Element type.
/// @tparam TCompare Comparator type.
/// @param begin Iterator to beginning of range.
/// @param end Iterator to end of range.
/// @param value Value to search for.
/// @return Index of value, or `-1` if not found.
template<
	Type::Container::Iterator T,
	Type::Signed TIndex = ssize,
	class TData = typename T::DataType,
	Type::Algorithm::Search::FullHouseComparator<TData> TCompare = SimpleComparator<TData>
>
constexpr TIndex bsearch(T const& begin, T const& end, TData const& value) {
	if (end <= begin) return -1;
	auto const size = (end - begin);
	if (TCompare::equals(*begin, value)) return 0;
	if (TCompare::equals(*(end-1), value)) return size - 1;
	TIndex lo = 0, hi = size - 1, i = -1;
	while (hi >= lo) {
		i = lo + (hi - lo) / 2;
		switch(TCompare::compare(value, *(begin + i))) {
			case StandardOrder::LESS:		hi = i-1; break;
			case StandardOrder::EQUAL:		return i;
			case StandardOrder::GREATER:	lo = i+1; break;
			default:
			case StandardOrder::UNORDERED:	return -1;
		}
	}
	return -1;
}

/// @brief Nearest match search algorithms.
namespace Nearest {
	/// @brief Search result.
	/// @tparam T Index type.
	template <Type::Signed T>
	struct Match {
		/// @brief Nearest lowest match.
		T lowest;
		/// @brief Nearest highest match.
		T highest;
	};

	/// @brief Performs a forward search through a range of elements, and returns the closest to the value.
	/// @tparam T Iterator type.
	/// @tparam TIndex Index type.
	/// @tparam TData Element type.
	/// @tparam TCompare Comparator type.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator to end of range.
	/// @param value Value to search for.
	/// @return Index of matches, or the nearest. Returns `-1` for both if range is empty or invalid.
	template<
		Type::Container::Iterator T,
		Type::Signed TIndex = ssize,
		class TData = typename T::DataType,
		Type::Algorithm::Search::ThreewayComparator<TData> TCompare = SimpleComparator<TData>
	>
	constexpr Match<TIndex> fsearch(T begin, T const& end, TData const& value) {
		if (end <= begin) return {-1, -1};
		auto const size = (end - begin);
		Match<TIndex> match = {0, 0};
		for (decltype(size) i = 0; i < size; ++i) {
			auto const current	= (begin + i);
			auto const lo		= (begin + match.lowest);
			auto const hi		= (begin + match.highest);
			if (
				TCompare::compare(*current, *lo)	== StandardOrder::GREATER
			&&	TCompare::compare(*current, value)	!= StandardOrder::GREATER
			) match.lowest = i;
			if (
				TCompare::compare(*current, *hi)	== StandardOrder::LESS
			&&	TCompare::compare(*current, value)	!= StandardOrder::LESS
			) match.highest = i;
			++begin;
		}
		return match;
	}

	/// @brief Performs a reverse search through a range of elements, and returns the closest to the value.
	/// @tparam T Iterator type.
	/// @tparam TIndex Index type.
	/// @tparam TData Element type.
	/// @tparam TCompare Comparator type.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator to end of range.
	/// @param value Value to search for.
	/// @return Index of matches, or the nearest. Returns `-1` for both if range is empty or invalid.
	template<
		Type::Container::Iterator T,
		Type::Signed TIndex = ssize,
		class TData = typename T::DataType,
		Type::Algorithm::Search::ThreewayComparator<TData> TCompare = SimpleComparator<TData>
	>
	constexpr Match<TIndex> rsearch(T begin, T const& end, TData const& value) {
		if (end <= begin) return {-1, -1};
		auto const size = (end - begin);
		Match<TIndex> match = {0, 0};
		for (decltype(size) i = size-1; i >= 0; --i) {
			auto const current	= (begin + i);
			auto const lo		= (begin + match.lowest);
			auto const hi		= (begin + match.highest);
			if (
				TCompare::compare(*current, *lo)	== StandardOrder::GREATER
			&&	TCompare::compare(*current, value)	!= StandardOrder::GREATER
			) match.lowest = i;
			if (
				TCompare::compare(*current, *hi)	== StandardOrder::LESS
			&&	TCompare::compare(*current, value)	!= StandardOrder::LESS
			) match.highest = i;
			++begin;
		}
		return match;
	}


	/// @brief Performs a binary search through a range of elements, and returns the closest to the value.
	/// @tparam T Iterator type.
	/// @tparam TIndex Index type.
	/// @tparam TData Element type.
	/// @tparam TCompare Comparator type.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator to end of range.
	/// @param value Value to search for.
	/// @return
	///		Index of matches, or the nearest. Returns `-1` for both if
	///		range is empty or invalid, or an invalid comparison happened.
	template<
		Type::Container::Iterator T,
		Type::Signed TIndex = ssize,
		class TData = typename T::DataType,
		Type::Algorithm::Search::FullHouseComparator<TData> TCompare = SimpleComparator<TData>
	>
	constexpr Match<TIndex> bsearch(T const& begin, T const& end, TData const& value) {
		if (end <= begin) return {-1, -1};
		auto const size = (end - begin);
		if (TCompare::equals(*begin, value)) return {0, 0};
		if (TCompare::equals(*(end-1), value)) return {size - 1, size - 1};
		TIndex lo = 0, hi = size - 1, i = -1;
		while (hi >= lo) {
			i = lo + (hi - lo) / 2;
			switch(TCompare::compare(value, *(begin + i))) {
				case StandardOrder::LESS:		hi = i-1; break;
				case StandardOrder::EQUAL:		return {i, i};
				case StandardOrder::GREATER:	lo = i+1; break;
				default:						[[unlikely]]
				case StandardOrder::UNORDERED:	[[unlikely]]
												return {-1, -1};
			}
		}
		return {lo, hi};
	}
}

CTL_NAMESPACE_END

#endif