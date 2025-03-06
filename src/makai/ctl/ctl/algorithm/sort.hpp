#ifndef CTL_ALGORITHM_SORTING_H
#define CTL_ALGORITHM_SORTING_H

#include "../container/iterator.hpp"
#include "../memory/memory.hpp"
#include "../adapter/comparator.hpp"
#include "transform.hpp"

CTL_NAMESPACE_BEGIN

/*
	Other possible algorithms:
 	+ Radix sort (https://pt.wikipedia.org/wiki/Radix_sort#C%C3%B3digo_em_C);
  		- Requires arithmetic operations on element types, so maybe not :/
  	- Comb sort (https://pt.wikipedia.org/wiki/Comb_sort#C)
*/

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
	// TODO: Fix this mess
	// Based off of https://www.geeksforgeeks.org/3-way-quicksort-dutch-national-flag/
	/// @brief Threeway quicksort implementation.
	namespace QuickSort3 {
		template <Type::Algorithm::Sortable T>
		constexpr void partition(ref<T> const arr, ssize left, ssize right, ssize& start, ssize& stop) {
			ssize
				i = start	= left-1,
				j = stop	= right
			;
			T pivot = arr[right];
			while (true) {
				while (SimpleComparator<T>::lesser(arr[++left], pivot));
				while (SimpleComparator<T>::greater(arr[--right], pivot))
					if (left == right) break;
				if (left >= right) break;
				swap(arr[left], arr[right]);
				if (SimpleComparator<T>::equals(arr[left], pivot))
					swap(arr[++i], arr[start]);
				if (SimpleComparator<T>::equals(arr[left], pivot))
					swap(arr[--j], arr[stop]);
			}
			swap(arr[start], arr[right]);
			stop = start - 1;
			for (ssize k = left; k < i; ++k, --stop)
				swap(arr[k], arr[stop]);
			++start;
			for (ssize k = right - 1; k > j; --k, ++start)
				swap(arr[k], arr[start]);
		}
		
		template <Type::Algorithm::Sortable T>
		constexpr void sort(ref<T> const arr, usize const left, usize const right) {
			if (left < right)
				return;
			ssize start, stop;
			partition(arr, left, right, start, stop);
			sort(arr, left, stop);
			sort(arr, start, right);
		}
	}

	/// @brief Sorts the given range of elements using 3-way quick sort.
	/// @tparam T Element type.
	/// @param arr Pointer to beginning of range.
	/// @param sz Size of range.
	template<Type::Algorithm::Sortable T>
	constexpr void quickSort3(ref<T> const arr, usize const sz) {
		QuickSort3::sort(arr, 0, sz-1);
	}

	// Based off of https://pt.wikipedia.org/wiki/Shell_sort#Código_em_C
	/// @brief Sorts the given range of elements using shell sort.
	/// @tparam T Element type.
	/// @param arr Pointer to beginning of range.
	/// @param sz Size of range.
	template<Type::Algorithm::Sortable T>
	constexpr void shellSort(ref<T> const arr, usize const sz) {
		usize gap = 1;
		while (gap < sz) gap = (gap * 3) + 1;
		while (gap > 0) {
			for (usize i = gap; i < sz; ++i) {
				T const val = arr[i];
				usize j;
				for (j = i; j >= gap && SimpleComparator<T>::greater(arr[j-gap], val); j -= gap)
					arr[j] = arr[j-gap];
				arr[j] = val;
			}
			gap /= 3;
		}
	}

	/// @brief Sorts the given range of elements using insertion sort.
	/// @tparam T Element type.
	/// @param arr Pointer to beginning of range.
	/// @param sz Size of range.
	template<Type::Algorithm::Sortable T>
	constexpr void insertionSort(ref<T> const arr, usize const sz) {
		for (usize i = 1; i < sz; ++i) {
			usize j = i-1;
			while(j-- > 0 && SimpleComparator<T>::lesser(arr[j+1], arr[j]))
				if (arr[j+1] < arr[j])
					swap(arr[j], arr[j+1]);
		}
	}

	// Based off of https://pt.wikipedia.org/wiki/Merge_sort#Implementa%C3%A7%C3%A3o_em_C++
	/// @brief Merge sort implementation.
	namespace MergeSort {
		template <Type::Algorithm::Sortable T>
		constexpr void merge(ref<T> const arr, usize const start, usize const mid, usize const stop, ref<T> const aux) {
			usize left	= start;
			usize right	= mid;
			for (usize i = start; i < stop; ++i)
				if (left < mid && (right >= stop || SimpleComparator<T>::lesser(arr[left], arr[right])))
					aux[i] = arr[left++];
				else
					aux[i] = arr[right++];
			for (usize i = start; i < stop; ++i)
					arr[i] = aux[i];
		}
	
		template <Type::Algorithm::Sortable T>
		constexpr void sort(ref<T> const arr, usize const start, usize const stop, ref<T> const aux) {
			if ((stop - start) < 2)
				return;
			usize const mid = (start + stop) / 2;
			sort(arr, start, mid, aux);
			sort(arr, mid, stop, aux);
			merge(arr, start, mid, stop, aux);
		}
	}

	/// @brief Sorts the given range of elements using merge sort.
	/// @tparam T Element type.
	/// @param arr Pointer to beginning of range.
	/// @param sz Size of range.
	template <Type::Algorithm::Sortable T>
	constexpr void mergeSort(ref<T> const arr, usize const sz) {
		if (sz < 2) return;
		else if (sz == 2) {
			if (arr[0] > arr[1])
				swap(arr[0], arr[1]);
			return;
		}
		T* aux = new T[sz];
		MergeSort::sort(arr, 0, sz, aux);
		delete[] aux;
	}

	/// @brief partial algorithm implementations.
	namespace Partial {
		// TODO: fix(?) this
		template<Type::Algorithm::Sortable T>
		constexpr void mergeSort(ref<T> const arr, usize const sz) {
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
			ref<T>
				left	= new T[szLeft],
				right	= new T[szRight]
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
	// TODO: fix this (`mergeSort` maybe not working)
	/// @brief Sorts the given range of elements using a modified version of TimSort.
	/// @tparam T Element type.
	/// @param arr Pointer to beginning of range.
	/// @param sz Size of range.
	template<Type::Algorithm::Sortable T>
	constexpr void vivoSort(ref<T> const arr, usize const sz) {
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
						mergeSort(arr+offset, j);
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
	Sorting::shellSort(begin.raw(), end - begin);
}

/// @brief Sorts a given range of elements.
/// @tparam T Element type.
/// @param begin Pointer to beginning of range.
/// @param end Pointer to end of range.
template <Type::Algorithm::Sortable T>
constexpr void sort(ref<T> const begin, ref<T> const end) {
	Sorting::shellSort(begin, end - begin);
}

namespace {
	template<class T, usize N>
	constexpr bool isSortingCorrectly(As<T const[N]> const& arr) {
		As<T[N]> buf;
		for (usize i = 0; i < N; ++i)
			buf[i] = arr[i];
		sort(buf, buf + N-1);
		for (usize i = 0; i < N-1; ++i)
			if (buf[i] > buf[i+1]) return false;
		return true;		
	}
}
/*
static_assert(isSortingCorrectly<int>({10, 1, -1, -43, 281, 34, 35, 819, 22, -77, -1024, -2048}));
static_assert(isSortingCorrectly<int>({-2048, 10, 1, -1, -43, 281, 34, 35, 819, 22, -77, -1024}));
static_assert(isSortingCorrectly<int>({-1024, -2048, 10, 1, -1, -43, 281, 34, 35, 819, 22, -77}));
static_assert(isSortingCorrectly<int>({-77, -1024, -2048, 10, 1, -1, -43, 281, 34, 35, 819, 22}));
static_assert(isSortingCorrectly<int>({22, -77, -1024, -2048, 10, 1, -1, -43, 281, 34, 35, 819}));
static_assert(isSortingCorrectly<int>({819, 22, -77, -1024, -2048, 10, 1, -1, -43, 281, 34, 35}));
static_assert(isSortingCorrectly<int>({35, 819, 22, -77, -1024, -2048, 10, 1, -1, -43, 281, 34}));

static_assert(isSortingCorrectly<int>({12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4}));
*/
CTL_NAMESPACE_END

#endif // CTL_ALGORITHM_FUNCTIONS_H
