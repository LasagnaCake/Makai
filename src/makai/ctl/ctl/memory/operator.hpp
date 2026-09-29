#ifndef CTL_MEMORY_OPERATOR_H
#define CTL_MEMORY_OPERATOR_H

#include "allocator.hpp"

CTL_NAMESPACE_BEGIN

namespace MX {
	struct OperatorNewDelete {
		inline static GSPAllocator<void> alloc;
	};
}

CTL_NAMESPACE_END

inline pointer operator new(usize const count) {
	return CTL::MX::OperatorNewDelete::alloc.allocate(count);
}

inline pointer operator new[](usize const count) {
	return CTL::MX::OperatorNewDelete::alloc.allocate(count);
}

inline void operator delete(pointer const mem, usize const count) noexcept {
	return CTL::MX::OperatorNewDelete::alloc.deallocate(mem, count);
}

inline void operator delete[](pointer const mem, usize const count) noexcept {
	return CTL::MX::OperatorNewDelete::alloc.deallocate(mem, count);
}

#endif
