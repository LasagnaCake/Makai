#ifndef CTL_CONTAINER_FUNCTION_H
#define CTL_CONTAINER_FUNCTION_H

#include "../templates.hpp"
#include "../typetraits/traits.hpp"
#include "../cpperror.hpp"

// Function implementation based off of: https://stackoverflow.com/a/61191826

CTL_NAMESPACE_BEGIN

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"

namespace Impl {
	namespace Partial {
		template <typename TReturn, typename... TArgs>
		struct Function {
			constexpr virtual TReturn invoke(TArgs...) const		= 0;
			constexpr TReturn operator()(TArgs... args) const		{return invoke(args...);}
			constexpr virtual ~Function()							= default;
		};
	}

	template <typename TFunction, typename TReturn, typename... TArgs>
	struct Function: Partial::Function<TReturn, TArgs...> {
		TFunction func;
		constexpr TReturn invoke(TArgs... args) const override {return func(args...);}
		constexpr Function(TFunction&& func):		func(move(func))	{}
		constexpr Function(TFunction const& func):	func(func)			{}

		constexpr virtual ~Function() {}
	};
}

/// @brief Callable object wrapper.
/// @tparam T Function type.
/// @note Callable objects: functions and lambdas.
template<typename T> struct Function;

/// @brief Callable object wrapper.
/// @tparam TReturn Function return type.
/// @tparam ...TArgs Function arguments.
/// @note Callable objects: functions and lambdas.
template<typename TReturn, typename... TArgs>
struct Function<TReturn(TArgs...)>:
	SelfIdentified<Function<TReturn(TArgs...)>>,
	Returnable<TReturn>,
	Argumented<TArgs...>,
	Functional<TReturn(TArgs...)> {
public:
	using Functional		= ::CTL::Functional<TReturn(TArgs...)>;
	using SelfIdentified	= ::CTL::SelfIdentified<Function<TReturn(TArgs...)>>;
	using Returnable		= ::CTL::Returnable<TReturn>;
	using Argumented		= ::CTL::Argumented<TArgs...>;

	using
		typename SelfIdentified::SelfType
	;

	using
		typename Functional::FunctionType
	;

	using
		typename Returnable::ReturnType
	;

private:
	/// @brief Callable bound to the object.
	owner<Impl::Partial::Function<ReturnType, TArgs...>> func{nullptr};

	template<typename TFunction>
	using Callable = Impl::Function<Decay::AsArgument<TFunction>, TReturn, TArgs...>;

	template<typename TFunction>
	constexpr static owner<Callable<TFunction>>
	makeCallable(TFunction&& f) {
		return ::new Callable<TFunction>(CTL::move(f));
	}

	template<typename TFunction>
	constexpr static owner<Callable<TFunction>>
	makeCallable(TFunction const& f) {
		return ::new Callable<TFunction>(f);
	}

	constexpr void assign(SelfType const& other) {
		if (!other.func) return;
		func = makeCallable(((owner<Callable<FunctionType>>)other.func)->func);
	}

	constexpr void destroy() {
		if (func) delete func;
		func = nullptr;
	}

	[[noreturn]] static void badCallError() {
		throw BadCallException("No function assigned!");
	}

public:
	/// @brief Invokes the callable bound to it.
	/// @param ...args Argument types.
	/// @return Result of the call.
	/// @throw BadCallException when there is no callable bound.
	constexpr ReturnType invoke(TArgs... args) const {
		if (!func) badCallError();
        return func->invoke(args...);
    }

	/// @brief Invokes the callable bound to it.
	/// @param ...args Argument types.
	/// @return Result of the call.
	/// @throw BadCallException when there is no callable bound.
    constexpr ReturnType operator()(TArgs... args) const {
        return invoke(args...);
    }

	/// @brief Destructor.
    constexpr ~Function() {destroy();}

	/// @brief Removes the stored function.
	/// @return Reference to self.
	constexpr SelfType& clear() {destroy(); return *this;}

	/// @brief Move assignment operator.
	/// @tparam TFunction Derived type.
	/// @param f Function to bind.
	/// @return Reference to self.
	template<typename TFunction>
    constexpr SelfType& operator=(TFunction&& f)
	requires (!Type::Derived<TFunction, SelfType>)		{destroy(); func = makeCallable(move(f)); return *this;				}
	/// @brief Copy assignment operator.
	/// @tparam TFunction Derived type.
	/// @param f Function to bind.
	/// @return Reference to self.
    template<typename TFunction>
    constexpr SelfType& operator=(TFunction const& f)
	requires (!Type::Derived<TFunction, SelfType>)		{destroy(); func = makeCallable(f); return *this;					}
    /// @brief Move assignment operator.
    /// @param f `Function` to move from.
    /// @return Reference to self.
    constexpr SelfType& operator=(SelfType&& f)			{destroy(); func = move(f.func); f.func = nullptr; return *this;	}
    /// @brief Copy assignment operator.
    /// @param f `Function` to copy from.
    /// @return Reference to self.
    constexpr SelfType& operator=(SelfType const& f)	{destroy(); assign(f); return *this;								}

    /// @brief Default constructor.
    constexpr Function() {}

    /// @brief Move constructor (callable).
    /// @tparam TFunction Callable type.
    /// @param f Callable.
    template<typename TFunction>
    constexpr Function(TFunction&& f)		{operator=(move(f));	}
    /// @brief Copy constructor (callable).
    /// @tparam TFunction Callable type.
    /// @param f Callable.
    template<typename TFunction>
    constexpr Function(TFunction const& f)	{operator=(f);			}
	/// @brief Copy constructor (`Function`).
	/// @param f `Function` object.
	constexpr Function(SelfType&& f)		{operator=(move(f));	}
	/// @brief Move constructor (`Function`).
	/// @param f `Function` object.
    constexpr Function(SelfType const& f)	{operator=(f);			}
};

