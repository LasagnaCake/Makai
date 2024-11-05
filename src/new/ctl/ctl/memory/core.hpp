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

// One day... one day these will work without builtins...

CTL_NAMESPACE_BEGIN

/// @brief Memory eXtensions.
namespace MX {
	/// @brief Underlying address type.
	typedef uint8* Address;

	/// @brief Copies data from one place to another, byte-by-byte.
	/// @param dst Destination.
	/// @param src Source.
	/// @param size Size in bytes.
	/// @return Pointer to destination.
	constexpr pointer memcpy(pointer const& dst, void const* const& src, usize size) {
		#ifdef CTL_DO_NOT_USE_BUILTINS
		Address s = (Address)src, d = (Address)dst;
		if (size == 1) *d = *s;
		else while (size--) *d++ = *s++;
		#else
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
	constexpr T* memcpy(T* const& dst, T const* const& src, usize const& count) {
		return (T*)memcpy((pointer)dst, (pointer)src, count * sizeof(T));
	}

	/// @brief Copies data from one place to another, byte-by-byte.
	/// @tparam T Type of data to copy.
	/// @param dst Destination.
	/// @param src Source.
	/// @return Pointer to destination.
	template<Type::NonVoid T>
	constexpr T* memcpy(T* const& dst, T const* const& src) {
		return (T*)memcpy((pointer)dst, (pointer)src, sizeof(T));
	}

	/// @brief Copies data from one place to another, byte-by-byte, while respecting memory layout.
	/// @param dst Destination.
	/// @param src Source.
	/// @param size Size in bytes.
	/// @return Pointer to destination.
	constexpr pointer memmove(pointer const& dst, void const* const& src, usize size) {
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
		return __builtin_memmove(dst, src, size);
		#endif // CTL_DO_NOT_USE_BUILTINS
	}

	/// @brief Copies data from one place to another, byte-by-byte, while respecting memory layout.
	/// @tparam T Type of data to copy.
	/// @param dst Destination.
	/// @param src Source.
	/// @param count Count of elements to copy.
	/// @return Pointer to destination.
	template<Type::NonVoid T>
	constexpr T* memmove(T* const& dst, T const* const& src, usize const& count) {
		return (T*)memmove((pointer)dst, (pointer)src, count * sizeof(T));
	}

	/// @brief Copies data from one place to another, byte-by-byte, while respecting memory layout.
	/// @tparam T Type of data to copy.
	/// @param dst Destination.
	/// @param src Source.
	/// @return Pointer to destination.
	template<Type::NonVoid T>
	constexpr T* memmove(T* const& dst, T const* const& src) {
		return (T*)memmove((pointer)dst, (pointer)src, sizeof(T));
	}

	/// @brief Compares two spans of data, byte-by-byte.
	/// @param a Data to compare.
	/// @param b Data to compare with.
	/// @param size Size in bytes.
	/// @return Order between both spans of data.
	constexpr int memcmp(void const* const& a, void const* const& b, usize size) {
		#ifdef CTL_DO_NOT_USE_BUILTINS
		Address s1 = (Address)a, s2 = (Address)b;
		while (size-- > 0)
			if (*s1++ != *s2++)
				return s1[-1] < s2[-1] ? -1 : 1;
		#else
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
	constexpr int memcmp(T const* const& a, T const* const& b, usize const& count) {
		return memcmp((pointer)a, (pointer)b, count * sizeof(T));
	}

	/// @brief Compares two spans of data, byte-by-byte.
	/// @tparam T Type of data to compare.
	/// @param a Data to compare.
	/// @param b Data to compare with.
	/// @return Order between both spans of data.
	template<Type::NonVoid T>
	constexpr int memcmp(T const* const& a, T const* const& b) {
		return memcmp(a, b, 1);
	}

	/// @brief Sets every byte of data to a value.
	/// @param dst Data to set.
	/// @param val Value to set each byte.
	/// @param size Size in bytes.
	/// @return Pointer to data.
	constexpr pointer memset(pointer const& dst, int const& val, usize size) {
		#ifdef CTL_DO_NOT_USE_BUILTINS
		Address d = (Address)dst;
		while (size-- > 0)
			*d++ = val;
		#else
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
	constexpr T* memset(T* const& dst, int const& val, usize const& count) {
		return (T*)memset((pointer)dst, val, count * sizeof(T));
	}

	/// @brief Sets every byte of data to a value.
	/// @tparam T Type of data to set.
	/// @param dst Data to set.
	/// @param val Value to set each byte.
	/// @return Pointer to data.
	template<Type::NonVoid T>
	constexpr T* memset(T* const& dst, int const& val) {
		return (T*)memset((pointer)dst, val, sizeof(T));
	}
	
	/// @brief Sets every byte of data to zero.
	/// @param dst Data to set.
	/// @param size Size in bytes.
	/// @return Pointer to data.
	constexpr pointer memzero(pointer const& dst, usize const& size) {
		return memset(dst, 0, size);
	}

	/// @brief Sets every byte of data to zero.
	/// @tparam T Type of data to zero.
	/// @param dst Data to set.
	/// @param count Count of elements to zero.
	/// @return Pointer to data.
	template<Type::NonVoid T>
	constexpr T* memzero(T* const& dst, usize const& count) {
		return (T*)memzero((pointer)dst, count * sizeof(T));
	}

	/// @brief Sets every byte of data to zero.
	/// @tparam T Type of data to zero out.
	/// @param dst Data to set.
	/// @return Pointer to data.
	template<Type::NonVoid T>
	constexpr T* memzero(T* const& dst) {
		return (T*)memzero((pointer)dst, sizeof(T));
	}
	
	/// @brief Allocates space for a given size of bytes in the heap.
	/// @param sz Byte size.
	/// @return Pointer to start of allocated memory.
	/// @throw AllocationFailure if size is zero, or memory allocation fails.
	constexpr pointer malloc(usize const& sz) {
		if (!sz) throw AllocationFailure();
		auto* m = __builtin_malloc(sz);
		if (!m) throw AllocationFailure();
		return m;
	}

	/// @brief Allocates space for a given count of elements in the heap.
	/// @tparam T Type of data to allocate for.
	/// @param sz Count of elements to allocate.
	/// @return Pointer to start of allocated memory.
	/// @throw AllocationFailure if size is zero, or memory allocation fails.
	template<Type::NonVoid T>
	constexpr T* malloc(usize const& sz) {
		if (!sz) throw AllocationFailure();
		auto* m = __builtin_malloc(sz * sizeof(T));
		if (!m) throw AllocationFailure();
		//memzero(m, sz);
		return (T*)m;
	}
	
	/// @brief Allocates space for an element in the heap.
	/// @tparam T Type of data to allocate for.
	/// @return Pointer to start of allocated memory.
	/// @throw AllocationFailure if memory allocation fails.
	template<Type::NonVoid T>
	constexpr T* malloc() {
		return (T*)malloc<T>(1);
	}

	/// @brief Frees memory allocated in the heap.
	/// @param mem Pointer to allocated memory.
	constexpr void free(pointer const& mem) {
		if (mem) return __builtin_free(mem);
	}

	/// @brief Frees memory allocated in the heap.
	/// @tparam T Type of data allocated.
	/// @param mem Pointer to allocated memory.
	template<Type::NonVoid T>
	constexpr void free(T* const& mem) {
		if (mem) return __builtin_free((pointer)mem);
	}

	/// @brief Reallocates memory allocated in the heap.
	/// @param mem Memory to reallocate.
	/// @param sz New size in bytes.
	/// @return Pointer to new memory location.
	constexpr pointer realloc(pointer const& mem, usize const& sz) {
		if (!sz) {
			free(mem);
			return nullptr;
		}
		auto* m = __builtin_realloc(mem, sz);
		if (!m) throw AllocationFailure();
		return m;
	}

	/// @brief Reallocates memory allocated in the heap.
	/// @tparam T Type of data to reallocate.
	/// @param mem Memory to reallocate.
	/// @param sz New count of elements.
	/// @return Pointer to new memory location.
	template<Type::NonVoid T>
	constexpr T* realloc(T* const& mem, usize const& sz) {
		if (!sz) {
			free(mem);
			return nullptr;
		}
		auto* m = __builtin_realloc((pointer)mem, sz * sizeof(T));
		if (!m) throw AllocationFailure();
		return (T*)m;
	}

	/// @brief Destructs an object of a given type.
	/// @tparam T Type of object to destruct.
	/// @param val Object to destruct.
	/// @note Does not delete the underlying memory.
	template<Type::NonVoid T>
	constexpr void destruct(T* const& val) {
		if (!val) return;
		val->~T();
	}

	/// @brief Constructs a given address of memory as a given type, with a list of arguments.
	/// @tparam ...Args Constructor argument types.
	/// @tparam T Type to construct.
	/// @param mem Memory to construct as `T`.
	/// @param ...args Constructor arguments.
	/// @return Pointer to constructed memory.
	template<Type::NonVoid T, typename... Args>
	constexpr T* construct(T* const& mem, Args... args) {
		if (!mem) throw ConstructionFailure();
		::new (static_cast<void*>(mem)) T(args...);
		return mem;
	}

	/// @brief Destructs a given address of memory of a given type, then constructs it with a list of arguments.
	/// @tparam ...Args Constructor argument types.
	/// @tparam T Type to reconstruct.
	/// @param mem Memory to reconstruct.
	/// @param ...args Constructor arguments.
	/// @return Pointer to reconstructed memory.
	template<Type::NonVoid T, typename... Args>
	constexpr void reconstruct(T* const& mem, Args... args) {
		destruct(mem);
		construct(mem, args...);
	}

	/// @brief Allocates & constructs an object of a given type on the heap.
	/// @tparam ...Args Constructor argument types.
	/// @tparam T Type of object to create.
	/// @param ...args Constructor arguments.
	/// @return Pointer to created object.
	template<Type::NonVoid T, typename... Args>
	constexpr T* create(Args... args) {
		return construct<T>(malloc<T>(), args...);
	}

	/// @brief Resizes memory allocated in the heap.
	/// @param mem Memory to resize.
	/// @param sz New size in bytes.
	constexpr pointer resize(pointer& mem, usize const& sz) {
		return mem = realloc(mem, sz);
	}

	/// @brief Resizes memory allocated in the heap.
	/// @tparam T Type of data to resize.
	/// @param mem Memory to resize.
	/// @param sz New count of elements.
	template<Type::NonVoid T>
	constexpr T* resize(T*& mem, usize const& sz) {
		return mem = realloc<T>(mem, sz);
	}

	
	/// @brief Safely copies data from one place to another, respecting the type's constructor.
	/// @tparam T Type of data to copy.
	/// @param dst Destination.
	/// @param src Source.
	/// @param count Count of elements to copy.
	/// @return Pointer to destination.
	template<Type::NonVoid T>
	constexpr T* objcopy(T* dst, T const* src, usize sz) {
		T* start = dst;
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
	constexpr T* objclear(T* mem, usize sz) {
		while (sz--)
			destruct<T>(mem++);
		return dst;
	}
}

CTL_NAMESPACE_END

inline pointer operator new(usize sz) noexcept(false)		{return CTL::MX::malloc(sz);	}
inline pointer operator new[](usize sz) noexcept(false)		{return CTL::MX::malloc(sz);	}

inline void operator delete(pointer mem) noexcept(false)	{return CTL::MX::free(mem);		}
inline void operator delete[](pointer mem) noexcept(false)	{return CTL::MX::free(mem);		}

#endif // CTL_MEMORY_CORE_H
