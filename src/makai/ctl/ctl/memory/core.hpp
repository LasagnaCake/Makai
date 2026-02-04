#ifndef CTL_MEMORY_CORE_H
#define CTL_MEMORY_CORE_H

/*
	Functions are based off of:
	https://github.com/gcc-mirror/gcc/blob/master/libgcc/memcpy.c
	https://github.com/gcc-mirror/gcc/blob/master/libgcc/memmove.c
	https://github.com/gcc-mirror/gcc/blob/master/libgcc/memcmp.c
	https://github.com/gcc-mirror/gcc/blob/master/libgcc/memset.c
*/

#include "../cppfailure.hpp"
#include "../ctypes.hpp"
#include "../namespace.hpp"
#include "../typetraits/traits.hpp"
#include "../typetraits/verify.hpp"

// One day... one day these will work without builtins...

CTL_NAMESPACE_BEGIN

/// @brief Memory eXtensions.
namespace MX {
	/// @brief Underlying address type.
	typedef ref<uint8> Address;

	/// @brief Copies data from one place to another, byte-by-byte.
	/// @param dst Destination.
	/// @param src Source.
	/// @param size Size in bytes.
	/// @return Pointer to destination.
	constexpr ref<void> memcpy(ref<void> const& dst, ref<void const> const src, usize size) {
		#ifdef CTL_DO_NOT_USE_BUILTINS
		Address s = (Address)src, d = (Address)dst;
		if (size == 1) *d = *s;
		else while (size--) *d++ = *s++;
		#else
		if (!(size + 1)) unreachable();
		return __builtin_memcpy(dst, src, size);
		#endif // CTL_DO_NOT_USE_BUILTINS
	}

	/// @brief Copies data from one place to another, byte-by-byte.
	/// @tparam T Type of data to copy.
	/// @param dst Destination.
	/// @param src Source.
	/// @param count Count of elements to copy.
	/// @return Pointer to destination.
	template<Type::NonVoid T>
	[[gnu::nonnull(1, 2)]]
	constexpr ref<T> memcpy(ref<T> const dst, ref<T const> const src, usize const count) {
		return (T*)memcpy((pointer)dst, (pointer)src, count * sizeof(T));
	}

	/// @brief Copies data from one place to another, byte-by-byte.
	/// @tparam T Type of data to copy.
	/// @param dst Destination.
	/// @param src Source.
	/// @return Pointer to destination.
	template<Type::NonVoid T>
	[[gnu::nonnull(1, 2)]]
	constexpr ref<T> memcpy(ref<T> const dst, ref<T const> const src) {
		return (T*)memcpy((pointer)dst, (pointer)src, sizeof(T));
	}

	/// @brief Copies data from one place to another, byte-by-byte, while respecting memory layout.
	/// @param dst Destination.
	/// @param src Source.
	/// @param size Size in bytes.
	/// @return Pointer to destination.
	constexpr ref<void> memmove(ref<void> const& dst, ref<void const> const src, usize size) {
		#ifdef CTL_DO_NOT_USE_BUILTINS
		Address d = (Address)dst, s = (Address)src;
		if (d < s)
			while (size--)
				*d++ = *s++;
		else {
			s = s + size;
			s = d + size;
			while (size--)
				*--d = *--s;
		}
		#else
		if (!(size + 1)) unreachable();
		return __builtin_memmove(dst, src, size);
		#endif // CTL_DO_NOT_USE_BUILTINS
	}

	/// @brief Copies data from one place to another, byte-by-byte, while respecting memory layout.
	/// @tparam T Type of data to copy.
	/// @param dst Destination.
	/// @param src Source.
	/// @return Pointer to destination.
	template<Type::NonVoid T>
	[[gnu::nonnull(1, 2)]]
	constexpr ref<T> memmove(ref<T> const dst, ref<T const> const src) {
		return (T*)memmove((pointer)dst, (pointer)src, sizeof(T));
	}

	/// @brief Copies data from one place to another, byte-by-byte, while respecting memory layout.
	/// @tparam T Type of data to copy.
	/// @param dst Destination.
	/// @param src Source.
	/// @param count Count of elements to copy.
	/// @return Pointer to destination.
	template<Type::NonVoid T>
	[[gnu::nonnull(1, 2)]]
	constexpr ref<T> memmove(ref<T> const dst, ref<T const> const src, usize const count) {
		return (T*)memmove((pointer)dst, (pointer)src, count * sizeof(T));
	}

