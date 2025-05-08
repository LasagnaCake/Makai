#ifndef CTL_RANDOM_CTPRNG_H
#define CTL_RANDOM_CTPRNG_H

#include "../namespace.hpp"
#include "../algorithm/hash.hpp"
#include "../typetraits/nameof.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Random number generation.
namespace Random {
	/// @brief Compile-time "pseudo"-random number. There's enough entropy for most purposes.
	template<Type::Integer T>
	constexpr T CTPRNG = (
		ConstHasher::hash(__DATE__)
	+	ConstHasher::hash(__TIME__)
	+	ConstHasher::hash(__FILE__)
	+	ConstHasher::hash(__VERSION__)
	+	ConstHasher::hash(__BASE_FILE__)
	+	ConstHasher::hash(__FILE_NAME__)
	+	ConstHasher::hash(nameof<T>())
	+	(__INCLUDE_LEVEL__)
	+	(__LINE__)
	);
}

CTL_NAMESPACE_END

#endif