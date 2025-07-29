#ifndef CTL_RANDOM_CTPRNG_H
#define CTL_RANDOM_CTPRNG_H

#include "../namespace.hpp"
#include "../algorithm/hash.hpp"
#include "../typetraits/nameof.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Random number generation.
namespace Random {
	/// @brief Compile-time "pseudo"-random number. Has enough entropy for most purposes.
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
	#ifdef CTL_CTPRNG_ENTROPY_OFFSET
	+	(CTL_CTPRNG_ENTROPY_OFFSET)
	#endif
	#ifdef CTL_CTPRNG_ENTROPY_HASH
	+	ConstHasher::hash(CTL_CTPRNG_ENTROPY_HASH)
	#endif
	);

	/// @brief Implementation details.
	namespace Impl {
		/// @brief Compile-time "pseudo"-random number generator offset value.
		/// @tparam T integer type.
		template<Type::Integer T>
		struct CTRNGOffset {
			///	@brief Offset value. Formulated from a series of sources.
			T const value = (
				strhash(__builtin_FILE())
			+	strhash(__builtin_FUNCTION())
			+	__builtin_LINE()
			);

			/// @brief Hashes a compile-time string.
			/// @param str String to hash.
			/// @return Hash of string.
			consteval static usize strhash(cstring const str) {
				cstring current = str;
				if (!str) return 0;
				while (*current != '\0') ++current;
				return ConstHasher::hash(str, current - str);
			}
		};
	}

	/// @brief
	///		Generates a compile-time "pseudo"-random number
	///		between the lowest and highest integer value for the given type.
	///		Has enough entropy for most purposes.
	/// @tparam T Integer type.
	/// @param offset Result offset. Does not need to be passed, as it is deduced on a per-file, per-scope and per-line basis.
	template<Type::Integer T>
	consteval T ctsprng(Impl::CTRNGOffset<T> const& offset = {}) {return CTPRNG<T> + offset.value;}
}

CTL_NAMESPACE_END

#endif