	/// @brief Compares two spans of data, byte-by-byte.
	/// @param a Data to compare.
	/// @param b Data to compare with.
	/// @param size Size in bytes.
	/// @return Order between both spans of data.
	constexpr int memcmp(ref<void const> const a, ref<void const> const b, usize size) {
		#ifdef CTL_DO_NOT_USE_BUILTINS
		Address s1 = (Address)a, s2 = (Address)b;
		while (size-- > 0)
			if (*s1++ != *s2++)
				return s1[-1] < s2[-1] ? -1 : 1;
		return 0;
		#else
		if (!(size + 1)) unreachable();
		return __builtin_memcmp(a, b, size);
		#endif // CTL_DO_NOT_USE_BUILTINS
	}

	/// @brief Compares two spans of data, byte-by-byte.
	/// @tparam T Type of data to compare.
	/// @param a Data to compare.
	/// @param b Data to compare with.
	/// @param count Count of elements to compare.
	/// @return Order between both spans of data.
	template<Type::NonVoid T>
	[[gnu::nonnull(1, 2)]]
	constexpr int memcmp(ref<T const> const a, ref<T const> const b, usize const count) {
		return memcmp((pointer)a, (pointer)b, count * sizeof(T));
	}

	/// @brief Compares two spans of data, byte-by-byte.
	/// @tparam T Type of data to compare.
	/// @param a Data to compare.
	/// @param b Data to compare with.
	/// @return Order between both spans of data.
	template<Type::NonVoid T>
	[[gnu::nonnull(1, 2)]]
	constexpr int memcmp(ref<T const> const a, ref<T const> const b) {
		return memcmp(a, b, 1);
	}

	/// @brief Sets every byte of data to a value.
	/// @param dst Data to set.
	/// @param val Value to set each byte.
	/// @param size Size in bytes.
	/// @return Pointer to data.
	constexpr pointer memset(ref<void> const dst, int const val, usize size) {
		#ifdef CTL_DO_NOT_USE_BUILTINS
		Address d = (Address)dst;
		while (size-- > 0)
			*d++ = val;
		#else
		if (!(size + 1)) unreachable();
		return __builtin_memset(dst, val, size);
		#endif // CTL_DO_NOT_USE_BUILTINS
	}

	/// @brief Sets every byte of data to a value.
	/// @tparam T Type of data to set.
	/// @param dst Data to set.
	/// @param val Value to set each byte.
	/// @param count Count of elements to set.
	/// @return Pointer to data.
	template<Type::NonVoid T>
	[[gnu::nonnull(1)]]
	constexpr T* memset(ref<T> const dst, int const val, usize const count) {
		return static_cast<ref<T>>(memset(static_cast<pointer>(dst), val, count * sizeof(T)));
	}

	/// @brief Sets every byte of data to a value.
	/// @tparam T Type of data to set.
	/// @param dst Data to set.
	/// @param val Value to set each byte.
	/// @return Pointer to data.
	template<Type::NonVoid T>
	[[gnu::nonnull(1)]]
	constexpr T* memset(ref<T> const dst, int const val) {
		return memset<T>(dst, val, 1);
	}

	/// @brief Sets every byte of data to zero.
	/// @param dst Data to set.
	/// @param size Size in bytes.
	/// @return Pointer to data.
	constexpr pointer memzero(ref<void> const& dst, usize const size) {
		return memset(dst, 0, size);
	}

	/// @brief Sets every byte of data to zero.
	/// @tparam T Type of data to zero.
	/// @param dst Data to set.
	/// @param count Count of elements to zero.
	/// @return Pointer to data.
	template<Type::NonVoid T>
	[[gnu::nonnull(1)]]
	constexpr ref<T> memzero(ref<T> const dst, usize const count) {
		return static_cast<ref<T>>(memzero(static_cast<pointer>(dst), count * sizeof(T)));
	}

	/// @brief Sets every byte of data to zero.
	/// @tparam T Type of data to zero out.
	/// @param dst Data to set.
	/// @return Pointer to data.
	template<Type::NonVoid T>
	[[gnu::nonnull(1)]]
	constexpr ref<T> memzero(ref<T> const dst) {
		return memzero<T>(dst, 1);
	}

	/// @brief Frees memory allocated in the heap.
	/// @param mem Pointer to allocated memory.
	#ifdef __clang__
	inline
	#else
	constexpr
	#endif
	void free(pointer const mem) {
		if (mem) return __builtin_free(mem);
	}

