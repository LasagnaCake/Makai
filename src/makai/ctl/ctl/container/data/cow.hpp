#ifndef CTL_CONTAINER_DATA_COW_H
#define CTL_CONTAINER_DATA_COW_H

#include "../../namespace.hpp"
#include "../../templates.hpp"
#include "../../typeinfo.hpp"
#include "../../ctypes.hpp"
#include "../../cpperror.hpp"
#include "../../order.hpp"
#include "../../typetraits/traits.hpp"
#include "../../async/lock.hpp"
#include "../pointer/cell.hpp"


CTL_NAMESPACE_BEGIN

/// @brief Copy-on-Write value.
/// @tparam TData Value type.
template <class TData>
requires Type::Constructible<TData, TData const&>
struct Cow:
	Typed<TData>,
	SelfIdentified<Cow<TData>>,
	Ordered {
	using Typed				= ::CTL::Typed<TData>;
	using SelfIdentified	= ::CTL::SelfIdentified<Cow<TData>>;

	using
		typename Typed::DataType,
		typename Typed::ConstantType,
		typename Typed::PointerType,
		typename Typed::ConstPointerType,
		typename Typed::ReferenceType,
		typename Typed::ConstReferenceType,
		typename Typed::TemporaryType
	;

	using
		typename SelfIdentified::SelfType
	;

	constexpr Cow() requires Type::DefaultConstructible<DataType> 	{set(DataType());	}
	constexpr Cow(DataType const& value)							{set(value);		}
	constexpr Cow(SelfType const& other)							{set(other);		}

	constexpr SelfType& operator=(DataType const& newValue)	{return set(value);}
	constexpr SelfType& operator=(SelfType const& other)	{return set(other);}

	SelfType& set(DataType const& newValue) {
		value = value.create(newValue);
		return *this;
	}

	SelfType& set(SelfType const& other) {
		value = other.value;
		return *this;
	}

	DataType get() const {return *value;}

	constexpr DataType operator*() const	{return get();}
	constexpr operator DataType() const		{return get();}

	/// @brief Equality comparison operator (`Cow`).
	/// @param obj `Cow` to compare to.
	/// @return Whether they're equal.
	constexpr bool operator==(SelfType const& other) const			{return get() == other.get();	}
	/// @brief Threeway comparison operator (`Cow`).
	/// @param obj `Cow` to compare to.
	/// @return Order between objects.
	constexpr OrderType operator<=>(SelfType const& other) const	{return get() <=> other.get();	}

private:
	Cell<DataType> value;
};

CTL_NAMESPACE_END

#endif
