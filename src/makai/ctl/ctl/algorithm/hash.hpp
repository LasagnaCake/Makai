#ifndef CTL_ALGORITHM_HASH_H
#define CTL_ALGORITHM_HASH_H

#include "../ctypes.hpp"
#include "../namespace.hpp"
#include "../typetraits/traits.hpp"
#include "../typetraits/cast.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Hash algorithm implementations.
namespace Impl::Hash {
	// Based off of https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/libsupc%2B%2B/hash_bytes.cc

	/// @brief Simple hash algorithm implementation.	
	namespace Simple {
		/// @brief Generates a hash from the given data, using GCC's dummy hash implementation.
		/// @param data Data to hash.
		/// @param sz Size of data to hash.
		/// @param seed Starting seed.
		/// @return Resulting hash.
		constexpr usize hash(ref<void const> data, usize sz, usize seed) {
			ref<char const> byte = static_cast<ref<char const>>(data);
			for (; sz; --sz) {
				seed *= 131;
				seed += *byte++;
			}
			return seed;
		}
	}
	
	// Based off of https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/libsupc%2B%2B/hash_bytes.cc

	/// @brief FNV1-A hash algorithm implementation.
	namespace FNV1a {
		/// @brief Generates the prime to be used.
		/// @return Prime to be used.
		consteval usize prime() {
			switch(sizeof(usize)) {
				case sizeof(uint64):	return static_cast<usize>(1099511628211ull);
				case sizeof(uint32):	return static_cast<usize>(16777619ull);
				default:				return 0;
			}
		}
		
		/// @brief Generates the offset to be used.
		/// @return Offset to be used.
		consteval usize offset() {
			switch(sizeof(usize)) {
				case sizeof(uint64):	return static_cast<usize>(14695981039346656037ull);
				case sizeof(uint32):	return static_cast<usize>(2166136261ull);
				default:				return 0;
			}
		}
		
		/// @brief Generates a hash from the given data, using the FNV1-A hash algorithm.
		/// @param data Data to hash.
		/// @param sz Size of data to hash.
		/// @param seed Starting seed.
		/// @return Resulting hash.
		constexpr usize hash(ref<void const> const data, usize sz, usize seed = offset()) {
			ref<char const> byte = static_cast<ref<char const>>(data);
			for (; sz; --sz) {
				seed ^= static_cast<usize>(*byte++);
				seed *= prime();
			}
			return seed;
		}
	}
	
	// Based off of https://github.com/aappleby/smhasher/blob/master/src/MurmurHash2.cpp
	
	/// @brief Murmur2 hash algorithm implementation.
	namespace Murmur2 {
		/// @brief Mixes a set of values.
		constexpr void mix(usize& h, usize& k, usize const m, int const r) {
			k *= m;
			k ^= k >> r;
			k *= m;
			h *= m;
			h ^= k;
		}
		
		/// @brief Shuffles a set of values.
		constexpr void shuffle(usize& h, usize const m, usize const v) {
			h ^= h >> v;
			h *= m;
			h ^= h >> v;
		}
		
		/// @brief Shuffles a set of values.
		constexpr void shuffle(usize& h, usize const m, usize const v1, usize const v2) {
			h ^= h >> v1;
			h *= m;
			h ^= h >> v2;
		}
		
		/// @brief Generates a hash from the given data, using the Murmur2 hash algorithm's 64-bit implementation.
		/// @param data Data to hash.
		/// @param sz Size of data to hash.
		/// @param seed Starting seed.
		/// @return Resulting hash.
		constexpr usize hash64(ref<void const> data, usize sz, usize seed = 0) {
			constexpr usize	m = 0xc6a4a7935bd1e995ull;
			constexpr int	r = 47;
			usize s		= sz;
			usize hash	= seed;
			ref<usize const> d1 = static_cast<ref<usize const>>(data);
			ref<usize const> end = d1 + (sz/8);
			while(data != end) {
				usize k = *d1++;
				mix(hash, k, m, r);
			}
			ref<byte const> d2 = (ref<byte const>)d2;
			usize t = 0;
			switch(sz & 7) {
			case 7:		t ^= static_cast<usize>(d2[6]) << 48;
			case 6:		t ^= static_cast<usize>(d2[5]) << 40;
			case 5:		t ^= static_cast<usize>(d2[4]) << 32;
			case 4:		t ^= static_cast<usize>(d2[3]) << 24;
			case 3:		t ^= static_cast<usize>(d2[2]) << 16;
			case 2:		t ^= static_cast<usize>(d2[1]) << 8;
			case 1:		t ^= static_cast<usize>(d2[0]);
			default:	break;
			}
			mix(hash, t, m, r);
			mix(hash, s, m, r);
			shuffle(hash, m, r);
			return hash;
		} 
		
