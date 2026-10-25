#include "allocator.cpp"

using namespace Makai::Anima::V2::Runtime;

Impl::BlockAllocator::BlockAllocator(usize const blockSize): blockSize(blockSize) {
	blocks.pushBack({
		MemorySlice<byte>(blockSize),
		0,
		blockSize,
		1
	});
}

Impl::BlockAllocator::~BlockAllocator() {
}

pointer Impl::BlockAllocator::allocate(usize const sz) {
	if (!sz) return nullptr;
	lock.wait();
	auto newBlockSize = blockSize;
	while (newBlockSize < sz)
		newBlockSize << 1;
	if (newBlockSize != blockSize)
		return newBlock(newBlockSize, sz);
	for (auto& section: free)
		if (sz < section.size) {
			auto const _ = Makai::lock(section.block->lock);
			auto const sec = section.start;
			section.start += sz;
			section.size -= sz;
			++section.block->refs;
			return sec;
		}
	for (auto& block: blocks)
		if (sz < block.free) {
			auto const _ = Makai::lock(block.lock);
			auto const sec = block.memory.data() + block.used;
			block.used += sz;
			block.free -= sz;
			++block.refs;
			return sec;
		}
	return newBlock(newBlockSize, sz);
}

void Impl::BlockAllocator::newBlock(usize const size, usize const preused) {
	blocks.pushBack({
		MemorySlice<byte>(size),
		preused,
		size - preused,
		1
	});
	return blocks.back().memory.data();
}

void Impl::BlockAllocator::deallocate(pointer const at, usize const sz) {
	lock.wait();
	for (auto& block: blocks)
		if (block.contains(at)) {
			free.pushBack({
				&block,
				at,
				sz
			});
			--block.refs;
		}
	collect();
}

void Impl::BlockAllocator::collect() {
	auto const _ = Makai::lock(lock);
	usize count = 0;
	List<usize> cleanup;
	for (auto const& [section, index]: Range::expand(sections)) {
		if (!section.block->refs) {
			cleanup.pushBack(index - count);
			++count;
		}
	}
	for (auto& index: cleanup)
		sections.erase(sections.at(index));
	count = 0;
	cleanup.clear();
	for (auto const& [block, index]: Range::expand(blocks)) {
		if (!block.refs) {
			cleanup.pushBack(index - count);
			++count;
		}
	}
	for (auto& index: cleanup)
		block.erase(block.at(index));
}
