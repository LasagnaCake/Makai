#ifndef CTL_META_LOGIC_H
#define CTL_META_LOGIC_H

#include "../namespace.hpp"
#include "../typetraits/typecontainer.hpp"
#include "../typetraits/converter.hpp"

CTL_NAMESPACE_BEGIN

namespace Meta {
	namespace Impl {
		template<bool COND, class True, class False>	struct DualType:						TypeContainer<True> {};
		template<class True, class False>				struct DualType<false, True, False>:	TypeContainer<False> {};

		template<class T, template <class> class A>
		struct ApplyType;

		template<template <class> class A> struct ApplyType<void, A>				{using Type = void;};
		template<template <class> class A> struct ApplyType<void const, A>			{using Type = void;};
		template<template <class> class A> struct ApplyType<void volatile, A>		{using Type = void;};
		template<template <class> class A> struct ApplyType<void volatile const, A>	{using Type = void;};

		template<class T, template <class> class A>
		struct ApplyType {
			using Type = typename A<T>::Type;
		};
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

	/// @brief Applies a type qualifier to another type, if it is not void.
	/// @tparam T Type to apply qualifier to.
	/// @tparam TQualifier<class> Qualifier to apply.
	template<class T, template <class> class TQualifier>
	using Apply = typename Impl::ApplyType<T, TQualifier>::Type;

	/// @brief Decays to either `TTrue` or `TFalse`, depending on the condition.
	/// @tparam TTrue Type to decay to when `COND` is true.
	/// @tparam TFalse Type to decay to when `COND` is false.
	/// @tparam COND Condition to check for.
	template<bool COND, class TTrue, class TFalse>
	using If = Meta::DualType<COND, TTrue, TFalse>;

	
	/// @brief Decays to either `T const` or `T`, depending on the condition.
	/// @tparam T Type to const-ify depending on condition.
	/// @tparam COND Condition to check for.
	template<bool COND, class T>
	using MakeConstIf = Meta::DualType<COND, AsConstant<T>, T>;
}

CTL_NAMESPACE_END

#endif // CTL_META_LOGIC_H