		/// @brief Generates a hash from the given data, using the Murmur2 hash algorithm's 32-bit implementation.
		/// @param data Data to hash.
		/// @param sz Size of data to hash.
		/// @param seed Starting seed.
		/// @return Resulting hash.
		constexpr usize hash32(ref<void const> data, usize sz, usize seed = 0) {
			constexpr usize	m = 0x5bd1e995;
			constexpr int	r = 24;
			usize s		= sz;
			usize hash	= seed;
			ref<byte const> dt = static_cast<ref<byte const>>(data);
			while(sz >= 4) {
				usize k = *(usize*)dt;
				mix(hash, k, m, r);
				dt += 4;
				sz -= 4;
			}
			usize t = 0;
			switch(sz) {
			case 3:		t ^= dt[2] << 16;
			case 2:		t ^= dt[1] << 8;
			case 1:		t ^= dt[0];
			default:	break;
			};
			mix(hash, t, m, r);
			mix(hash, s, m, r);
			shuffle(hash, m, 13, 15);
			return hash;
		}

		namespace Impl {
			constexpr usize part(ref<char const> const data, usize const sz, usize const i) {
				if (i > 7) __builtin_unreachable();
				return ((i < sz) ? (usize(data[i]) << i * 8) : 0);
			}

			constexpr usize combine64(ref<char const> const data, usize const sz) {
				return (
					part(data, sz, 0)
				|	part(data, sz, 1)
				|	part(data, sz, 2)
				|	part(data, sz, 3)
				|	part(data, sz, 4)
				|	part(data, sz, 5)
				|	part(data, sz, 6)
				|	part(data, sz, 7)
				);
			}

			constexpr usize combine32(ref<char const> const data, usize const sz) {
				return (
					part(data, sz, 0)
				|	part(data, sz, 1)
				|	part(data, sz, 2)
				|	part(data, sz, 3)
				);
			}

			constexpr usize combine(ref<char const> const data, usize const sz) {
				if constexpr (sizeof(usize) == sizeof(uint64))
					return combine64(data, sz);
				else if constexpr (sizeof(usize) == sizeof(uint32))
					return combine32(data, sz);
				else __builtin_unreachable();
			}

			constexpr usize constHash(
				ref<char const> data,
				usize sz,
				usize seed,
				As<usize[2]> const& m,
				As<int[2]> const& r
			) {
				constexpr usize hashSize = sizeof(usize);
				usize const	m0 = m[0], m1 = m[1];
				int const	r0 = r[0], r1 = r[1];
				usize s		= sz;
				while (sz >= hashSize) {
					usize k = combine(data, sz);
					mix(seed, k, m0, r0);
					data += hashSize;
					sz -= hashSize;
				}
				usize t = combine(data, sz);
				mix(seed, t, m0, r0);
				mix(seed, s, m0, r0);
				shuffle(seed, m1, r1);
				return seed;
			}
		}

		/// @brief Generates a compile-time hash from the given data, using the Murmur2 hash algorithm's 64-bit implementation.
		/// @param data Data to hash.
		/// @param sz Size of data to hash.
		/// @param seed Starting seed.
		/// @return Resulting hash.
		constexpr usize constHash64(ref<char const> data, usize sz, usize seed = 0) {
			return Impl::constHash(data, sz, seed, {0xc6a4a7935bd1e995ull, 0xc6a4a7935bd1e995ull}, {47, 47});
		}
	}
	
	/// @brief Generates a hash from the given data.
	/// @param data Data to hash.
	/// @param sz Size of data to hash.
	/// @param seed Starting seed.
	/// @return Resulting hash.
	constexpr usize hash(ref<void const> data, usize sz, usize seed = 0) {
		if constexpr (sizeof(usize) == sizeof(uint64))	
			return Murmur2::hash64(data, sz, seed);
		else if constexpr (sizeof(usize) == sizeof(uint32))
			return Murmur2::hash32(data, sz, seed);
		else return Simple::hash(data, sz, seed);
	}

	/// @brief Generates a compile-time hash from the given data.
	/// @param data Data to hash.
	/// @param sz Size of data to hash.
	/// @param seed Starting seed.
	/// @return Resulting hash.
	constexpr usize constHash(ref<char const> const data, usize const sz, usize seed = 0) {
		if constexpr (sizeof(usize) == sizeof(uint64))	
			return Murmur2::constHash64(data, sz, seed);
		else __builtin_unreachable();
	}

