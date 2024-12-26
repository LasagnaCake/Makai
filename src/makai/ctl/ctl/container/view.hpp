#ifndef CTL_CONTAINER_VIEW_H
#define CTL_CONTAINER_VIEW_H

#include "../ctypes.hpp"
#include "../typetraits/traits.hpp"
#include "../templates.hpp"
#include "../namespace.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Container-specific type constraints.
namespace Type::Container {
	/// @brief Type must be a viewable type.
	template <typename T> concept Viewable = Safe<T>;
}

/// @brief Specialized reference wrapper.
/// @tparam T Reference type.
template<Type::Container::Viewable T>
class View: Typed<T>, SelfIdentified<View<T>> {
public:
	using Typed				= Typed<T>;
	using SelfIdentified	= SelfIdentified<View<T>>;

	using
		typename Typed::DataType,
		typename Typed::ReferenceType,
		typename Typed::ConstReferenceType
	;

	using
		typename SelfIdentified::SelfType
	;

	/// @brief Operation type.
	using OperationType = Decay::AsFunction<DataType(ConstReferenceType)>;
	
	/// @brief Binds a reference.
	/// @param data Reference to bind.
	constexpr View(ReferenceType data):	data(data)				{}
	/// @brief Move constructor (`View`).
	/// @param other `View` to move.
	constexpr View(SelfType&& other):		data(other.data)	{}
	/// @brief Copy constructor (`View`).
	/// @param other `View` to copy from.
	constexpr View(SelfType const& other):	data(other.data)	{}

	/// @brief Copy assignment operator (value type).
	/// @param val Value to store in bound reference.
	/// @return Reference to self.
	constexpr SelfType& operator=(T const& val)	{data = val; return (*this);			}
	/// @brief Move assignment operator (value type).
	/// @param val Value to store in bound reference.
	/// @return Reference to self.
	constexpr SelfType& operator=(T&& val)		{data = CTL::move(val); return (*this);	}

	/// @brief Access (and modifies) the underlying bound reference.
	/// @tparam TFunction Function type.
	/// @param op Operation to perform.
	/// @return Reference to self.
	template <Type::Functional<OperationType> TFunction>
	constexpr SelfType& modify(TFunction const& op)		{data = op(data); return (*this);	}
	/// @brief Access (and modifies) the underlying bound reference.
	/// @tparam TFunction Function type.
	/// @param op Operation to perform.
	/// @return Reference to self.
	template <Type::Functional<OperationType> TFunction>
	constexpr SelfType& operator()(TFunction const& op)	{return modify(op);					}

	/// @brief Returns the value of the bound reference.
	/// @return Value of the bound reference.
	constexpr operator DataType() const	{return value();	}
	/// @brief Returns the value of the bound reference.
	/// @return Value of the bound reference.
	constexpr operator DataType()		{return value();	}
	/// @brief Returns the value of the bound reference.
	/// @return Value of the bound reference.
	constexpr DataType value() const	{return data;		}
	/// @brief Returns the value of the bound reference.
	/// @return Value of the bound reference.
	constexpr DataType value() 			{return data;		}

protected:
	/// @brief Returns the bound reference.
	/// @return Bound reference.
	constexpr ReferenceType view()				{return data;}
	/// @brief Returns the bound reference.
	/// @return Bound reference.
	constexpr ConstReferenceType view() const	{return data;}

private:
	/// @brief Bound reference associated with the view.
	T& data;
};

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_VIEW_H