	/// @brief Frees memory allocated in the heap.
	/// @tparam T Type of data allocated.
	/// @param mem Pointer to allocated memory.
	template<Type::NonVoid T>
	constexpr void free(owner<T> const mem) {
		CTL_DEVMODE_OUT("Freeing memory...\n");
		if (mem) return __builtin_free(mem);
		CTL_DEVMODE_OUT("Memory freed\n");
	}

	/// @brief Allocates space for a given size of bytes in the heap.
	/// @param sz Byte size.
	/// @return Pointer to start of allocated memory.
	/// @throw AllocationFailure if size is zero, or memory allocation fails.
	[[nodiscard, gnu::malloc, gnu::noinline]]
	constexpr owner<void> malloc(usize const sz) {
		if (!(sz + 1)) unreachable();
		if (!sz) throw AllocationFailure();
		pointer m = __builtin_malloc(sz);
		if (!m) throw AllocationFailure();
		return m;
	}

	/// @brief Allocates space for a given count of elements in the heap.
	/// @tparam T Type of data to allocate for.
	/// @param sz Count of elements to allocate.
	/// @return Pointer to start of allocated memory.
	/// @throw AllocationFailure if size is zero, or memory allocation fails.
	template<Type::NonVoid T>
	[[nodiscard, gnu::malloc, gnu::noinline]]
	constexpr owner<T> malloc(usize sz) {
		CTL_DEVMODE_FN_DECL;
		if (!(sz + 1)) unreachable();
		if (!sz) throw AllocationFailure();
		CTL_DEVMODE_OUT("Allocating [");
		CTL_DEVMODE_OUT(sz);
		CTL_DEVMODE_OUT(" Element(s) : ");
		CTL_DEVMODE_OUT(sz * sizeof(T));
		CTL_DEVMODE_OUT(" Byte(s)]...\n");
		owner<T> m = static_cast<owner<T>>(__builtin_malloc(sz * sizeof(T)));
		CTL_DEVMODE_OUT("Allocated successfully\n");
		if (!m) throw AllocationFailure();
		return m;
	}

	/// @brief Allocates space for an element in the heap.
	/// @tparam T Type of data to allocate for.
	/// @return Pointer to start of allocated memory.
	/// @throw AllocationFailure if memory allocation fails.
	template<Type::NonVoid T>
	[[nodiscard, gnu::malloc, gnu::noinline]]
	constexpr owner<T> malloc() {
		return malloc<T>(1);
	}
	/// @brief Reallocates memory allocated in the heap.
	/// @param mem Memory to reallocate.
	/// @param sz New size in bytes.
	/// @return Pointer to new memory location, or `nullptr` if size is zero.
	[[nodiscard, gnu::nonnull(1)]]
	constexpr owner<void> realloc(owner<void> const mem, usize const sz) {
		if (!(sz + 1)) unreachable();
		if (!sz) {
			free(mem);
			return nullptr;
		}
		pointer m = nullptr;
		if (inCompileTime()) {
			free(mem);
			m = malloc(sz);
		} else m = __builtin_realloc(mem, sz);
		if (!m) throw AllocationFailure();
		return m;
	}

	/// @brief Reallocates memory allocated in the heap.
	/// @tparam T Type of data to reallocate.
	/// @param mem Memory to reallocate.
	/// @param sz New count of elements.
	/// @return Pointer to new memory location, or `nullptr` if size is zero.
	template<Type::NonVoid T>
	[[nodiscard, gnu::nonnull(1)]]
	constexpr owner<T> realloc(owner<T> const mem, usize const sz) {
		return static_cast<owner<T>>(realloc(static_cast<pointer>(mem), sz * sizeof(T)));
	}

	/// @brief Destructs an object of a given type, if it exists.
	/// @tparam T Type of object to destruct.
	/// @param val Object to destruct.
	/// @return Pointer to destructed object.
	/// @note Does not delete the underlying memory.
	/// @warning This WILL destruct an object at the given memory location, EVEN IF it is a pointer to constant memory!
	template<Type::NonVoid T>
	[[gnu::nonnull(1)]]
	constexpr ref<T> destruct(ref<T> const val) {
		val->~T();
		return val;
	}

	/// @brief Constructs a given address of memory as a given type, with a list of arguments.
	/// @tparam ...Args Constructor argument types.
	/// @tparam T Type to construct.
	/// @param mem Memory to construct as `T`.
	/// @param ...args Constructor arguments.
	/// @return Pointer to constructed memory.
	/// @throw ConstructionFailure if memory does not exist.
	/// @warning This WILL construct an object at the given memory location, EVEN IF it is a pointer to constant memory!
	template<Type::NonVoid T, typename... Args>
	[[gnu::nonnull(1)]]
	constexpr ref<T> construct(ref<T> const mem, Args&&... args)
	requires (Type::Constructible<T, Args...>) {
		#ifdef CTL_EXPERIMENTAL_COMPILE_TIME_MEMORY
		if (inCompileTime()) std::construct_at(mem, args...);
		else
		#endif
		::new (static_cast<pointer>(mem)) T(::CTL::forward<Args>(args)...);
		return mem;
	}

