#ifndef CTL_CONTAINER_NULLABLE_H
#define CTL_CONTAINER_NULLABLE_H

#include "../namespace.hpp"
#include "../templates.hpp"
#include "../order.hpp"
#include "../typetraits/traits.hpp"
#include "../algorithm/strconv.hpp"
#include "../adapter/comparator.hpp"
#include "error.hpp"

CTL_NAMESPACE_BEGIN

template<class TData> class Nullable;

/// @brief Nullable type for void.
template<> class Nullable<void>:
	//public Typed<void>,
	public SelfIdentified<Nullable<void>>,
	public Nulled,
	public Ordered {
public:
	using DataType = void;

	using typename Nulled::NullType;
	using typename Ordered::OrderType;

	/// @brief Empty constructor.
	constexpr Nullable() noexcept			{}
	/// @brief Null constructor.
	constexpr Nullable(NullType) noexcept	{}

	/// @brief Always returns `false`.
	/// @return `false`.
	constexpr bool exists()		const {return false;	}
	/// @brief Always returns `false`.
	/// @return `false`.
	constexpr operator bool()	const {return exists();	}
	/// @brief Always returns `false`.
	/// @return `false`.
	constexpr bool operator()()	const {return exists();	}

	/// @brief Does nothing.
	/// @return Reference to self.
	constexpr Nullable& operator=(NullType)						{return *this;	}
	/// @brief Does nothing.
	/// @return Reference to self.
	constexpr Nullable& operator=(Nullable<void> const& other)	{return *this;	}
	/// @brief Does nothing.
	/// @return Reference to self.
	constexpr Nullable& operator=(Nullable<void>&& other)		{return *this;	}

	/// @brief Always returns `true`.
	/// @return `true`.
	constexpr bool operator==(NullType) const						{return true;	}
	/// @brief Always returns `true`.
	/// @return `true`.
	constexpr bool operator==(Nullable<void> const& other) const	{return true;	}
	/// @brief Always returns `true`.
	/// @return `true`.
	constexpr bool operator==(Nullable<void>&& other) const			{return true;	}

	/// @brief Always returns `Order::EQUAL`.
	/// @return `Order::EQUAL`.
	constexpr OrderType operator<=>(Nullable<void> const& other) const	{return Order::EQUAL;	}
	/// @brief Always returns `Order::EQUAL`.
	/// @return `Order::EQUAL`.
	constexpr OrderType operator<=>(Nullable<void>&& other) const		{return Order::EQUAL;	}

	/// @brief Destructor.
	constexpr ~Nullable() {}
};

