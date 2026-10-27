#ifndef MAKAILIB_ANIMA_V2_MEMORY_MODEL_ALLOCATOR_H
#define MAKAILIB_ANIMA_V2_MEMORY_MODEL_ALLOCATOR_H

#include "../../../../compat/ctl.hpp"

namespace Makai::Anima::V2::Runtime {
	namspace Impl {
		struct BlockAllocator {
			BlockAllocator(usize const blockSize);

			~BlockAllocator();

			struct Block {
				MemorySlice<byte>	memory;
				usize				used, free;
				usize				refs;
				Mutex				lock;

				constexpr bool contains(ref<byte const> const ptr) {return ptr >= memory.data() && ptr < memory.data() + memory.size();}
			};

			struct Section {
				ref<Block>	block;
				pointer		start;
				usize		size;
				Mutex		lock;
			};

			LinkedList<Block>	blocks;
			LinkedList<Section>	free;

			pointer	allocate(usize const sz);
			void	deallocate(pointer const at, usize const sz);

		private:
			void collect();

			pointer newBlock(usize const sz, usize const preused);

			usize const blockSize;

			Mutex lock;
		};
	}

	template <class TData>
	struct BlockAllocator {
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
		Impl::BlockAllocator alloc(2u << 20);
	};

	template <class T>
	using Allocator = GSAllocator<T, BlockAllocator>;
}

#endif
