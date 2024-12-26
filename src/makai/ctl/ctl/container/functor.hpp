#ifndef CTL_CONTAINER_FUNCTOR_H
#define CTL_CONTAINER_FUNCTOR_H

#include "../namespace.hpp"
#include "nullable.hpp"
#include "../staticvalue.hpp"
#include "function.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Callable object wrapper.
/// @tparam TFunction Callable type.
/// @note Callable objects: functions and lambdas.
template<typename TFunction> class Functor;

/// @brief Callable object wrapper.
/// @tparam TReturn Return type.
/// @tparam ...TArgs Argument types.
/// @note Callable objects: functions and lambdas.
template<typename TReturn, typename... TArgs>
struct Functor<TReturn(TArgs...)>:
	Ordered,
	SelfIdentified<Functor<TReturn(TArgs...)>>,
	Returnable<TReturn>,
	Argumented<TArgs...>,
	Functional<TReturn(TArgs...)> {
private:
public:
	using SelfIdentified	= ::CTL::SelfIdentified<Functor<TReturn(TArgs...)>>;
	using Returnable		= ::CTL::Returnable<TReturn>;
	using Argumented		= ::CTL::Argumented<TArgs...>;
	using Functional		= ::CTL::Functional<TReturn(TArgs...)>;

	using
		typename SelfIdentified::SelfType
	;

	using
		typename Returnable::ReturnType
	;

	using
		typename Functional::FunctionType
	;

	using
		typename Ordered::OrderType,
		typename Ordered::Order
	;

	/// @brief Type used to wrap callables in.
	using WrapperType = Function<FunctionType>;

	/// @brief Whether the `Functor` is a procedure (does not return any value).
	constexpr static bool PROCEDURE = Type::Different<ReturnType, void>;

	/// @brief Empty constructor.
	constexpr Functor(): id(0) {}
	/// @brief Callable/wrapper constructor.
	/// @param f Callable to bind.
	constexpr Functor(WrapperType const& f): func(f), id(++count)				{}
	/// @brief Callable/wrapper constructor.
	/// @tparam TFunction function type.
	/// @param f Callable to bind.
	template<Type::Functional<FunctionType> TFunction>
	constexpr Functor(TFunction const& f): func(f), id(++count)					{}
	/// @brief Copy constructor.
	/// @param f Other `Functor` object.
	constexpr Functor(SelfType const& other): func(other.func), id(other.id)	{}
	
	/// @brief Callable/wrapper assignment operator.
	/// @tparam TFunction function type.
	/// @param f Callable to bind.
	/// @return Reference to self.
	template<Type::Functional<FunctionType> TFunction>
	constexpr SelfType& operator=(TFunction const& f)		{func = f; id = ++count; return *this;				}
	/// @brief Callable/wrapper assignment operator.
	/// @param f Callable to bind.
	/// @return Reference to self.
	constexpr SelfType& operator=(WrapperType const& f)		{func = f; id = ++count; return *this;				}
	/// @brief Copy assignment constructor.
	/// @param f Callable to bind.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType const& other)	{func = other.func; id = other.id; return *this;	}
	
	/// @brief Invokes the callable bound to it. If no callable was attached, returns `nullptr`.
	/// @param ...args Argument types.
	/// @return Result of the call, or `nullptr` if no callable was attached.
	constexpr Nullable<ReturnType> invoke(TArgs... args) const requires (PROCEDURE) {
		if (id)
			return func(args...);
		return nullptr;
	}
	/// @brief Invokes the callable bound to it. If no callable was attached, returns `nullptr`.
	/// @param ...args Argument types.
	/// @return Result of the call, or `nullptr` if no callable was attached.
	constexpr Nullable<ReturnType> operator()(TArgs... args) const requires (PROCEDURE)	{
		return invoke(args...);
	}

	/// @brief If it exists, invokes the callable bound to it.
	/// @param ...args Argument types.
	constexpr void invoke(TArgs... args) const requires (!PROCEDURE)		{if (id) func(args...);	}
	/// @brief If it exists, invokes the callable bound to it.
	/// @param ...args Argument types.
	constexpr void operator()(TArgs... args) const requires (!PROCEDURE)	{invoke(args...);		}

	/// @brief Returns whether there is a callable bound to this object.
	constexpr bool exists() const	{return id;			}
	/// @brief Returns whether there is a callable bound to this object.
	constexpr operator bool() const	{return exists();	}

	/// @brief Equality operator.
	/// @param other Other `Functor` object to compare with.
	/// @return Whether they're equal.
	constexpr bool operator==(SelfType const& other) const {return id == other.id;}

	/// @brief Threeway comparison operator.
	/// @param other Other `Functor` to compare with.
	/// @return Order between both `Functor`s.
	constexpr OrderType operator<=>(SelfType const& other) const {return id <=> other.id;}

private:
	/// @brief Callable bound to the object.
	WrapperType			func;
	/// @brief ID of the object.
	usize				id = 0;
	/// @brief ID generator.
	inline static usize	count;
};

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_FUNCTOR_H
