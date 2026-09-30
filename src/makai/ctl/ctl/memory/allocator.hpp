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
			{t.allocate(sz)}		-> Type::Equal<owner<TData>>	;
			{t.deallocate(p, sz)}	-> Type::Equal<void>			;
		};

		/// @brief Type must be a valid constant allocator for `TData`.
		template<template <class> class T, class TData>
		concept ConstantAllocator = requires (T<TData> t, usize sz, owner<TData> p) {
			{t.allocate(sz)}		-> Type::Equal<owner<TData>>	;
			{t.deallocate(p, sz)}	-> Type::Equal<void>			;
		};
	}
}

/// @brief Default allocator. Allocates from the process's own heap.
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

constexpr usize const ONE_KIBIBYTE = usize(1) << 10;
constexpr usize const ONE_MIBIBYTE = usize(1) << 20;
constexpr usize const ONE_GIBIBYTE = usize(1) << 40;

namespace Impl::Memory {
	struct Page {
		inline ref<Page> top() {
			if (!next) return this;
			return next->top();
		}

		inline void attach(ref<Page> const tail) {
			if (!next) next = tail;
		}

		bool contains(pointer const addr) const {
			CTL_DIAGBLOCK_BEGIN;
			_Pragma("GCC diagnostic ignored \"-Wpointer-arith\"");
			return memory <= addr && addr <= (memory + free + used);
			CTL_DIAGBLOCK_END;
		}

		owner<Page>	next;
		pointer		memory;
		usize		free;
		usize		used;

		static owner<Page> create(usize const sz) {
			owner<Page> page = owner<Page>(MX::malloc(sizeof(Page)));
			MX::memzero(page);
			page->memory = MX::malloc(sz);
			MX::memzero(page->memory, sz);
			page->free = sz;
			return page;
		}

		static void destroy(owner<Page> const page) {
			MX::free(page->memory);
			MX::free(page);
		}
	};

	struct Section {
		inline ref<Section> back() {
			if (!prev) return this;
			return prev->back();
		}

		inline void push(ref<Section> const head) {
			if (!prev) prev = head;
		}

		static owner<Section> create() {
			owner<Section> section = owner<Section>(MX::malloc(sizeof(Page)));
			MX::memzero(section);
			return section;
		}

		static void destroy(owner<Section> const section) {
			MX::free(section);
		}

		owner<Section>	prev;
		ref<Page>		page;
		pointer			start;
		usize			size;
	};

	template<
		usize PAGE_SIZE		= ONE_MIBIBYTE/*,
		usize PAGE_COUNT	= 1024,
		usize SECTION_COUNT	= 1024 */
	>
	struct PagedAllocator {
		using Page		= Impl::Memory::Page;
		using Section	= Impl::Memory::Section;

		PagedAllocator() {
			pages = Page::create(PAGE_SIZE);
			free = Section::create();
			free->start = pages->memory;
			free->size = PAGE_SIZE;
			free->page = pages;
		}

		~PagedAllocator() {
			auto page = pages;
			while (page) {
				auto prev = page;
				page = page->next;
				Page::destroy(prev);
			}
			if (page) Page::destroy(page);
			auto section = free;
			while (section) {
				auto next = section;
				section = section->prev;
				Section::destroy(next);
			}
			if (section) Section::destroy(section);
		}

		[[nodiscard, gnu::malloc, gnu::noinline, gnu::nonnull(1)]]
		pointer allocate(usize const sz) {
			if (!sz) return nullptr;
			auto pageSize = PAGE_SIZE;
			auto prevSection	= free;
			auto section		= free;
			while (section && section->size < sz) {
				if (!section->size && section != free) {
					prevSection->prev = section->prev;
					Section::destroy(section);
					section = prevSection->prev;
					continue;
				}
				prevSection = section;
				section = prevSection->prev;
			}
			if (section) {
				auto const mem = section->start;
				section->start = section->start + sz;
				section->size -= sz;
				return mem;
			}
			auto const top = pages->top();
			auto page = pages;
			while (page && page->free < sz)
				page = page->next;
			if (page) {
				auto const mem = page->memory + page->used;
				page->used += sz;
				page->free -= sz;
				return mem;
			}
			if (sz > pageSize)
				while (pageSize < sz) pageSize <<= 2;
			auto newPage = Page::create(pageSize);
			newPage->free = pageSize;
			newPage->used = 0;
			top->attach(newPage);
			auto const mem = top->memory + page->used;
			top->used += sz;
			top->free -= sz;
			return mem;
		}

		[[gnu::noinline, gnu::nonnull(2)]]
		void deallocate(pointer const mem, usize const sz) {
			auto page = pages;
			auto nextPage = page;
			while (page && !page->contains(mem)) {
				if (!page->used && page != pages) {
					nextPage->next = page->next;
					Page::destroy(page);
					page = nextPage->next;
					continue;
				}
				nextPage = page;
				page = nextPage->next;
			}
			if (!page) return;
			auto const newSection = Section::create();
			newSection->page = page;
			newSection->start = mem;
			newSection->size = sz;
			if (free)
				newSection->prev = free;
			free = newSection;
		}

		owner<Page>		pages	= nullptr;
		owner<Section>	free	= nullptr;
	};
}

/// @brief Paged allocator.
/// @tparam TData Type to handle memory for.
template<
	Type::NonVoid TData,
	usize PAGE_SIZE		= ONE_MIBIBYTE/*,
	usize PAGE_COUNT	= 1024,
	usize SECTION_COUNT	= 1024 */
>
struct PagedAllocator {
	using DataType = TData;

	[[nodiscard, gnu::malloc, gnu::noinline, gnu::nonnull(1)]]
	owner<DataType> allocate(usize const sz) {
		return (owner<DataType>)alloc.allocate(sz * sizeof(DataType));
	}

	[[gnu::noinline, gnu::nonnull(2)]]
	void deallocate(owner<DataType> const mem, usize const sz) {
		return alloc.deallocate((pointer)mem, sz * sizeof(DataType));
	}

private:
	Impl::Memory::PagedAllocator<PAGE_SIZE> alloc;
};

namespace Impl {
	template <class TGroup, template <class> class TAlloc>
	struct SharedAllocator {
		static auto& memory() {
			static TAlloc<byte> alloc;
			return alloc;
		}
	};
}

/// @brief "Globally-Shared" allocator.
/// @tparam TData Type to handle memory for.
/// @tparam TGroup Storage group. By default, it is `void`.
/// @tparam TAlloc Allocator type. By default, it is `HeapAllocator`.
template <class TData, class TGroup = void, template <class> class TAlloc = HeapAllocator>
struct GSAllocator: private Impl::SharedAllocator<TGroup, TAlloc> {
	using DataType = TData;

	owner<DataType> allocate(usize const sz) {
		return (owner<DataType>)memory().allocate(sz * sizeof(DataType));
	}

	void deallocate(owner<DataType> const mem, usize const sz) {
		memory().deallocate((owner<byte>)mem, sz * sizeof(DataType));
	}

private:
	using Impl::SharedAllocator<TGroup, TAlloc>::memory;
};

template <class T> using DefaultAllocator = GSAllocator<T>;

CTL_NAMESPACE_END

#endif // CTL_MEMORY_ALLOCATOR_H
