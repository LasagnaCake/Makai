#include "allocator.hpp"

using namespace Makai::Anima::V2::Runtime;

Impl::BlockAllocator::BlockAllocator(usize const blockSize): blockSize(blockSize) {
	newBlock(blockSize);
}

Impl::BlockAllocator::~BlockAllocator() {
}

pointer Impl::BlockAllocator::allocate(usize const sz) {
	if (!sz) return nullptr;
	lock.wait();
	auto newBlockSize = blockSize;
	while (newBlockSize < sz)
		newBlockSize <<= 1;
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

pointer Impl::BlockAllocator::newBlock(usize const size, usize const preused) {
	auto& block = blocks.pushBack(Block(size, preused)).back();
	block.refs = 1;
	return block.memory.data();
}

void Impl::BlockAllocator::deallocate(pointer const at, usize const sz) {
	lock.wait();
	for (auto& block: blocks)
		if (block.contains((ref<byte const>)at)) {
			free.pushBack({
				&block,
				(ref<byte>)at,
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
	for (auto const& [section, index]: Range::expand(free)) {
		if (!section.block->refs) {
			cleanup.pushBack(index - count);
			++count;
		}
	}
	for (auto& index: cleanup)
		free.erase(free.at(index));
	count = 0;
	cleanup.clear();
	for (auto const& [block, index]: Range::expand(blocks)) {
		if (!block.refs) {
			cleanup.pushBack(index - count);
			++count;
		}
	}
	for (auto& index: cleanup)
		blocks.erase(index);
}
