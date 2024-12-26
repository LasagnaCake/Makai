#ifndef CTL_CLASS_TEMPLATES_H
#define CTL_CLASS_TEMPLATES_H

#include "ctypes.hpp"
#include "typeinfo.hpp"
#include "meta/pack.hpp"
#include "namespace.hpp"
#include "typetraits/traits.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Tags the deriving class as knowing itself.
/// @tparam TSelf Self type.
template<class TSelf>
struct SelfIdentified {
	/// @brief Self type.
	using SelfType	= TSelf;
	/// @brief Type info of self.
	using Self		= TypeInfo<TSelf>;
};

/// @brief Tags the deriving class as having content related to a specific type.
/// @tparam TData Type.
template<class TData>
struct Typed {
	/// @brief Type.
	using DataType				= TData;
	/// @brief Constant type.
	using ConstantType			= DataType const;
	/// @brief Reference type.
	using ReferenceType			= DataType&;
	/// @brief Temporary type.
	using TemporaryType			= DataType&&;
	/// @brief Const reference type.
	using ConstReferenceType	= ConstantType&;
	/// @brief Pointer type.
	using PointerType			= DataType*;
	/// @brief Const pointer type.
	using ConstPointerType		= DataType const*;
};

/// @brief Tags the deriving class as being indexable.
/// @tparam TIndex Index type.
template<Type::Integer TIndex>
struct Indexed {
	/// @brief Size type.
	using SizeType	= AsUnsigned<TIndex>;
	/// @brief Index type.
	using IndexType	= AsSigned<SizeType>;

	/// @brief Maximum size of size type.
	constexpr static SizeType MAX_SIZE = TypeInfo<SizeType>::HIGHEST;

	static_assert(IndexType(-1) == -1);
};

/// @brief Contains information on a parameter pack.
/// @tparam ...T 
template<class... T>
struct PackInfo: Decay::Pack<T...> {
	/// @brief Nth type finder.
	template<usize N> using	Type = Meta::NthType<N, T...>;
	/// @brief First type.
	using FirstType	= Meta::FirstType<T...>;
	/// @brief Last type.
	using LastType	= Meta::LastType<T...>;

	/// @brief Parameter pack size.
	constexpr static usize COUNT = sizeof...(T);
};

template<class... BaseTypes>
struct Derived;

/// @brief Tags the deriving class as being derived from a `TBase` class.
/// @tparam TBase Base type.
template<class TBase>
struct Derived<TBase> {
	/// @brief Base type.
	using BaseType = TBase;
};

/// @brief Tags the deriving class as being derived from `FirstBase`, and a series of `BaseTypes`.
/// @tparam FirstBase First base.
/// @tparam ...BaseTypes Following bases.
template<class FirstBase, class... BaseTypes>
struct Derived<FirstBase, BaseTypes...> {
	/// @brief Base types.
	using Bases = PackInfo<FirstBase, BaseTypes...>;
};

/// @brief Tags the deriving class as having content related to multiple types.
/// @tparam ...Types Types.
template<class... Types>
struct Polyglot {
	/// @brief Types.
	using DataTypes = PackInfo<Types...>;
};

/// @brief Tags the deriving class as being nullable.
struct Nulled {
	/// @brief Null type.
	using NullType = nulltype;
};

/// @brief Tags the deriving class as containing a default value for a given type.
/// @tparam T Type. 
template<Type::Constructible<> T>
struct Defaultable {
	constexpr static T defaultValue() {return T();}
};

/// @brief Tags the deriving class as returning a given type, when called like a function.
/// @tparam TReturn Return type.
template<class TReturn>
struct Returnable {
	/// @brief Return type.
	using ReturnType	= TReturn;
	/// @brief Return type information.
	using Return		= Typed<ReturnType>;
};

/// @brief Tags the deriving class as taking a list of argument types, when called like a function.
/// @tparam ...Types Argument types.
template<class... Types>
struct Argumented {
	/// @brief Argument types.
	using ArgumentTypes = PackInfo<Types...>;
};

template<class TFunction>
struct Functional {};

/// @brief Tags the class as behaving like a function.
/// @tparam TReturn Return type.
/// @tparam ...TArguments Argument types.
template<class TReturn, class... TArguments>
struct Functional<TReturn(TArguments...)> {
	/// @brief Function type.
	using FunctionType = Decay::AsFunction<TReturn(TArguments...)>;
};

/// @brief Tags the class as being able to be converted to a "C-style" string, in some capacity.
/// @tparam T Character type.
template<Type::ASCII T>
struct CStringable {
	using CStringType = typename Typed<T>::ConstPointerType;
};

CTL_NAMESPACE_END

#endif // CTL_CLASS_TEMPLATES_H
