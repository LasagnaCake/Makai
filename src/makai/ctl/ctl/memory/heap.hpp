#ifndef CTL_MEMORY_HEAP_H
#define CTL_MEMORY_HEAP_H

#include "../namespace.hpp"
#include "../ctypes.hpp"
#include "../typetraits/traits.hpp"
#include "../templates.hpp"
#include "../cppfailure.hpp"

#ifdef CTL_ON_WINDOWS
#include <heapapi.h>
#else
#include <sys/mman.h>
#endif

CTL_NAMESPACE_BEGIN

/// @brief Memory heap.
struct Heap {
	/// @brief Memory heap size.
	struct Size {
		usize const max		= 0;
		usize const start	= 0;
	};

	/// @brief Heap flags.
	struct Flags {
		bool executable		= false;
		bool synchronized	= false;
		bool readonly		= false;
	};

	/// @brief Empty constructor.
	Heap() {}

	Heap(Size const sz, Flags flags) {open(sz, flags);}

	~Heap() {close();}

	Heap(Heap const&)	= delete;
	Heap(Heap&&)		= default;

	Heap& operator=(Heap const&)	= delete;
	Heap& operator=(Heap&&)			= default;

	void open(Size const sz, Flags flags) {
		synchronize = flags.synchronized;
		#ifdef CTL_ON_WINDOWS
		impl = HeapCreate(
			flags.executable ? HEAP_CREATE_ENABLE_EXECUTE : 0,
			sz.start,
			sz.max
		);
		if (!impl)
			throw HeapCreationFailure();
		#else
		#endif
	}

	pointer allocate(usize const sz) const {
		#ifdef CTL_ON_WINDOWS
		if (synchronize) {
			HeapLock(impl);
			auto const p = HeapAlloc(impl, HEAP_ZERO_MEMORY, sz);
			HeapUnlock(impl);
			return p;
		} else return HeapAlloc(impl, HEAP_ZERO_MEMORY, sz);
		#else
		#endif
	}

	pointer reallocate(pointer const at, usize const sz) const {
		#ifdef CTL_ON_WINDOWS
		if (synchronize) {
			HeapLock(impl);
			auto const p = HeapReAlloc(impl, HEAP_ZERO_MEMORY, at, sz);
			HeapUnlock(impl);
			return p;
		} else return HeapReAlloc(impl, HEAP_ZERO_MEMORY, at, sz);
		#else
		#endif
	}

	void free(pointer const at) const {
		#ifdef CTL_ON_WINDOWS
		if (synchronize) {
			HeapLock(impl);
			if (at) HeapFree(impl, 0, at);
			HeapUnlock(impl);
		} else if (at) HeapFree(impl, 0, at);
		#else
		#endif
	}

	void close() {
		#ifdef CTL_ON_WINDOWS
		if (impl && impl != GetProcessHeap())
			HeapDestroy(impl);
		impl = nullptr;
		#else
		#endif
	}

	static Heap& getDefault() {
		static Heap heap = [] () -> Heap {
			Heap heap;
			heap.impl = GetProcessHeap();
			heap.synchronize = true;
			return heap;
		} ();
		return heap;
	}

private:
	#ifdef CTL_ON_WINDOWS
	HANDLE impl = nullptr;
	#else
	#endif

	bool synchronize = false;
};

struct DefaultHeap {
	Heap& getHeap() const {return Heap::getDefault();};
};

namespace Type {
	/// @brief Memory-specific type constraints.
	namespace Memory {
		/// @brief Type must provide a heap.
		template <class T>
		concept HeapProvider = requires (T t) {
			{t.getHeap()} -> Equal<Heap&>;
		};
	}
}

CTL_NAMESPACE_END

#endif // CTL_MEMORY_HEAP_H
