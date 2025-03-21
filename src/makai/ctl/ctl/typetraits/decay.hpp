#ifndef CTL_TYPETRAITS_DECAY_H
#define CTL_TYPETRAITS_DECAY_H

#include "typecontainer.hpp"
#include "converter.hpp"
#include "basictraits.hpp"
#include "../meta/logic.hpp"
#include "../namespace.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Type decay facilities.
namespace Decay {
	/// @brief Decays type as if it was a function argument passed by value.
	template<class T>
	using AsArgument = Meta::DualType<
        Type::Array<AsNonReference<T>>,
        AsPointer<AsNonExtent<AsNonReference<T>>>,
        Meta::DualType<
            Type::Function<AsNonReference<T>>,
            AsPointer<AsNonReference<T>>,
            AsNonCV<AsNonReference<T>>
        >
    >;

	/// @brief Decays type to itself. Behaves the same as `As<T>`.
	template<class T> using AsType = As<T>;

    /// @brief Decays type parameter pack to a kingdom of nothingness.
    template<class... Types> struct Pack {};

	/// @brief Type decay implementations.
	namespace Impl {
		template<class F>
		struct FunctionDecayType;

		template<class R>
		struct FunctionDecayType: TypeContainer<R()> {};

		template<class R, class A>
		struct FunctionDecayType<R(A)>:
			TypeContainer<Meta::DualType<Type::Void<A>, R(), R(A)>> {};

		template<class R, class A, class... Args>
		struct FunctionDecayType<R(A, Args...)>:
			TypeContainer<R(A, Args...)> {};

		template<class F>
		struct FunctionType;

		template<class R, class... Args>
		struct FunctionType<R(Args...)>: TypeContainer<R(Args...)> {};
	}

	/// @brief Decays function type to a proper C++ function type.
	template<typename F>
	using AsFunction = typename Impl::FunctionType<F>::Type;
};

static_assert(Type::Equal<Decay::AsFunction<bool(void)>, bool()>, "Something's not correct...");

/// @brief Ensures value passed can safely be moved.
/// @param v Value to move.
/// @return Value as a temporary.
template<class T>
constexpr AsNonReference<T>&&	move(T&& v)						{return static_cast<AsNonReference<T>&&>(v);	}
/// @brief Ensures value passed can safely be copied.
/// @param v Value to copy.
/// @return Copy of value.
template<class T>
constexpr AsNonReference<T>		copy(T const& v)				{return static_cast<AsNonReference<T>>(v);		}
/// @brief Ensures the given value is a constant reference.
/// @param v Value to decay.
/// @return Value as const reference.
template <class T>
constexpr AsConstant<T>&		constant(T& v)					{return v;										}

/// @brief Forwards references as either references or temporaries, depending on its type.
/// @param v Value to decay.
/// @return Value as either a reference or temporary.
template<class T>
constexpr T&& forward(typename Impl::NonReferenceType<T>::Type& v)	{return static_cast<T&&>(v);	}

/// @brief Forwards references as either references or temporaries, depending on its type.
/// @param v Value to decay.
/// @return Value as either a reference or temporary.
template<class T>
constexpr T&& forward(typename Impl::NonReferenceType<T>::Type&& v)	{return static_cast<T&&>(v);	}

CTL_NAMESPACE_END

#endif // CTL_TYPETRAITS_DECAY_H
