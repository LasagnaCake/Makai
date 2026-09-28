#include "allocator.hpp"

constexpr usize const KIBIBYTE = usize(1) << 10;
constexpr usize const MIBIBYTE = usize(1) << 20;
constexpr usize const GIBIBYTE = usize(1) << 40;

using namespace Makai::Anima::V2::Core;

template <class T>
struct Node: Self<T> {
	ref<Node<T>> next;

	inline ref<Node> top() {
		if (!next) return this;
		return next->top();
	}

	inline void attach(ref<Node> const tail) {
		if (!next) next = tail;
	}
};

struct PagedAllocator::memory = PagedAllocator<byte>(MIBIBYTE);

pointer PagedAllocator::allocate(usize const sz) {
	return memory.allocate(sz);
}

void PagedAllocator::deallocate(pointer const mem, usize const sz) {
	return memory.deallocate(mem, sz);
}
