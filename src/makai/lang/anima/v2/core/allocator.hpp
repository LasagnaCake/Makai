#ifndef MAKAILIB_ANIMA_V2_CORE_ALLOCATOR_H
#define MAKAILIB_ANIMA_V2_CORE_ALLOCATOR_H

#include "../../../../compat/ctl.hpp"

namespace Makai::Anima::V2::Core {
	struct Allocator {
		Allocator();
		~Allocator();

		owner<byte> allocate(usize const sz);
		void deallocate(owner<byte> const mem, usize const sz);

	private:
		static PagedAllocator<byte> memory;
	};
}

#endif