	/// @brief Destructs a given address of memory of a given type, then constructs it with a list of arguments.
	/// @tparam ...Args Constructor argument types.
	/// @tparam T Type to reconstruct.
	/// @param mem Memory to reconstruct.
	/// @param ...args Constructor arguments.
	/// @return Pointer to reconstructed memory.
	/// @warning This WILL construct an object at the given memory location, EVEN IF it is a pointer to constant memory!
	template<Type::NonVoid T, typename... Args>
	[[gnu::nonnull(1)]]
	constexpr void reconstruct(ref<T> const mem, Args&&... args) {
		destruct(mem);
		construct(mem, ::CTL::forward<Args>(args)...);
	}

	/// @brief Allocates & constructs an object of a given type on the heap.
	/// @tparam ...Args Constructor argument types.
	/// @tparam T Type of object to create.
	/// @param ...args Constructor arguments.
	/// @return Pointer to created object.
	template<Type::NonVoid T, typename... Args>
	[[nodiscard]]
	constexpr owner<T> create(Args&&... args) {
		return construct<T>(malloc<T>(), ::CTL::forward<Args>(args)...);
	}

	/// @brief Resizes memory allocated in the heap.
	/// @param mem Memory to resize.
	/// @param sz New size in bytes.
	constexpr ref<void> resize(ref<void>& mem, usize const sz) {
		return mem = realloc(mem, sz);
	}

	/// @brief Resizes memory allocated in the heap.
	/// @tparam T Type of data to resize.
	/// @param mem Memory to resize.
	/// @param sz New count of elements.
	template<Type::NonVoid T>
	[[gnu::nonnull(1)]]
	constexpr ref<T> resize(ref<T>& mem, usize const sz) {
		return mem = realloc<T>(mem, sz);
	}


	/// @brief Safely copies data from one place to another, respecting the type's constructor.
	/// @tparam T Type of data to copy.
	/// @param dst Destination.
	/// @param src Source.
	/// @param count Count of elements to copy.
	/// @return Pointer to destination.
	template<Type::NonVoid T>
	[[gnu::nonnull(1, 2)]]
	constexpr ref<T> objcopy(ref<T> dst, ref<T const> src, usize sz) {
		if (!(sz + 1)) unreachable();
		if (!sz) return dst;
		T* start = dst;
		#ifdef CTL_EXPERIMENTAL_COMPILE_TIME_MEMORY
		if (inCompileTime())
			while (sz--)
				construct(dst++, *src++);
		else
		#endif
		try {
			if (dst < src) {
				while (sz--) {
					//*(dst++) = *(src++);
					construct(dst++, *src++);
				}
			}
			else {
				dst += sz;
				src += sz;
				start = dst;
				while (sz--) {
					//*(--dst) = *(--src);
					construct(--dst, *--src);
				}
			}
		} catch (...) {
			if (dst < src)
				for (;dst != start; --dst)
					destruct(dst);
			else
				for (;dst != start; ++dst)
					destruct(dst);
		}
		return dst;
	}

	/// @brief Safely clears data, respecting the type's destructor.
	/// @tparam T Type of data to clear.
	/// @param dst Data to clear.
	/// @param count Count of elements to clear.
	/// @return Pointer to destination.
	template<Type::NonVoid T>
	[[gnu::nonnull(1)]]
	constexpr ref<T> objclear(ref<T> mem, usize sz) {
		CTL_DEVMODE_FN_DECL;
		if (!(sz + 1)) unreachable();
		if (!sz) return mem;
		while (sz--)
			destruct<T>(mem++);
		return mem;
	}
}

CTL_NAMESPACE_END

/*
inline pointer operator new(usize sz) noexcept(false)		{return CTL::MX::malloc(sz);	}
inline pointer operator new[](usize sz) noexcept(false)		{return CTL::MX::malloc(sz);	}

inline void operator delete(pointer mem) noexcept(false)	{return CTL::MX::free(mem);		}
inline void operator delete[](pointer mem) noexcept(false)	{return CTL::MX::free(mem);		}
*/

#endif // CTL_MEMORY_CORE_H