/// @brief `Function` analog for a callable that takes zero or more arguments, and does not return a value.
/// @tparam ...Args Argument types.
template<typename... Args>
using Procedure		= Function<void(Args...)>;

/// @brief `Function` analog for a callable that takes an argument `T`, and returns a value of type `R`.
/// @tparam R Return type.
/// @tparam T Argument type. If not specified, it is `R const&`.
template<typename R, typename T = R const&>
using Operation		= Function<R(T)>;

/// @brief `Function` analog for a callable that takes no arguments, and returns a value.
/// @tparam R Return type.
template<typename R>
using Acquisition	= Function<R()>;

/// @brief `Function` analog for a signal.
/// @tparam ...Args Signal argument types.
template<typename... Args>
using Signal	= Procedure<Args...>;

/// @brief `Function` analog for a trigger. 
/// @tparam ...Args Argument types.
template<typename... Args>
using Trigger	= Function<bool(Args...)>;

#pragma GCC diagnostic pop

/// @brief Wraps an object with its member function.
/// @tparam TObject Object type.
/// @tparam TFunction Member function type.
/// @tparam TReturn Member function return type.
/// @tparam ...TArgs Member function argument types.
/// @param obj Object to wrap.
/// @param func Member function to wrap for object.
/// @return Wrapped function.
template<class TObject, class TReturn, class... TArgs>
constexpr Function<TReturn(TArgs...)>
memberCall(TObject& obj, As<TReturn(TObject::*)(TArgs...)> const func) {
	return [&] (TArgs... args) -> TReturn {return (obj.*func)(args...);};
}

/// @brief Returns a callable version of a member function.
/// @tparam TObject Object type.
/// @tparam TFunction Member function type.
/// @tparam TReturn Member function return type.
/// @tparam ...TArgs Member function argument types.
/// @param func Member function to wrap.
/// @return Member function as callable function.
template<class TObject, class TReturn, class... TArgs>
constexpr Function<TReturn(TObject&, TArgs...)>
fromMemberCall(As<TReturn(TObject::*)(TArgs...)> const func) {
	return [&] (TObject& obj, TArgs... args) -> TReturn {return (obj.*func)(args...);};
}

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_FUNCTION_H
