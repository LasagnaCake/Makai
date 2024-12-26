#ifndef CTL_ALGORITHM_SORTING_H
#define CTL_ALGORITHM_SORTING_H

#include "../container/iterator.hpp"
#include "../memory/memory.hpp"
#include "../adapter/comparator.hpp"
#include "transform.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Algorithm-specific type constraints.
namespace Type::Algorithm {
	/// @brief Type must be able to be sorted.
	template <class T>
	concept Sortable = Type::Comparator::Threeway<T, T>;

	/// @brief Type must be an iterator, and its iterand must be sortable.
	template <class T>
	concept SortableIterator =
		Type::Container::Iterator<T>
	&&	Sortable<typename T::DataType>
	;

	static_assert(SortableIterator<Iterator<int>>);
}

/// @brief Sorting algorithm implementations.
namespace Sorting {
	/// @brief Sorts the given range of elements using insertion sort.
	/// @tparam T Element type.
	/// @param arr Pointer to beginning of range.
	/// @param sz Size of range.
	template<Type::Algorithm::Sortable T>
	constexpr void insertionSort(T* const arr, usize const sz) {
		for (usize i = 1; i < sz; ++i) {
			usize j = i-1;
			while(j-- > 0 && SimpleComparator<T>::lesser(arr[j+1], arr[j]))
				if (arr[j+1] < arr[j])
					swap(arr[j], arr[j+1]);
		}
	}

	// Based off of https://www.geeksforgeeks.org/merge-sort/
	// TODO: fix this
	/// @brief Sorts the given range of elements using merge sort.
	/// @tparam T Element type.
	/// @param arr Pointer to beginning of range.
	/// @param sz Size of range.
	template<Type::Algorithm::Sortable T>
	constexpr void mergeSort(T* const arr, usize const sz) {
		if (sz == 1) return;
		if (sz == 2) {
			if (SimpleComparator<T>::greater(arr[0], arr[1]))
				swap(arr[0], arr[1]);
			return;
		}
		usize
			szRight	= sz/2,
			szLeft	= szRight + (sz%2==0 ? 0 : 1)
		;
		T
			*left	= new T[szLeft],
			*right	= new T[szRight]
		;
		MX::objcopy(left, arr, szLeft);
		MX::objcopy(right, arr+szLeft, szRight);
		mergeSort(left, szLeft);
		mergeSort(right, szRight);
		usize
			i = 0,
			j = 0,
			k = szLeft
		;
		while (i < szLeft && j < szRight) {
			if (SimpleComparator<T>::lesserEquals(left[i], right[j]))
				arr[k] = left[i++];
			else
				arr[k] = right[j++];
			k++;
		}
		while (i < szLeft) arr[k++] = left[i++];
		while (j < szRight) arr[k++] = right[j++];
		delete[] left;
		delete[] right;
	}

	namespace Partial {
		// TODO: fix this
		template<Type::Algorithm::Sortable T>
		constexpr void mergeSort(T* const arr, usize const sz) {
			if (sz == 1) return;
			if (sz == 2) {
				if (SimpleComparator<T>::greater(arr[0], arr[1]))
					swap(arr[0], arr[1]);
				return;
			}
			usize
				szRight	= sz/2,
				szLeft	= szRight + (sz%2==0 ? 0 : 1)
			;
			T
				*left = new T[szLeft],
				*right = new T[szRight]
			;
			MX::objcopy(left, arr, szLeft);
			MX::objcopy(right, arr+szLeft, szRight);
			usize
				i = 0,
				j = 0,
				k = szLeft
			;
			while (i < szLeft && j < szRight) {
				if (SimpleComparator<T>::lesserEquals(left[i], right[j]))
					arr[k] = left[i++];
				else
					arr[k] = right[j++];
				k++;
			}
			while (i < szLeft) arr[k++] = left[i++];
			while (j < szRight) arr[k++] = right[j++];
			delete[] left;
			delete[] right;
		}
	}

	// Based off of Tim Sort, with minor changes
	// TODO: fix this (`mergeSort` not working, so must start by fixing that first)
	/// @brief Sorts the given range of elements using a modified version of TimSort.
	/// @tparam T Element type.
	/// @param arr Pointer to beginning of range.
	/// @param sz Size of range.
	template<Type::Algorithm::Sortable T>
	constexpr void vivoSort(T* const arr, usize const sz) {
		if (sz < 2) return;
		if (sz == 2) {
			if (SimpleComparator<T>::lesser(arr[0], arr[1]))
				swap(arr[0], arr[1]);
			return;
		}
		if (sz < 4) insertionSort(arr, sz);
		usize
			j		= 1,
			offset	= 0
		;
		usize const hibit = highBit(sz);
		typename Ordered::OrderType
			prevOrder = SimpleComparator<T>::compare(arr[1], arr[0]),
			currentOrder = prevOrder
		;
		for (usize run = ((hibit >> 4) > 4 ? (hibit >> 4) : 4); run < sz; run <<= 1) {
			for (usize i = 1; i < sz; ++i) {
				currentOrder = SimpleComparator<T>::compare(arr[i], arr[i-1]);
				if (currentOrder != prevOrder && currentOrder != Ordered::Order::EQUAL) {
					if (j < run) {
						j = (offset+j > sz) ? (sz-offset) : j;
						Partial::mergeSort(arr+offset, j);
					} else if (SimpleComparator<T>::lesser(arr[offset], arr[offset+j]))
						reverse(arr+offset, j);
					offset += j;
					j = 1;
					++i;
					if (i < sz)
						prevOrder = currentOrder = SimpleComparator<T>::compare(arr[i+1], arr[i]);
				} else ++j;
				if (currentOrder != Ordered::Order::EQUAL)
					prevOrder = currentOrder;
			}
			if (j == sz) {
				if (SimpleComparator<T>::greater(arr[0], arr[sz-1]))
					reverse(arr, sz);
				return;
			}
			currentOrder = prevOrder = SimpleComparator<T>::compare(arr[1], arr[0]);
			offset = 0;
		}
	}
}

/// @brief Sorts a given range of elements.
/// @tparam T Iterator type.
/// @param begin Iterator to beginning of range.
/// @param end Iterator to end of range.
template <Type::Algorithm::SortableIterator T>
constexpr void sort(T const& begin, T const& end) {
	Sorting::insertionSort(begin.raw(), end - begin + 1);
}

/// @brief Sorts a given range of elements.
/// @tparam T Element type.
/// @param begin Pointer to beginning of range.
/// @param end Pointer to end of range.
template <Type::Algorithm::Sortable T>
constexpr void sort(T* const begin, T* const end) {
	Sorting::insertionSort(begin, end - begin + 1);
}

CTL_NAMESPACE_END

#endif // CTL_ALGORITHM_FUNCTIONS_H
