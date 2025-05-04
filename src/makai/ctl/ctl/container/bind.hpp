#ifndef CTL_CONTAINER_BIND_H
#define CTL_CONTAINER_BIND_H

#include "function.hpp"
#include "tuple.hpp"
#include "pointer/reference.hpp"

CTL_NAMESPACE_BEGIN

// Based off of GCC's implementation

/// @brief Argument placeholder representation.
/// @tparam N Argument index.
template <usize N> struct Placeholder: ValueConstant<usize, N> {};

/// @brief Argument placeholder.
/// @tparam N Argument index.
template <usize N> constexpr Placeholder<N> arg = Placeholder<N>();

/// @brief Implementations.
namespace Impl {
	/// @brief Bind call deductor.
	template <class T>
	struct IsBindCall: BooleanConstant<false> {};

	/// @brief Type must be a placeholder representation type.
	template <class T>
	concept PlaceholderType	= requires {
		AsNormal<T>::value;
		requires Type::Equal<AsNormal<T>, Placeholder<AsNormal<T>::value>>;
	};

	/// @brief Type must NOT be a placeholder representation type.
	template <class T>
	concept NonPlaceholderType = !PlaceholderType<T>;

	/// @brief Whether a given type is a placeholder.
	template <class T>
	constexpr bool isPlaceholder = PlaceholderType<T>;

	/// @brief Gets the argument index of the placeholder.
	template <PlaceholderType T>
	constexpr usize placeof		= AsNormal<T>::value;
	/// @brief Gets whether the type is a bind call type.
	template <class T>
	constexpr bool isBindCall	= IsBindCall<T>::value;

	/// @brief Element type unpacker.
	/// @tparam T Element type.
	/// @tparam B Whether type is a bind call type.
	template <class T, bool B = isBindCall<T>>
	struct Unpacker {};

	template<class T>
	struct Unpacker<T, true> {
		template<class TBind, class... TArgs>
		constexpr auto get(TBind& bind, Tuple<TArgs...>& args)
		const volatile -> decltype(bind(declval<TArgs>()...)) {
			using Indices = IntegerPack<sizeof...(TArgs)>;
			return invoke(bind, args, Indices());
		}

		template<class TBind, class... TArgs, usize... I>
		constexpr auto invoke(TBind& bind, Tuple<TArgs...>& args, IndexTuple<I...> const&)
		const volatile -> decltype(bind(declval<TArgs>()...)) {
			return bind(::CTL::get<I>(move(args))...);
		}
	};

	template <PlaceholderType T>
	struct Unpacker<T, false> {
		template<class TTuple>
		constexpr TupleType<TTuple, placeof<T>-1> get(T volatile const&, TTuple& tuple)
		const volatile {
			return ::CTL::get<placeof<T>-1>(move(tuple));
		}
	};

	template <NonPlaceholderType T>
	struct Unpacker<T, false> {
		template<class TTuple>
		constexpr T get(T arg, TTuple&)
		const volatile {
			return arg;
		}
	};

	template <NonPlaceholderType T>
	struct Unpacker<T&, false> {
		template<class TTuple>
		constexpr T& get(T& arg, TTuple&)
		const volatile {
			return arg;
		}
	};

	/// @brief Call binder.
	/// @tparam T Callable type.
	template <class T> struct Binder;

	template<class TFunction, class... TArgs>
	struct Binder<TFunction(TArgs...)> {
	private:
		using Indices = IntegerPack<sizeof...(TArgs)>;

		using ReturnType = AsReturn<TFunction>;

		TFunction		func;
		Tuple<TArgs...>	binds;

		template<class... Args, usize... I>
		constexpr auto call(Tuple<Args...>&& args, IndexTuple<I...>) const {
			return func(Unpacker<TArgs>().get(get<I>(binds), args)...);
		}

	public:
		template<class... Args>
		explicit constexpr Binder(TFunction const& func, Args&&... args)
		: func(func), binds(forward<Args>(args)...) {}

		template<class... Args>
		explicit constexpr Binder(TFunction&& func, Args&&... args)
		requires (Type::NonConstant<TFunction>)
		: func(move(func)), binds(forward<Args>(args)...) {}

		Binder(Binder const&)	= default;
		Binder(Binder&&)		= default;

		template<class... Args>
		constexpr auto invoke(Args... args) const {
			return call(Tuple<Args...>(args...), Indices());
		}

		template<class... Args>
		constexpr auto operator()(Args&&... args) const {
			return call(Tuple<Args...>(args...), Indices());
		}
	};

	template <class T> struct IsBindCall<Binder<T>>:		TrueType {};
	template <class T> struct IsBindCall<Binder<T> const>:	TrueType {};
}

/// @brief Binds a function to a series of arguments, or placeholders.
/// @tparam F Function type.
/// @tparam ...Binds Argument/placeholder types.
/// @param func Function to bind.
/// @param ...args Arguments/placeholders to bind.
/// @return Bound function.
template <class F, class... Binds>
constexpr auto bind(F&& func, Binds&&... args) {
	return Impl::Binder<F(Binds...)>(forward<decltype(func)>(func), forward<Binds>(args)...);
}

CTL_NAMESPACE_END

#endif