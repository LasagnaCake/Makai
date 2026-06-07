#ifndef CTL_CONTAINER_LISTS_LIST_H
#define CTL_CONTAINER_LISTS_LIST_H

#ifdef CTL_USE_OLD_LIST
#include "list.old.hpp"
#else
#include "list.new.hpp"
#endif

CTL_NAMESPACE_BEGIN
//static_assert(List<int>().empty());

/// @brief `List` analog for dynamic array of bytes.
/// @tparam TIndex Index type.
/// @tparam TAlloc<class> Allocator type.
/// @tparam TConstAlloc<class> Constant allocator type.
template <Type::Integer TIndex = usize, template <class> class TAlloc = HeapAllocator, template <class> class TConstAlloc = ConstantAllocator>
using BinaryData = List<byte, TIndex, TAlloc, TConstAlloc>;

/// @brief `List` analog for dynamic array of bytes.
/// @tparam TIndex Index type.
/// @tparam TAlloc<class> Allocator type.
/// @tparam TConstAlloc<class> Constant allocator type.
template <Type::Integer TIndex = usize, template <class> class TAlloc = HeapAllocator, template <class> class TConstAlloc = ConstantAllocator>
using ByteList = BinaryData<TIndex, TAlloc, TConstAlloc>;

/// @brief `List` analog for dynamic array of bytes.
/// @tparam TIndex Index type.
/// @tparam TAlloc<class> Allocator type.
/// @tparam TConstAlloc<class> Constant allocator type.
template <Type::Integer TIndex = usize, template <class> class TAlloc = HeapAllocator, template <class> class TConstAlloc = ConstantAllocator>
using Binary = BinaryData<TIndex, TAlloc, TConstAlloc>;

/// @brief `List` analog for dynamic array of bytes.
/// @tparam TIndex Index type.
/// @tparam TAlloc<class> Allocator type.
/// @tparam TConstAlloc<class> Constant allocator type.
template <Type::Integer TIndex = usize, template <class> class TAlloc = HeapAllocator, template <class> class TConstAlloc = ConstantAllocator>
using Bytes = BinaryData<TIndex, TAlloc, TConstAlloc>;

static_assert(Type::Container::List<List<int>>);
CTL_NAMESPACE_END

#endif // CTL_CONTAINER_LIST_H
