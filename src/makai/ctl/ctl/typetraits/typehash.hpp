#ifndef CTL_TYPETRAITS_TYPEHASH_H
#define CTL_TYPETRAITS_TYPEHASH_H

#include "nameof.hpp"
#include "../algorithm/hash.hpp"
#include "../order.hpp"

CTL_NAMESPACE_BEGIN;

struct TypeHash: Ordered {
	constexpr static usize const HASH_SIZE = 4;
	carr<usize, HASH_SIZE> hash;

	template <class T>
	consteval static TypeHash forType() {
		constexpr auto const name = nameof<T>();
		constexpr usize const id = (sizeof(T) << 32) ^ (alignof(T) << 16) ^ (nameof<T>().size());
		usize garbage = 0;
		for (auto& c: name) garbage += c;
		constexpr As<usize const[]> jank = {
			ConstHasher::hash(nameof<ref<void>>()),
			ConstHasher::hash(nameof<ref<bool>>()),
			ConstHasher::hash(nameof<ref<int8>>()),
			ConstHasher::hash(nameof<ref<uint8>>()),
			ConstHasher::hash(nameof<ref<int16>>()),
			ConstHasher::hash(nameof<ref<uint16>>()),
			ConstHasher::hash(nameof<ref<int32>>()),
			ConstHasher::hash(nameof<ref<uint32>>()),
			ConstHasher::hash(nameof<ref<int64>>()),
			ConstHasher::hash(nameof<ref<uint64>>()),
			ConstHasher::hash(nameof<ref<float32>>()),
			ConstHasher::hash(nameof<ref<float64>>()),
			ConstHasher::hash(nameof<ref<float128>>()),
			ConstHasher::hash(nameof<ref<pointer>>()),
			ConstHasher::hash(nameof<ref<nulltype>>())
		};
		return {
			.hash = {
				ConstHasher::hash(name),
				garbage,
				id * sizeof(jank),
				ConstHasher::hash(name) * jank[id % (sizeof(jank) / sizeof(usize))]
			}
		};
	}

	constexpr OrderType operator<=>(TypeHash const& other) const {
		OrderType order = Order::EQUAL;
		for (usize i = 0; i < HASH_SIZE; ++i)
			if ((order = hash[i] <=> other.hash[i]) != Order::EQUAL)
				return order;
		return Order::EQUAL;
	}

	constexpr bool operator==(TypeHash const& other) const {
		for (usize i = 0; i < HASH_SIZE; ++i)
			if (hash[i] != other.hash[i])
				return false;
		return true;
	}
};

static_assert(TypeHash::forType<int8>() != TypeHash::forType<uint8>());

CTL_NAMESPACE_END

#endif
