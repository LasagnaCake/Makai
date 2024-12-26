#ifndef MAKAILIB_DATA_HASH_H
#define MAKAILIB_DATA_HASH_H

#include "../compat/ctl.hpp"

/// @brief Data manipulation facilities.
namespace Makai::Data {
	/// @brief Data hash mode.
	enum class HashMode {
		HM_SHA3_224,
		HM_SHA3_256,
		HM_SHA3_384,
		HM_SHA3_512,
	};

	/// @brief Hashes the given bytes in a given hashing scheme.
	/// @param data Data to hash.
	/// @param mode Hashing scheme to use.
	void hash(BinaryData<>& data, HashMode const& mode = HashMode::HM_SHA3_256);
	/// @brief Hashes a set of bytes in a given hashing scheme.
	/// @param data Data to hash.
	/// @param mode Hashing scheme to use.
	/// @return Hashed data.
	BinaryData<> hashed(BinaryData<> const& data, HashMode const& mode = HashMode::HM_SHA3_256);
}

#endif // MAKAILIB_DATA_HASH_H
