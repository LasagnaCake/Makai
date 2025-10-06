#ifndef CTL_TYPETRAITS_CAST_H
#define CTL_TYPETRAITS_CAST_H

#include "../namespace.hpp"
#ifdef CTL_DO_NOT_USE_BUILTINS
#include "../memory/memory.hpp"
#endif // CTL_DO_NOT_USE_BUILTINS

CTL_NAMESPACE_BEGIN

/// @brief Bit-cast implementation.
/// @tparam TDst Destination type.
/// @tparam TSrc Source type.
/// @param v Value to cast.
/// @return Value casted as `TDst`.
/// @note Size of `TDst` and `TSrc` must be equivalent.
template<class TDst, class TSrc>
#ifndef CTL_DO_NOT_USE_BUILTINS
constexpr
#endif // CTL_DO_NOT_USE_BUILTINS
TDst bitcast(TSrc const& v) noexcept {
	static_assert(sizeof(TDst) == sizeof(TSrc), "Sizes of source and target type must match!");
	#ifdef CTL_DO_NOT_USE_BUILTINS
	TDst r;
	MX::memcpy(&r, &v, sizeof(TDst));
	return r;
	#else
	return __builtin_bit_cast(TDst, v);
	#endif // CTL_DO_NOT_USE_BUILTINS
}

/// @brief casting functions.
namespace Cast {
	/// @brief `static_cast` alias.
	/// @tparam TDst Destination type.
	/// @tparam TSrc Source type.
	/// @param v Value to cast.
	/// @return Value casted as `TDst`.
	template<class TDst, class TSrc> constexpr TDst as(TSrc v) noexcept				{return static_cast<TDst>(v);		}
	/// @brief `const_cast` alias.
	/// @tparam TDst Destination type.
	/// @tparam TSrc Source type.
	/// @param v Value to cast.
	/// @return Value casted as `TDst`.
	template<class TDst, class TSrc> constexpr TDst mutate(TSrc v) noexcept			{return const_cast<TDst>(v);		}
	/// @brief `dynamic_cast` alias.
	/// @tparam TDst Destination type.
	/// @tparam TSrc Source type.
	/// @param v Value to cast.
	/// @return Value casted as `TDst`.
	template<class TDst, class TSrc> constexpr TDst morph(TSrc v) noexcept			{return dynamic_cast<TDst>(v);		}
	/// @brief `reinterpret_cast` alias.
	/// @tparam TDst Destination type.
	/// @tparam TSrc Source type.
	/// @param v Value to cast.
	/// @return Value casted as `TDst`.
	template<class TDst, class TSrc> constexpr TDst rewrite(TSrc v) noexcept		{return reinterpret_cast<TDst>(v);	}
	/// @brief `reinterpret_cast` alias.
	/// @tparam TDst Destination type.
	/// @tparam TSrc Source type.
	/// @param v Value to cast.
	/// @return Value casted as `TDst`.
	template<class TDst, class TSrc> constexpr TDst reinterpret(TSrc v) noexcept	{return rewrite<TDst, TSrc>(v);		}
	/// @brief `bitcast` alias.
	/// @tparam TDst Destination type.
	/// @tparam TSrc Source type.
	/// @param v Value to cast.
	/// @return Value casted as `TDst`.
	template<class TDst, class TSrc> constexpr TDst bit(TSrc const& v) noexcept		{return bitcast<TDst, TSrc>(v);		}
}

CTL_NAMESPACE_END

#endif