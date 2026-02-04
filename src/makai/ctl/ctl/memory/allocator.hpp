#ifndef CTL_MEMORY_ALLOCATOR_H
#define CTL_MEMORY_ALLOCATOR_H

#include "../namespace.hpp"
#include "../ctypes.hpp"
#include "../typetraits/traits.hpp"
#include "../templates.hpp"
#include "core.hpp"

#include <memory>

CTL_NAMESPACE_BEGIN

namespace Type {
	/// @brief Memory-specific type constraints.
	namespace Memory {
		/// @brief Type must be a valid allocator for `TData`.
		template<template <class> class T, class TData>
		concept Allocator = requires (T<TData> t, usize sz, owner<TData> p) {
			{t.allocate(sz)}	-> Type::Equal<owner<TData>>	;
			{t.deallocate(p)}	-> Type::Equal<void>		;
		};

		/// @brief Type must be a valid constant allocator for `TData`.
		template<template <class> class T, class TData>
		concept ConstantAllocator = requires (T<TData> t, usize sz, owner<TData> p) {
			{t.allocate(sz)}		-> Type::Equal<owner<TData>>	;
			{t.deallocate(p, sz)}	-> Type::Equal<void>			;
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
	[[nodiscard, gnu::malloc, gnu::noinline]]
	owner<T> allocate(usize const sz) {
		if (!sz) return nullptr;
		return MX::malloc<T>(sz);
	}

	/// @brief Allocates space for a single element on the heap.
	/// @return Pointer to allocated memory.
	[[nodiscard, gnu::malloc, gnu::noinline]]
	owner<T> allocate() {
		return MX::malloc<T>();
	}

	/// @brief Deallocates allocated memory.
	/// @param mem Pointer to allocated memory.
	[[gnu::nonnull(2)]]
	void deallocate(owner<T> const mem, usize const = 0) {
		return MX::free<T>(mem);
	}

	/// @brief Resizes allocated memory.
	/// @param mem Memory to resize.
	/// @param sz New element count.
	[[deprecated("Please use proper value reallocation instead!")]]
	void resize(ref<T>& mem, usize const sz) {
		if (!mem) return;
		mem = MX::realloc<T>(mem, sz);
	}

	/// @brief Resizes allocated memory.
	/// @param mem Memory to resize.
	/// @param sz New element count.
	/// @return Pointer to new memory location, or `nullptr` if size is zero.
	[[deprecated("Please use proper value reallocation instead!"), nodiscard, gnu::nonnull(2)]]
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
	[[nodiscard, gnu::malloc, gnu::noinline]]
	consteval owner<T> allocate(usize const sz) {
		if (!sz) return nullptr;
		return impl.allocate(sz);
	}

	/// @brief Allocates space for a single element.
	/// @return Pointer to allocated memory.
	[[nodiscard, gnu::malloc, gnu::noinline]]
	consteval owner<T> allocate() {
		return impl.allocate(1);
	}

	/// @brief Deallocates allocated memory.
	/// @param mem Pointer to allocated memory.
	[[gnu::nonnull(2)]]
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
	[[nodiscard, gnu::malloc, gnu::noinline]]
	#ifdef CTL_EXPERIMENTAL_COMPILE_TIME_MEMORY
	constexpr
	#endif
	owner<TData> allocate(usize const sz) {
		CTL_DEVMODE_FN_DECL;
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
	[[nodiscard, gnu::malloc, gnu::noinline]]
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
	[[gnu::nonnull(2)]]
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
/// @tparam TData Type to handle memory for.
/// @tparam TAlloc<class> Runtime allocator type.
/// @tparam TAlloc<class> Compile-time allocator type. By default, it is `ConstantAllocator`.
template<class TData, template <class> class TAlloc, template <class> class TConstAlloc = ConstantAllocator>
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

CTL_NAMESPACE_END

#endif // CTL_MEMORY_ALLOCATOR_H