/// @brief Nullable value.
/// @tparam TData Value type.
template<Type::NonVoid TData>
class Nullable<TData>:
	public SelfIdentified<Nullable<TData>>,
	public Typed<TData>,
	public Nulled,
	public Ordered {
public:
	using Typed				= ::CTL::Typed<TData>;
	using SelfIdentified	= ::CTL::SelfIdentified<Nullable<TData>>;

	using
		typename Typed::DataType,
		typename Typed::ConstantType,
		typename Typed::ReferenceType,
		typename Typed::ConstReferenceType,
		typename Typed::TemporaryType
	;

	using typename SelfIdentified::SelfType;

	using typename Nulled::NullType;

	using 
		typename Ordered::OrderType, 
		typename Ordered::Order
	;

	/// @brief Operation type.
	using OperationType	= Decay::AsFunction<DataType(DataType const&)>;
	/// @brief Procedure type.
	using ProcedureType	= Decay::AsFunction<void(DataType const&)>;

	/// @brief Empty constructor.
	constexpr Nullable() noexcept										{												}
	/// @brief Null constructor.
	constexpr Nullable(NullType) noexcept								{												}
	/// @brief Copy constructor (value).
	/// @param value Value to copy.
	constexpr Nullable(ConstReferenceType value): isSet(true)			{data = value;									}
	/// @brief Move constructor (value).
	/// @param value Value to move.
	constexpr Nullable(TemporaryType value): isSet(true)				{data = CTL::move(value);						}
	/// @brief Copy constructor (`Nullable`).
	/// @param other `Nullable` to copy from.
	constexpr Nullable(SelfType const& other): isSet(other.isSet)		{if (other.isSet) data = other.data;			}
	/// @brief Move constructor (`Nullable`).
	/// @param other `Nullable` to move from.
	constexpr Nullable(SelfType&& other): isSet(CTL::move(other.isSet))	{if (other.isSet) data = CTL::move(other.data);	}

	/// @brief Destructor.
	constexpr ~Nullable() {}

	/// @brief Returns the stored value.
	/// @return Stored value.
	/// @throw Error::NonexistentValue if value is not set.
	constexpr DataType value() const {
		if (isSet) return data;
		throw Error::NonexistentValue("Value is not set!", CTL_CPP_PRETTY_SOURCE);
	}

	/// @brief Returns the stored value, or a fallback.
	/// @param fallback Fallback value.
	/// @return Stored value, or `fallback` if not set.
	constexpr DataType orElse(DataType const& fallback) const {return (isSet) ? data : fallback;}

	/// @brief Applies an operation to the stored value, if it exists.
	/// @tparam TFunction Operation type.
	/// @param op Operation to apply.
	/// @return Reference to self.
	template<Type::Functional<OperationType> TFunction>
	constexpr SelfType& then(TFunction const& op) {if (isSet) data = op(data); return *this;}

	/// @brief Returns whether there is a stored value.
	/// @return Whether there is a stored value.
	constexpr bool exists()		const {return isSet;	}
	/// @brief Returns whether there is a stored value.
	/// @return Whether there is a stored value.
	constexpr operator bool()	const {return exists();	}
	/// @brief Returns whether there is a stored value.
	/// @return Whether there is a stored value.
	constexpr bool operator()()	const {return exists();	}

	/// @brief Applies an operation to the stored value, if it exists.
	/// @tparam TFunction Operation type.
	/// @param op Operation to apply.
	/// @return Reference to self.
	template<Type::Functional<OperationType> TFunction>
	constexpr SelfType& operator()(TFunction const& op) {return then(op);}

	/// @brief Copy assignment operator (value).
	/// @param value Value to copy.
	/// @return Reference to self.
	constexpr SelfType& operator=(DataType const& value)	{return set(value);				}
	/// @brief Copy assignment operator (value).
	/// @param value Value to move.
	/// @return Reference to self.
	constexpr SelfType& operator=(DataType&& value)			{return set(value);				}
	/// @brief Clears the current stored value.
	/// @return Reference to self.
	constexpr SelfType& operator=(NullType)					{return clear();				}
	/// @brief Copy assignment operator (`Nullable`).
	/// @param other `Nullable` to copy from.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType const& other)	{return set(other);				}
	/// @brief Copy assignment operator (`Nullable`).
	/// @param other `Nullable` to move from.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType&& other)			{return set(CTL::move(other));	}

	/// @brief Sets the stored value.
	/// @param value Value to store.
	/// @return Reference to self.
	constexpr SelfType& set(DataType const& value) {
		clear();
		data = value;
		isSet = true;
		return *this;
	}

	/// @brief Sets the stored value.
	/// @param value Value to store.
	/// @return Reference to self.
	constexpr SelfType& set(DataType&& value) {
		clear();
		data = value;
		isSet = true;
		return *this;
	}

	/// @brief Sets the stored value.
	/// @param other `Nullable` to copy from.
	/// @return Reference to self.
	constexpr SelfType& set(SelfType const& other) {
		clear();
		if (other.isSet) {	
			data = other.data;
			isSet = true;
		}
		return *this;
	}

	/// @brief Sets the stored value.
	/// @param other `Nullable` to move from.
	/// @return Reference to self.
	constexpr SelfType& set(SelfType&& other) {
		clear();
		if (other.isSet) {	
			data = CTL::move(other.data);
			isSet = true;
		}
		return *this;
	}	

	/// @brief Clears the current stored value.
	/// @return Reference to self.
	constexpr SelfType& set(NullType) {
		return clear();
	}

	/// @brief Clears the current stored value.
	/// @return Reference to self.
	constexpr SelfType& clear() {
		if constexpr (Type::Constructible<DataType>)
			data = DataType();
		isSet = false;
		return *this;
	}
	
	/// @brief Equality comparison operator.
	/// @param other `Nullable` to compare with.
	/// @return Whether they're equal.
	/// @note Requires element type to be equally comparable.
	/// @sa Comparator::equals()
	constexpr bool operator==(SelfType const& other) const
	requires Type::Comparator::Equals<DataType, DataType> {
		if (!isSet && !other.isSet)
			return true;
		if (isSet && other.isSet)
			return SimpleComparator<DataType>::equals(data, other.data);
		return false;
	}

	/// @brief Equality comparison operator.
	/// @param value Value to compare with.
	/// @return Whether the stored value is equal to `value`.
	/// @note Requires element type to be equally comparable.
	/// @sa Comparator::equals()
	constexpr bool operator==(DataType const& value) const
	requires Type::Comparator::Equals<DataType, DataType> {
		if (isSet)
			return SimpleComparator<DataType>::equals(data, value);
		return false;
	}
	
	/// @brief Equality comparison operator.
	/// @return Whether the value is set.
	constexpr bool operator==(NullType) const {return !exists();}

	/// @brief Threeway comparison operator.
	/// @param other `Nullable` to compare with.
	/// @return Order between both `Nullable`s.
	/// @note Requires value type to be threeway comparable.
	/// @sa Comparator::compare()
	constexpr OrderType operator<=>(SelfType const& other) const
	requires Type::Comparator::Threeway<DataType, DataType>	{
		if (!other.isSet && !isSet)	return Order::EQUAL;
		if (other.isSet && !isSet)	return Order::LESS;
		if (!other.isSet && isSet)	return Order::GREATER;
		return SimpleComparator<DataType>::compare(data, other);
	}

	/// @brief Threeway comparison operator.
	/// @param value Value to compare with.
	/// @return Order between the stored value and `value`.
	/// @note Requires value type to be threeway comparable.
	/// @sa Comparator::compare()
	constexpr OrderType operator<=>(DataType const& value) const
	requires Type::Comparator::Threeway<DataType, DataType>	{
		if (!isSet)	return Order::LESS;
		return SimpleComparator<DataType>::compare(data, value);
	}

	/// @brief Threeway comparison operator.
	/// @param value Value to compare.
	/// @param self `Nullable` to compare with.
	/// @return Order between the stored value and `value`.
	/// @note Requires value type to be threeway comparable.
	/// @sa Comparator::compare()
	friend constexpr OrderType operator<=>(DataType const& value, SelfType const& self)
	requires Type::Comparator::Threeway<DataType, DataType>	{
		if (!self.isSet) return Order::LESS;
		return SimpleComparator<DataType>::compare(value, self.data);
	}

	/// @brief Returns the stored value.
	/// @return Stored value.
	/// @throw Error::NonexistentValue if value is not set.
	constexpr DataType operator*() const {
		return value();
	}

	/// @brief Returns the stored value.
	/// @return Stored value.
	/// @throw Error::NonexistentValue if value is not set.
	constexpr operator DataType() const
	requires Type::Different<DataType, bool> {
		return value();
	}

private:
	/// @brief Underlying data type.
	DataType data;
	/// @brief Whether there is data being stored.
	bool isSet = false;
};

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_NULLABLE_H
