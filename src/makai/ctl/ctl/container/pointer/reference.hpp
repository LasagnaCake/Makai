#ifndef CTL_CONTAINER_POINTER_REFERENCE_H
#define CTL_CONTAINER_POINTER_REFERENCE_H

#include "../../namespace.hpp"
#include "../../templates.hpp"
#include "../../cpperror.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Specialized reference.
/// @tparam T Pointed type.
template<class T>
struct Reference:
	Typed<T>,
	SelfIdentified<Reference<T>>,
	Ordered {
	using Typed				= ::CTL::Typed<T>;
	using SelfIdentified	= ::CTL::SelfIdentified<Reference<T>>;

	using
		typename Typed::DataType,
		typename Typed::ConstantType,
		typename Typed::ReferenceType,
		typename Typed::ConstReferenceType,
		typename Typed::PointerType,
		typename Typed::ConstPointerType
	;

	using
		typename SelfIdentified::SelfType
	;

	using
		typename Ordered::OrderType
	;

	/// @brief Default constructor.
	constexpr Reference() {}

	/// @brief Move constructor (defaulted).
	/// @param other `Reference` to move.
	constexpr Reference(Reference&& other)		= default;

	/// @brief Move constructor (defaulted)
	/// @param other `Reference` to copy from.
	constexpr Reference(Reference const& other)	= default;
	
	/// @brief Copy constructor (raw pointer).
	/// @param obj Pointer to bind.
	constexpr Reference(ref<DataType> const& obj): ref(obj) {}

	/// @brief Statically casts the pointer to point to a new type.
	/// @tparam TNew New object type.
	/// @return Reference to new object type.
	template<Type::Container::Pointable TNew>
	constexpr Reference<TNew>		as() const			{return	static_cast<TNew*>(raw());		}
	/// @brief Dynamically casts the pointer to point to a new type.
	/// @tparam TNew New object type.
	/// @return Reference to new object type.
	template<Type::Container::Pointable TNew>
	constexpr Reference<TNew>		polymorph() const	{return	dynamic_cast<TNew*>(raw());		}
	/// @brief Reinterprets the pointer as a different pointer type.
	/// @tparam TNew New object type.
	/// @return Reference to new object type.
	template<Type::Container::Pointable TNew>
	constexpr Reference<TNew>		reinterpret() const	{return	reinterpret_cast<TNew*>(raw());	}
	/// @brief Returns a raw pointer to the bound object.
	/// @return Raw pointer to bound object.
	constexpr ref<DataType>			raw() const			{return	ref;							}

	/// @brief Returns whether the bound object exists.
	/// @return Whether the bound object exists.
	constexpr operator bool() const		{return exists();	}
	/// @brief Returns whether the bound object doesn't exist.
	/// @return Whether the bound object doesn't exist.
	constexpr bool operator!() const	{return	!exists();	}
	
	/// @brief Equality comparison operator (raw pointer).
	/// @param obj Raw pointer to compare to.
	/// @return Whether they're equal.
	constexpr bool operator==(PointerType const& obj) const			{return	ref == obj;			}
	/// @brief Threeway comparison operator (raw pointer).
	/// @param obj Raw pointer to compare to.
	/// @return Order between objects.
	constexpr OrderType operator<=>(PointerType const& obj) const	{return	ref <=> obj;		}
	
	/// @brief Equality comparison operator (`Pointer`).
	/// @param obj `Pointer` to compare to.
	/// @return Whether they're equal.
	constexpr bool operator==(SelfType const& other) const			{return ref == other.ref;	}
	/// @brief Threeway comparison operator (`Pointer`).
	/// @param obj `Pointer` to compare to.
	/// @return Order between objects.
	constexpr OrderType operator<=>(SelfType const& other) const	{return ref <=> other.ref;	}

	/// @brief Assignment operator.
	/// @param obj Object to reference.
	/// @return Reference to self.
	constexpr SelfType& operator=(PointerType const& obj)	{ref = (obj); return (*this);		}
	/// @brief Copy assignment operator (defaulted).
	/// @param obj `Reference` to copy from.
	/// @return Reference to self.
	constexpr Reference& operator=(Reference const& other)	= default;
	/// @brief Move assignment operator (defaulted).
	/// @param obj `Reference` to move.
	/// @return Reference to self.
	constexpr Reference& operator=(Reference&& other)		= default;
	
	/// @brief Returns a raw pointer to the bound object.
	/// @return Raw pointer to bound object.
	constexpr PointerType operator&() const {return raw();}

	/// @brief Returns the value pointed to.
	/// @return Reference to object being pointed to.
	constexpr ReferenceType value() const {
		if (!exists()) nullPointerError();
		return (*ref);
	}
	
	/// @brief Pointer member access operator.
	/// @return Underlying pointer.
	constexpr PointerType operator->()				{return getPointer();	}
	/// @brief Pointer member access operator.
	/// @return Underlying pointer.
	constexpr PointerType const operator->() const	{return getPointer();	}
	/// @brief Dereference operator.
	/// @return Reference to underlying object.
	constexpr ReferenceType operator*() const		{return value();		}
	
	/// @brief Returns whether the object exists.
	/// @return Whether the object exists.
	constexpr bool exists() const {
		return ref;
	}

	/// @brief Returns a raw pointer to the bound object.
	/// @return Raw pointer to bound object.
	constexpr operator ref<DataType> const() const	{return raw();		}

	/// @brief Returns a raw pointer to the bound object.
	/// @return Raw pointer to bound object.
	constexpr operator ref<DataType>()				{return raw();		}

private:
	/// @brief Pointer to referenced object.
	PointerType ref = nullptr;

	constexpr PointerType getPointer() {
		if (!exists()) nullPointerError();
		return (ref);
	}

	constexpr PointerType getPointer() const {
		if (!exists()) nullPointerError();
		return (ref);
	}

	[[noreturn]]
	inline void nullPointerError() const {
		throw NullPointerException("No reference was bound to this object!");
	}
};

template<class T>
struct RefRef {
	constexpr operator Reference<T>() const {return *refref;}
private:
	T** refref = nullptr;
};

CTL_NAMESPACE_END

#endif