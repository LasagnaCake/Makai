#ifndef MAKAILIB_ANIMA_V2_CORE_ALLOCATOR_H
#define MAKAILIB_ANIMA_V2_CORE_ALLOCATOR_H

#include "../../../../compat/ctl.hpp"

namespace Makai::Anima::V2::Core {
	template <class T>
	struct Allocator {
		Allocator() {}

		~Allocator() {}

		owner<T> allocate(usize const sz) {
			return memory.allocate(sz);
		}

		void deallocate(owner<T> const mem, usize const sz) {
			memory.deallocate(mem, sz);
		}

	private:
		inline static PagedAllocator<T> memory{ONE_MIBIBYTE};
	};
}

#endif
