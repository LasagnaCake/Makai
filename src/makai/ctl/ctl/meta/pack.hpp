#ifndef CTL_META_PACK_H
#define CTL_META_PACK_H

#include "../namespace.hpp"
#include "../ctypes.hpp"

CTL_NAMESPACE_BEGIN

namespace Meta {
	namespace Impl {
		template <usize N, typename... T>
		struct NthInPack;

		template <typename T, typename... Types>
		struct NthInPack<0, T, Types...> {
			using Type = T;
		};
		template <usize N, typename T, typename... Types>
		struct NthInPack<N, T, Types...> {
			using Type = typename NthInPack<N-1, Types...>::Type;
		};
	}

	/// @brief Returns the Nth type in a series of types.
	/// @tparam ...Types Types.
	/// @tparam N Type to locate.
	template<usize N, typename... Types>
	using NthType = typename Impl::NthInPack<N, Types...>::Type;

	/// @brief Returns the first type in a series of types.
	/// @tparam ...Types Types.
	template<typename... Types>
	using FirstType = typename Impl::NthInPack<0, Types...>::Type;

	/// @brief Returns the last type in a series of types.
	/// @tparam ...Types Types.
	template<typename... Types>
	using LastType = typename Impl::NthInPack<sizeof...(Types)-1, Types...>::Type;
}

CTL_NAMESPACE_END

#endif