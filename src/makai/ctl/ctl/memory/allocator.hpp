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

		/// @brief Type must be a valid constant allocator for `TData`.
		template<template <class> class T, class TData>
		concept ConstantAllocator = requires (T<TData> t, usize sz, TData* p) {
			{t.allocate(sz)}		-> Type::Equal<TData*>	;
			{t.deallocate(p, sz)}	-> Type::Equal<void>	;
		};
	}
}

/// @brief Default allocator. Allocates from the heap, as the name implies.
/// @tparam T Type to handle memory for.
template<Type::NonVoid T>
struct HeapAllocator {
	using DataType = T;

	/// @brief Allocates space for elements on the heap.
	/// @param sz Element count to allocate for.
	/// @return Pointer to allocated memory, or `nullptr` if size is zero.
	[[nodiscard, gnu::always_inline]]
	owner<T> allocate(usize const sz) {
		if (!sz) return nullptr;
		return MX::malloc<T>(sz);
	}

	/// @brief Allocates space for a single element on the heap.
	/// @return Pointer to allocated memory.
	[[nodiscard, gnu::always_inline]]
	owner<T> allocate() {
		return MX::malloc<T>();
	}

	/// @brief Deallocates allocated memory.
	/// @param mem Pointer to allocated memory.
	[[gnu::always_inline]]
	void deallocate(owner<T> const mem, usize const = 0) {
		return MX::free<T>(mem);
	}

	/// @brief Resizes allocated memory.
	/// @param mem Memory to resize.
	/// @param sz New element count.
	[[deprecated, gnu::always_inline]]
	void resize(ref<T>& mem, usize const sz) {
		if (!mem) return;
		mem = MX::realloc<T>(mem, sz);
	}

	/// @brief Resizes allocated memory.
	/// @param mem Memory to resize.
	/// @param sz New element count.
	/// @return Pointer to new memory location, or `nullptr` if size is zero.
	[[deprecated, nodiscard, gnu::always_inline]]
	owner<T> resized(owner<T> const mem, usize const sz) {
		if (!mem) return nullptr;
		return MX::realloc<T>(mem, sz);
	}
};

/// @brief Compile-time allocator.
/// @tparam T Type to handle memory for.
template<Type::NonVoid T>
struct ConstantAllocator {
	using DataType = T;

	/// @brief Allocates space for elements.
	/// @param sz Element count to allocate for.
	/// @return Pointer to allocated memory, or `nullptr` if size is zero.
	[[nodiscard, gnu::always_inline]]
	consteval owner<T> allocate(usize const sz) {
		if (!sz) return nullptr;
		return impl.allocate(sz);
	}

	/// @brief Allocates space for a single element.
	/// @return Pointer to allocated memory.
	[[nodiscard, gnu::always_inline]]
	consteval owner<T> allocate() {
		return impl.allocate(1);
	}

	/// @brief Deallocates allocated memory.
	/// @param mem Pointer to allocated memory.
	[[gnu::always_inline]]
	consteval void deallocate(owner<T> const mem, usize const sz = 0) {
		return impl.deallocate(mem, sz);
	}

private:
	/// @brief Implementation.
	std::allocator<T> impl;
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
	/// @tparam T Type to handle memory for. By default, it is the same as the previous type to handle memory for.
	template<class T = TData>
	using AllocatorTemplateType	= TAlloc<T>;
};

/// @brief Tags the class as manually managing compile-time memory.
/// @tparam TData Type to handle memory for.
/// @tparam TAlloc<class> Constant allocator type. By default, it is `ConstantAllocator`.
template<class TData, template <class> class TConstAlloc = ConstantAllocator>
struct ConstantAllocatable {
	/// @brief Constant allocator type.
	using ConstantAllocatorType			= TConstAlloc<TData>;
	/// @brief Constant allocator template.
	/// @tparam T Type to handle memory for. By default, it is the same as the previous type to handle memory for.
	template<class T = TData>
	using ConstantAllocatorTemplateType	= TConstAlloc<T>;
};

/// @brief Context-aware memory allocator.
/// @tparam TAlloc Runtime allocator type.
/// @tparam TConstAlloc Compile-time Allocator type.
/// @tparam TData Type to handle memory for.
template<template <class> class TAlloc, template <class> class TConstAlloc, class TData>
struct ContextAllocator {
	using DataType = TData;

	/// @brief Allocates space for elements.
	/// @param sz Element count to allocate for.
	/// @return Pointer to allocated memory, or `nullptr` if size is zero.
	[[nodiscard, gnu::always_inline]]
	#ifdef CTL_EXPERIMENTAL_COMPILE_TIME_MEMORY
	constexpr
	#endif
	owner<TData> allocate(usize const sz) {
		if (!sz) return nullptr;
		#ifdef CTL_EXPERIMENTAL_COMPILE_TIME_MEMORY
		if (inCompileTime())
			return calloc.allocate(sz);
		else
		#endif
		return alloc.allocate(sz);
	}

