#ifndef CTL_CONTAINER_SETGET_H
#define CTL_CONTAINER_SETGET_H

#include "function.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Setter function wrapper.
/// @tparam TData Data type to wrap a setter function for.
/// @details Wraps a setter function, such that it behaves like an assignment operator.
template<typename TData>
struct Setter:
	SelfIdentified<Setter<TData>>,
	Typed<TData> {
private:
public:
	using Typed				= ::CTL::Typed<TData>;
	using SelfIdentified	= ::CTL::SelfIdentified<Setter<TData>>;

	using
		typename Typed::DataType,
		typename Typed::ConstReferenceType
	;

	using typename SelfIdentified::SelfType;

	/// @brief Setter function type.
	using SetterFunction = Function<void(DataType const&)>;
	
	/// @brief Move constructor (function).
	/// @brief Function to move.
	constexpr Setter(SetterFunction&& func):		setter(CTL::move(func))	{}
	/// @brief Copy constructor (function).
	/// @brief Function to copy from.
	constexpr Setter(SetterFunction const& func):	setter(func)			{}

	/// @brief Move constructor (`Setter`) (deleted).
	constexpr Setter(SelfType&& other)		= delete;
	/// @brief Copy constructor (`Setter`) (deleted).
	constexpr Setter(SelfType const& other)	= delete;

	/// @brief Calls the setter function.
	/// @param value Value to set.
	/// @return Reference to self.
	constexpr SelfType& set(ConstReferenceType value)		{setter(value); return *this;	}
	/// @brief Calls the setter function.
	/// @param value Value to set.
	/// @return Reference to self.
	constexpr SelfType& operator=(ConstReferenceType value)	{return set(value);				}

protected:
	/// @brief Underlying setter function.
	SetterFunction const setter;
private:
};

/// @brief Setter function wrapper.
/// @tparam TData Data type to wrap a getter function for.
/// @details Wraps a geter function, such that it behaves like a conventional value acquisition.
template<typename TData>
struct Getter:
	SelfIdentified<Getter<TData>>,
	Typed<TData> {
private:
public:
	using Typed				= ::CTL::Typed<TData>;
	using SelfIdentified	= ::CTL::SelfIdentified<Getter<TData>>;

	using typename Typed::DataType;

	using typename SelfIdentified::SelfType;

	using GetterFunction = Function<DataType()>;

	/// @brief Move constructor (function).
	/// @brief Function to move.
	constexpr Getter(GetterFunction&& func):		getter(CTL::move(func))	{}
	/// @brief Copy constructor (function).
	/// @brief Function to copy from.
	constexpr Getter(GetterFunction const& func):	getter(func)			{}
	/// @brief Move constructor (`Getter`) (deleted).
	constexpr Getter(SelfType&& other)		= delete;
	/// @brief Copy constructor (`Getter`) (deleted).
	constexpr Getter(SelfType const& other)	= delete;
	
	/// @brief Calls the getter function.
	/// @return Current value.
	constexpr DataType get() const		{return getter();	}
	/// @brief Calls the getter function.
	/// @return Current value.
	constexpr operator DataType() const	{return get();		}

protected:
	/// @brief Underlying getter function.
	GetterFunction const getter;
private:
};

/// @brief Setter function wrapper.
/// @tparam TData Data type to wrap both a setter and a getter function for.
/// @details Wraps a setter and a getter function, such that it behaves like a `Setter` and a `Getter`.
template<typename TData>
struct SetGet:
	Setter<TData>,
	Getter<TData>,
	SelfIdentified<SetGet<TData>>,
	Typed<TData>,
	Derived<Setter<TData>, Getter<TData>> {
private:
public:
	using Setter			= ::CTL::Setter<TData>;
	using Getter			= ::CTL::Getter<TData>;
	using SelfIdentified	= ::CTL::SelfIdentified<SetGet<TData>>;

	static_assert(Type::Equal<typename Setter::DataType, typename Getter::DataType>);

	using typename Setter::SetterFunction;

	using typename Getter::GetterFunction;

	using typename SelfIdentified::SelfType;

	using
		typename Setter::DataType,
		typename Setter::ConstReferenceType
	;

	/// @brief Full move constructor (setter & getter functions).
	/// @param set Setter function to move.
	/// @param get Getter function to move.
	constexpr SetGet(SetterFunction&& set, GetterFunction&& get):			Setter(CTL::move(set)), Getter(CTL::move(get))	{}
	/// @brief Move & copy constructor (setter & getter functions).
	/// @param set Setter function to move.
	/// @param get Getter function to copy from.
	constexpr SetGet(SetterFunction&& set, GetterFunction const& get):		Setter(CTL::move(set)), Getter(get)				{}
	/// @brief Copy & move constructor (setter & getter functions).
	/// @param set Setter function to copy from.
	/// @param get Getter function to move.
	constexpr SetGet(SetterFunction	const& set, GetterFunction&& get):		Setter(set), Getter(CTL::move(get))				{}
	/// @brief Full copy constructor (setter & getter functions).
	/// @param set Setter function to copy from.
	/// @param get Getter function to copy from.
	constexpr SetGet(SetterFunction const& set, GetterFunction const& get):	Setter(set), Getter(get)						{}
	/// @brief Move constructor (`Getter`) (deleted).
	constexpr SetGet(SelfType&& other)		= delete;
	/// @brief Copy constructor (`Getter`) (deleted).
	constexpr SetGet(SelfType const& other)	= delete;

	/// @brief Calls the setter function.
	/// @param value Value to set.
	/// @return Reference to self.
	constexpr SelfType& set(ConstReferenceType value)		{Setter::set(value); return *this;	}
	/// @brief Calls the setter function.
	/// @param value Value to set.
	/// @return Reference to self.
	constexpr SelfType& operator=(ConstReferenceType value)	{return set(value);					}

	/// @brief Calls the getter function.
	/// @return Current value.
	constexpr DataType get() const		{return Getter::get();	}
	/// @brief Calls the getter function.
	/// @return Current value.
	constexpr operator DataType() const	{return get();			}
};

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_SETGET_H
