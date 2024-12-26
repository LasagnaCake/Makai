#ifndef CTL_META_LOGIC_H
#define CTL_META_LOGIC_H

#include "../namespace.hpp"

CTL_NAMESPACE_BEGIN

namespace Meta {
	namespace Impl {
		template<bool COND, class True, class False>	struct DualType:						TypeContainer<True> {};
		template<class True, class False>				struct DualType<false, True, False>:	TypeContainer<False> {};
	}

	/// @brief Logical `and`.
	/// @tparam ...Values Values to `and`.
	template <bool... Values>
	constexpr bool LogicalAnd	= (... && Values);

	/// @brief Logical `or`.
	/// @tparam ...Values Values to `or`.
	template <bool... Values>
	constexpr bool LogicalOr	= (... && Values);

	/// @brief Decays to either `TTrue` or `TFalse`, depending on the condition.
	/// @tparam TTrue Type to decay to when `COND` is true.
	/// @tparam TFalse Type to decay to when `COND` is false.
	/// @tparam COND Condition to check for.
	template<bool COND, class TTrue, class TFalse>
	using DualType = typename Impl::DualType<COND, TTrue, TFalse>::Type;
}

CTL_NAMESPACE_END

#endif // CTL_META_LOGIC_H