	/// @brief Allocates space for a single element.
	/// @return Pointer to allocated memory.
	[[nodiscard, gnu::always_inline]]
	#ifdef CTL_EXPERIMENTAL_COMPILE_TIME_MEMORY
	constexpr
	#endif
	owner<TData> allocate() {
		#ifdef CTL_EXPERIMENTAL_COMPILE_TIME_MEMORY
		if (inCompileTime())
			return calloc.allocate();
		else
		#endif
		return alloc.allocate();
	}

	/// @brief Deallocates allocated memory.
	/// @param mem Pointer to allocated memory.
	[[gnu::always_inline]]
	#ifdef CTL_EXPERIMENTAL_COMPILE_TIME_MEMORY
	constexpr
	#endif
	void deallocate(owner<TData> const mem, usize const sz = 0) {
		#ifdef CTL_EXPERIMENTAL_COMPILE_TIME_MEMORY
		if (inCompileTime())
			return calloc.deallocate(mem, sz);
		else
		#endif
		return alloc.deallocate(mem, sz);
	}

	/// @brief Returns the associated allocator.
	/// @return Allocator.
	[[gnu::always_inline]]
	constexpr auto allocator() const	{return alloc;}
	/// @brief Returns the associated allocator.
	/// @return Allocator.
	[[gnu::always_inline]]
	constexpr auto& allocator() 		{return alloc;}

	#ifdef CTL_EXPERIMENTAL_COMPILE_TIME_MEMORY
	/// @brief Returns the associated constant allocator.
	/// @return Constant allocator.
	[[gnu::always_inline]]
	constexpr auto constantAllocator() const	{return calloc;}
	/// @brief Returns the associated constant allocator.
	/// @return Constant allocator.
	[[gnu::always_inline]]
	constexpr auto& constantAllocator() 		{return calloc;}
	#endif

private:
	#ifdef CTL_EXPERIMENTAL_COMPILE_TIME_MEMORY
	/// @brief Constant allocator.
	TConstAlloc<TData>	calloc;
	#endif
	/// @brief Allocator.
	TAlloc<TData>		alloc;
};

/// @brief Tags the class as manually managing memory, and is aware of evaluation contexts.
/// @tparam TAlloc<class> Runtime allocator type. 
/// @tparam TData Type to handle memory for.
/// @tparam TAlloc<class> Compile-time allocator type. By default, it is `ConstantAllocator`.
template<template <class> class TAlloc, class TData, template <class> class TConstAlloc = ConstantAllocator>
requires Type::Memory::Allocator<TAlloc, TData>
struct ContextAwareAllocatable:
	Allocatable<TAlloc, TData>,
	ConstantAllocatable<TData, TConstAlloc>  {
	using Allocatable			= ::CTL::Allocatable<TAlloc, TData>;
	using ConstantAllocatable	= ::CTL::ConstantAllocatable<TData, TConstAlloc>;

	using
		typename Allocatable::AllocatorType,
		typename ConstantAllocatable::ConstantAllocatorType
	;

	/// @brief Context-aware allocator type.
	using ContextAllocatorType = ContextAllocator<TAlloc, TConstAlloc, TData>; 
};

/// @brief Automatically-managed memory slice.
/// @tparam TData Type to handle memory for.
template<typename TData = void, template <class> class TAlloc = HeapAllocator>
struct MemorySlice:
	Typed<TData>,
	SelfIdentified<MemorySlice<TData, TAlloc>>,
	ContextAwareAllocatable<TAlloc, TData> {
	using SelfIdentified			= ::CTL::SelfIdentified<MemorySlice<TData, TAlloc>>;
	using ContextAwareAllocatable	= ::CTL::Allocatable<TAlloc, TData>;
	using Typed						= ::CTL::Typed<TData>;

	using
		typename ContextAwareAllocatable::ContextAllocatorType
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
		if (inCompileTime())
			for (usize i = 0; i < length; ++i)
				contents[i] = other.contents[i];
		else if constexpr(Type::Standard<TData>)
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
		else alloc.resize(contents, sz, length);
		length = sz;
	}

	/// @brief Frees the memory managed by the slice.
	constexpr void free() {
		if (!contents) return;
		alloc.deallocate(contents, length);
		contents	= nullptr;
		length		= 0;
	}

private:
	/// @brief Memory allocator.
	ContextAllocatorType	alloc;
	/// @brief Managed memory.
	PointerType				contents	= nullptr;
	/// @brief Element count.
	usize					length		= 0;
};

CTL_NAMESPACE_END

#endif // CTL_MEMORY_ALLOCATOR_H
