#ifndef CTL_MEMORY_MEMORYSLICE_H
#define CTL_MEMORY_MEMORYSLICE_H

#include "allocator.hpp"
#include "../algorithm/transform.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Automatically-managed memory slice.
/// @tparam TData Type to handle memory for.
/// @tparam TAlloc Runtime allocator to use.
template<
	typename TData						= void,
	template <class> class TAlloc		= HeapAllocator,
	template <class> class TConstAlloc	= ConstantAllocator
>
struct MemorySlice:
	Typed<TData>,
	SelfIdentified<MemorySlice<TData, TAlloc>>,
	ContextAwareAllocatable<TData, TAlloc> {
	using SelfIdentified			= ::CTL::SelfIdentified<MemorySlice<TData, TAlloc, TConstAlloc>>;
	using ContextAwareAllocatable	= ::CTL::ContextAwareAllocatable<TData, TAlloc, TConstAlloc>;
	using Typed						= ::CTL::Typed<TData>;

	using
		typename ContextAwareAllocatable::ContextAllocatorType
	;

	using typename SelfIdentified::SelfType;

	using
		typename Typed::DataType,
		typename Typed::PointerType,
		typename Typed::ConstPointerType,
		typename Typed::ReferenceType,
		typename Typed::ConstReferenceType
	;

	using CopyFunctionType = void(ref<DataType> const, ref<DataType const> const);

	constexpr static auto NO_COPY_FN = [] (ref<DataType> const, ref<DataType const> const) {};

	/// @brief Default constructor.
	constexpr MemorySlice()					{				}
	/// @brief Constructs the memory slice with space for a number of elements.
	/// @param sz Element count to allocate for.
	constexpr MemorySlice(usize const sz)	{invoke(sz);	}

	/// @brief Copy constructor (deleted).
	constexpr MemorySlice(SelfType const& other) = delete;

	/// @brief Move constructor (`MemorySlice`).
	/// @param other `MemorySlice` to move.
	constexpr MemorySlice(SelfType&& other):
		contents(::CTL::move(other.contents)),
		length(::CTL::move(other.length)) {
		other.contents = nullptr;
	}
	
	/// @brief Copy assignment operator (deleted).
	constexpr SelfType& operator=(SelfType const& other) = delete;

	/// @brief Move assignment operator (`MemorySlice`).
	/// @param other `MemorySlice` to move.
	constexpr SelfType& operator=(SelfType&& other) {
		free();
		contents	= ::CTL::move(other.contents);
		length		= ::CTL::move(other.length);
		other.contents = nullptr;
		return *this;
	}

	/// @brief `swap` algorithm for `MemorySlice`.
	/// @param a `MemorySlice` to swap.
	/// @param b `MemorySlice` to swap with.
	friend constexpr void swap(SelfType& a, SelfType& b) noexcept {
		swap(a.contents, b.contents);
		swap(a.length, b.length);
		swap(a.alloc, b.alloc);
	}

	/// @brief Destructor.
	constexpr ~MemorySlice() {free();}

	/// @brief Returns whether the memory slice is empty.
	/// @return Whether memory slice is empty.
	constexpr bool empty() const			{return !length;						}
	/// @brief Returns the maximum element count of the memory slice.
	/// @return Maxumum element count of memory slice.
	constexpr usize size() const			{return length;							}
	/// @brief Returns the size (in bytes) of the memory slice.
	/// @return Size of the memory slice.
	constexpr usize byteSize() const		{return length * sizeof (DataType);		}
	/// @brief Returns a pointer to the start of the memory slice.
	/// @return Pointer to start of memory slice.
	constexpr PointerType data() 			{return contents;						}
	/// @brief Returns a pointer to the start of the memory slice.
	/// @return Pointer to start of memory slice.
	constexpr ConstPointerType data() const	{return contents;						}


	/// @brief Gets the element at the given index.
	/// @param index Element index.
	/// @return Element at given index.
	/// @note Index wraps around if it is bigger than the current size.
	constexpr ReferenceType operator[](usize const index) {
		return contents[index % length];
	}

	/// @brief Gets the element at the given index.
	/// @param index Element index.
	/// @return Element at given index.
	/// @note Index wraps around if it is bigger than the current size.
	constexpr ConstReferenceType operator[](usize const index) const {
		return contents[index % length];
	}

	/// @brief Allocates (or resizes) the memory slice.
	/// @param sz Element count.
	/// @return Reference to self.
	constexpr SelfType& invoke(usize const sz) {
		//CTL_DEVMODE_FN_DECL;
		if (!sz) return *this;
		resize(sz);
		return *this;
	}

	/// @brief Allocates the memory slice.
	/// @param sz Element count.
	/// @return Reference to self.
	constexpr SelfType& create(usize const sz) {
		CTL_DEVMODE_FN_DECL;
		if (!sz || contents) return *this;
		contents = alloc.allocate(sz);
		length = sz;
		return *this;
	}

	/// @brief Resizes the memory slice.
	/// @param sz Element count.
	/// @return Reference to self.
	constexpr SelfType& resize(usize const newSize) {
		//CTL_DEVMODE_FN_DECL;
		if (!newSize) return free();
		if (!contents) return create(newSize);
		alloc.deallocate(contents, length);
		contents = alloc.allocate(newSize);
		length = newSize;
		return *this;
	}

	/// @brief Frees the memory managed by the slice.
	/// @return Reference to self.
	constexpr SelfType& free() {
		// Adding this debug line makes it stop fucking up memory somehow
		//CTL_DEVMODE_FN_DECL;
		if (!contents) return *this;
		alloc.deallocate(contents, length);
		contents	= nullptr;
		length		= 0;
		return *this;
	}

	/// @brief Returns the associated allocator.
	/// @return Allocator.
	constexpr ContextAllocatorType allocator() const {return alloc;}

private:
	/// @brief Memory allocator.
	ContextAllocatorType	alloc;
	/// @brief Managed memory.
	PointerType				contents	= nullptr;
	/// @brief Element count.
	usize					length		= 0;
};

CTL_NAMESPACE_END

#endif