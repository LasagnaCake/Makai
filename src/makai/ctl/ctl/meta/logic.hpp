#ifndef CTL_META_LOGIC_H
#define CTL_META_LOGIC_H

#include "../namespace.hpp"
#include "../typetraits/typecontainer.hpp"
#include "../typetraits/converter.hpp"
#include "if.hpp"

CTL_NAMESPACE_BEGIN

namespace Meta {
	namespace Impl {
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

		template<typename A, typename B>	struct IsEqual:			FalseType	{};
		template<typename T>				struct IsEqual<T, T>:	TrueType	{};
	}

	/// @brief Logical `and`.
	/// @tparam ...Values Values to `and`.
	template <bool... Values>
	constexpr bool LogicalAnd	= (... && Values);

	/// @brief Logical `or`.
	/// @tparam ...Values Values to `or`.
	template <bool... Values>
	constexpr bool LogicalOr	= (... && Values);

	/// @brief Applies a type qualifier to another type, if it is not void.
	/// @tparam T Type to apply qualifier to.
	/// @tparam TQualifier<class> Qualifier to apply.
	template<class T, template <class> class TQualifier>
	using Apply = typename Impl::ApplyType<T, TQualifier>::Type;

	/// @brief Decays to either `T const` or `T`, depending on the condition.
	/// @tparam T Type to const-ify depending on condition.
	/// @tparam COND Condition to check for.
	template<bool COND, class T>
	using MakeConstIf = If<COND, AsConstant<T>, T>;

	/// @brief Decays to either `T const` or `T`, depending on whether `T` is const.
	/// @tparam T Type to const-ify depending on itself.
	/// @note This is to handle some wacky template interactions.
	template<class T>
	using MakeConstIfConst = MakeConstIf<Impl::IsEqual<T, T const>::value, T>;
}

CTL_NAMESPACE_END

#endif // CTL_META_LOGIC_H
