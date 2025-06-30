#ifndef CTL_MEMORY_ALLOCATOR_H
#define CTL_MEMORY_ALLOCATOR_H

#include "../namespace.hpp"
#include "../ctypes.hpp"
#include "../typetraits/traits.hpp"
#include "../templates.hpp"
#include "core.hpp"

CTL_NAMESPACE_BEGIN

namespace Type {
	/// @brief Memory-specific type constraints.
	namespace Memory {
		/// @brief Type must be a valid allocator for `TData`.
		template<template <class> class T, class TData>
		concept Allocator = requires (T<TData> t, usize sz, TData* p) {
			{t.allocate(sz)}	-> Type::Equal<TData*>	;
			{t.deallocate(p)}	-> Type::Equal<void>	;
			{t.resize(p, sz)}	-> Type::Equal<void>	;
			{t.resized(p, sz)}	-> Type::Equal<TData*>	;
		};
	}
}

/// @brief Default allocator. Allocates from the heap, as the name implies.
/// @tparam T Type to handle memory for.
template<class T>
struct HeapAllocator {
	/// @brief Allocates memory on the heap.
	/// @param sz Size of memory to allocate.
	/// @return Pointer to allocated memory, or `nullptr` if size is zero.
	constexpr pointer allocate(usize const sz)
	requires Type::Void<T> {
		if (!sz) return nullptr;
		return MX::malloc(sz);
	}

	/// @brief Allocates space for elements on the heap.
	/// @param sz Element count to allocate for.
	/// @return Pointer to allocated memory, or `nullptr` if size is zero.
	constexpr owner<T> allocate(usize const sz)
	requires Type::NonVoid<T> {
		if (!sz) return nullptr;
		return MX::malloc<T>(sz);
	}

	/// @brief Allocates space for a single element on the heap.
	/// @return Pointer to allocated memory.
	constexpr owner<T> allocate()
	requires Type::NonVoid<T> {
		return MX::malloc<T>();
	}

	/// @brief Deallocates allocated memory.
	/// @param mem Pointer to allocated memory.
	constexpr void deallocate(pointer const& mem)
	requires Type::Void<T> {
		return MX::free(mem);
	}

	/// @brief Deallocates allocated memory.
	/// @param mem Pointer to allocated memory.
	constexpr void deallocate(owner<T> const mem)
	requires Type::NonVoid<T> {
		return MX::free<T>(mem);
	}

	/// @brief Resizes allocated memory.
	/// @param mem Memory to resize.
	/// @param sz New size.
	constexpr void resize(pointer& mem, usize const sz)
	requires Type::Void<T> {
		if (!mem) return;
		mem = MX::realloc(mem, sz);
	}

	/// @brief Resizes allocated memory.
	/// @param mem Memory to resize.
	/// @param sz New element count.
	constexpr void resize(ref<T>& mem, usize const sz)
	requires Type::NonVoid<T> {
		if (!mem) return;
		mem = MX::realloc<T>(mem, sz);
	}

	/// @brief Resizes allocated memory.
	/// @param mem Memory to resize.
	/// @param sz New size.
	/// @return Pointer to new memory location, or `nullptr` if size is zero.
	constexpr pointer resized(pointer const& mem, usize const sz)
	requires Type::Void<T> {
		if (!mem) return nullptr;
		return MX::realloc(mem, sz);
	}

	/// @brief Resizes allocated memory.
	/// @param mem Memory to resize.
	/// @param sz New element count.
	/// @return Pointer to new memory location, or `nullptr` if size is zero.
	constexpr owner<T> resized(owner<T> const mem, usize const sz)
	requires Type::NonVoid<T> {
		if (!mem) return nullptr;
		return MX::realloc<T>(mem, sz);
	}
};

/// @brief Tags the class as manually managing memory.
/// @tparam TAlloc<class> Allocator type. 
/// @tparam TData Type to handle memory for.
template<template <class> class TAlloc, class TData>
requires Type::Memory::Allocator<TAlloc, TData>
struct Allocatable {
	/// @brief Allocator type.
	using AllocatorType			= TAlloc<TData>;
	/// @brief Allocator template.
	/// @tparam T Type to handle memory for. By default, it is the same as the type to handle memory for.
	template<class T = TData>
	using AllocatorTemplateType	= TAlloc<T>;
};

/// @brief Automatically-managed memory slice.
/// @tparam TData Type to handle memory for.
template<typename TData = void, template <class> class TAlloc = HeapAllocator>
struct MemorySlice:
	Typed<TData>,
	SelfIdentified<MemorySlice<TData, TAlloc>>,
	Allocatable<TAlloc, TData> {
	using SelfIdentified	= ::CTL::SelfIdentified<MemorySlice<TData, TAlloc>>;
	using Allocatable		= ::CTL::Allocatable<TAlloc, TData>;
	using Typed				= ::CTL::Typed<TData>;

	using
		typename Allocatable::AllocatorType
	;

	using typename SelfIdentified::SelfType;

	using
		typename Typed::DataType,
		typename Typed::PointerType
	;

	/// @brief Default constructor.
	constexpr MemorySlice()					{				}
	/// @brief Constructs the memory slice with space for a number of elements.
	/// @param sz Element count to allocate for.
	constexpr MemorySlice(usize const sz)	{invoke(sz);	}

	/// @brief Copy constructor (`MemorySlice`).
	/// @param other `MemorySlice` to copy from.
	constexpr MemorySlice(SelfType const& other)	{
		invoke(other.length);
		if constexpr(Type::Standard<TData>)
			MX::memcpy<TData>(contents, other.contents, length);
		else MX::objcopy<TData>(contents, other.contents, length);
	}

	/// @brief Move constructor (`MemorySlice`).
	/// @param other `MemorySlice` to move.
	constexpr MemorySlice(SelfType&& other):
		contents(::CTL::move(other.contents)),
		length(::CTL::move(other.length)) {
		other.contents = nullptr;
	}

	/// @brief Destructor.
	constexpr ~MemorySlice() {free();}

	/// @brief Returns the maximum element count of the memory slice.
	/// @return Maxumum element count of memory slice.
	constexpr usize size() const		{return length;							}
	/// @brief Returns the size (in bytes) of the memory slice.
	/// @return Size of the memory slice.
	constexpr usize byteSize() const	{return length * sizeof (TData);		}
	/// @brief Returns a pointer to the start of the memory slice.
	/// @return Pointer to start of memory slice.
	constexpr PointerType data() const	{return contents;						}

protected:
	/// @brief Allocates (or resizes) the memory slice.
	/// @param sz Element count.
	constexpr void invoke(usize const sz) {
		if (!sz) return;
		if (!contents) contents = alloc.allocate(sz);
		else alloc.resize(contents, sz);
		length = sz;
	}

	/// @brief Frees the memory managed by the slice.
	constexpr void free() {
		if (!contents) return;
		alloc.deallocate(contents);
		contents	= nullptr;
		length		= 0;
	}

private:
	/// @brief Memory allocator.
	AllocatorType	alloc;
	/// @brief Managed memory.
	PointerType		contents	= nullptr;
	/// @brief Element count.
	usize			length		= 0;
};

CTL_NAMESPACE_END

#endif // CTL_MEMORY_ALLOCATOR_H