	/// @brief Returns the default seed.
	/// @return Default seed.
	constexpr usize seed() {
		return Impl::Hash::constHash(
			"The intelligence of modern machines...",
			32,
			bitcast<usize, ssize>(0x15'A271F1C1AL)
		) & static_cast<usize>(0xFFFF'FFFF);
	}
}

/// @brief Static class used for generating compile-time string hashes.
namespace ConstHasher {
	/// @brief Hasher seed.
	constexpr static usize SEED = Impl::Hash::seed();

	/// @brief Generates the hash for a given "C-style" string.
	/// @param data Pointer to beginning of string.
	/// @param size Size of string.
	/// @return Resulting hash.
	constexpr static usize hash(cstring const data, usize const size) {
		return Impl::Hash::constHash(data, size, SEED);
	}

	/// @brief Generates the hash for a given fixed char array.
	/// @tparam S Array size.
	/// @param str Array to hash.
	/// @return Resulting hash.
	template< usize S>
	constexpr static usize hash(As<const char[S]> const& data) {
		return hash(data, S-1);
	}

	/// @brief Generates the hash for a given ranged type.
	/// @tparam T Ranged type.
	/// @param value Ranged object to hash.
	/// @return Resulting hash.
	template <Type::Iteratable T>
	constexpr static usize hash(T const& value) {
		return hash(&*value.begin(), value.end() - value.begin());
	}

	/// @brief Generates the hash for a given bounded type.
	/// @tparam T Bounded type.
	/// @param value Ranged object to hash.
	/// @return Resulting hash.
	template <Type::CIteratable T>
	constexpr static usize hash(T const& value)
	requires (!Type::Iteratable<T>) {
		return hash(value.data(), value.size());
	}
}

static_assert(ConstHasher::hash("Compile-time Magics!") != 0);

/// @brief Static class used for generating hashes.
/// @note For any type that isn't an `union`, `class`, `struct` or an array, all hashes are guaranteed to be collision-free.
struct Hasher {
	/// @brief Hasher seed.
	constexpr static usize SEED = ConstHasher::SEED;

	/// @brief Generates the hash for a given pointer.
	/// @tparam T pointed type.
	/// @param ptr pointer to hash.
	/// @return Resulting hash.
	template<class T>
	constexpr static usize hashPointer(ref<T> const ptr)	{return bitcast<usize>(ptr);		}

	/// @brief Generates the hash for a given integer.
	/// @tparam T Integer type.
	/// @param value Integer to hash.
	/// @return Resulting hash.
	template <Type::Integer T>
	constexpr static usize hash(T const& value)				{return value;						}

	/// @brief Generates the hash for a given floating point number.
	/// @param value Number to hash.
	/// @return Resulting hash.
	constexpr static usize hash(float const value)			{return bitcast<uint32>(value);		}
	
	/// @brief Generates the hash for a given floating point number.
	/// @param value Number to hash.
	/// @return Resulting hash.
	constexpr static usize hash(double const value)			{return bitcast<uint64>(value);		}

	/// @brief Generates the hash for a given pointer.
	/// @tparam T Convertible type.
	/// @param value Pointer to hash.
	/// @return Resulting hash.
	template <Type::Convertible<usize> T>
	constexpr static usize hash(T const value)		{return static_cast<usize>(value);	}

	/// @brief Generates the hash for a given `enum`.
	/// @tparam T `enum` type.
	/// @param value `enum` value to hash.
	/// @return Resulting hash.
	template <Type::Enumerator T>
	constexpr static usize hash(T const& value)		{return static_cast<usize>(value);	}

	/// @brief Generates the hash for a given range of elements.
	/// @tparam T Element type.
	/// @param data Pointer to beginning of range.
	/// @param size Size of range.
	/// @return Resulting hash.
	template<class T>
	constexpr static usize hash(ref<T const> const data, usize const size) {
		return Impl::Hash::constHash(data, size * sizeof(T), SEED);
	}

	/// @brief Generates the hash for a given fixed array.
	/// @tparam T Element type.
	/// @tparam S Array size.
	/// @param str Array to hash.
	/// @return Resulting hash.
	template<class T, usize S>
	constexpr static usize hash(As<const T[S]> const& data) {
		return hash(data, S);
	}

	/// @brief Generates the hash for a given ranged type.
	/// @tparam T Ranged type.
	/// @param value Ranged object to hash.
	/// @return Resulting hash.
	template <Type::Iteratable T>
	constexpr static usize hash(T const& value) {
		return hash(&*value.begin(), value.end() - value.begin());
	}

	/// @brief Generates the hash for a given bounded type.
	/// @tparam T Bounded type.
	/// @param value Ranged object to hash.
	/// @return Resulting hash.
	template <Type::CIteratable T>
	constexpr static usize hash(T const& value)
	requires (!Type::Iteratable<T>) {
		return hash(value.data(), value.size());
	}

	
	/// @brief Generates the hash for a given bounded type.
	/// @tparam T Bounded type.
	/// @param value Ranged object to hash.
	/// @return Resulting hash.
	template <Type::Class T>
	constexpr static usize hash(T const& value)
	requires (
		requires (T const& t) {{t.hash(SEED)} -> Type::Convertible<usize>;}
	) {
		return value.hash(SEED);
	}

	/// @brief Function prototype for future hashing specializations.
	/// @tparam T Type to specialize for.
	/// @param value Value to hash.
	/// @return Resulting hash.
	template <class T>
	constexpr static usize hash(T const& value);
};

/// @brief Algorithm-specific type constraints.
namespace Type::Algorithm {
	/// @brief Type must be hashable by `THasher`.
	template<class TValue, class THasher>
	concept Hashable = requires (TValue v) {
		{THasher::hash(v)} -> Type::Equal<usize>;
	};
}

CTL_NAMESPACE_END

#endif // CTL_ALGORITHM_HASH_H
