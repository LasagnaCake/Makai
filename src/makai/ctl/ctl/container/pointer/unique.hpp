#ifndef CTL_CONTAINER_POINTER_UNIQUE_H
#define CTL_CONTAINER_POINTER_UNIQUE_H

#include "../../namespace.hpp"
#include "../../templates.hpp"

#include "../../memory/deleter.hpp"

#include "makai/ctl/ctl/typetraits/basictraits.hpp"
#include "reference.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Container-specific type constraints.
namespace Type::Container {
	/// @brief Type must be a pointable type.
	template <typename T> concept Pointable = Safe<T>;
}

/// @brief Unique pointer.
/// @tparam TData Pointed type.
/// @tparam D Deleter.
template<Type::Safe TData, auto D = Deleter<TData>()>
struct Unique:
	Typed<TData>,
	SelfIdentified<Unique<TData>>,
	Ordered,
	Deletable<D, TData> {
	using Typed				= ::CTL::Typed<TData>;
	using SelfIdentified	= ::CTL::SelfIdentified<Unique<TData>>;
	using Deletable			= ::CTL::Deletable<D, TData>;

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

	using Deletable::deleter;

	/// @brief Default constructor.
	constexpr Unique() {}

	/// @brief Move constructor.
	/// @param other `Unique` to move.
	constexpr Unique(Unique&& other) {
		ref = other.ref;
		other.ref = nullptr;
	}

	/// @brief Copy constructor (deleted)
	constexpr Unique(Unique const& other)	= delete;

	/// @brief Destructor.
	constexpr ~Unique() {unbind();}
	
	/// @brief Constructs the unique pointer from an unmanaged object.
	/// @param obj Object to bind.
	constexpr explicit Unique(owner<DataType> const& obj): ref(obj) {}
	
	/// @brief Returns a `Reference` to the underlying data.
	/// @return `Reference` to underlying data.
	constexpr Reference<DataType> reference() const {return	ref;}

	/// @brief Statically casts the pointer to point to a new type.
	/// @tparam TNew New object type.
	/// @return `Reference` to new object type.
	template<Type::Container::Pointable TNew>
	constexpr Reference<TNew>	as() const			{return	static_cast<TNew*>(raw());		}
	/// @brief Dynamically casts the pointer to point to a new type.
	/// @tparam TNew New object type.
	/// @return `Reference` to new object type.
	template<Type::Container::Pointable TNew>
	constexpr Reference<TNew>	morph() const		{return	dynamic_cast<TNew*>(raw());		}
	/// @brief Reinterprets the pointer as a different pointer type.
	/// @tparam TNew New object type.
	/// @return `Reference` to new object type.
	template<Type::Container::Pointable TNew>
	constexpr Reference<TNew>	reinterpret() const	{return	reinterpret_cast<TNew*>(raw());	}
	/// @brief Reinterprets the pointer as a pointer type with different constness and volatileness.
	/// @tparam TNew New object type.
	/// @return Reference to new object type.
	template<class TNew = AsNonConst<DataType>>
	constexpr Reference<TNew>	mutate() const		{return	const_cast<TNew*>(raw());		}
	/// @brief Returns a raw pointer to the bound object.
	/// @return Raw pointer to bound object.
	constexpr ref<DataType>		raw() const			{return	ref;							}

	/// @brief Relinquishes ownership of the bound object.
	/// @return Pointer to object.
	constexpr owner<DataType> release() {
		auto* r = ref;
		ref = nullptr;
		return r;
	}

	/// @brief Transfers ownership of the bound object.
	/// @return Temporary `Unique` to object.
	constexpr SelfType transfer() {
		return SelfType(release());
	}

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

	/// @brief Copy assignment operator (deleted).
	constexpr Unique& operator=(Unique const& other)	= delete;
	
	/// @brief Move assignment operator.
	/// @param other `Unique` to copy from.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType&& other) {
		ref = other.ref;
		other.ref = nullptr;
		return *this;
	}
	
	/// @brief Returns a raw pointer to the bound object.
	/// @return Raw pointer to bound object.
	constexpr PointerType operator&() const {return raw();}

	/// @brief Returns the value pointed to.
	/// @return Unique to object being pointed to.
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
	/// @return Unique to underlying object.
	constexpr ReferenceType operator*() const		{return value();		}
	
	/// @brief Returns whether the object exists.
	/// @return Whether the object exists.
	constexpr bool exists() const {
		return ref;
	}

	/// @brief Returns a raw pointer to the bound object.
	/// @return Raw pointer to bound object.
	constexpr explicit operator ref<DataType> const() const	{return raw();		}

	/// @brief Returns a raw pointer to the bound object.
	/// @return Raw pointer to bound object.
	constexpr explicit operator ref<DataType>()				{return raw();		}

	/// @brief Binds an object to the pointer.
	/// @param ptr Object to bind.
	/// @return Reference to self.
	constexpr SelfType& bind(owner<DataType> const ptr) {
		if (ref == ptr) return (*this);
		unbind();
		if (!ptr) return (*this);
		ref = ptr;
		return (*this);
	}
	
	/// @brief Unbinds the bound object.
	/// @return Reference to self.
	constexpr SelfType& unbind() {
		if (!exists()) return (*this);
		deleter(ref);
		ref = nullptr;
		return (*this);
	}

	/// @brief Creates an unique pointer.
	/// @tparam ...Args Argument types.
	/// @param ...args Arguments to pass to object construtor.
	/// @return Unique pointer.
	template<class... Args>
	constexpr static SelfType create(Args&&... args) {
		return SelfType(new DataType(forward<Args>(args)...));
	}

	/// @brief Creates an unique pointer.
	/// @tparam TDerived Derived type.
	/// @tparam ...Args Argument types.
	/// @param ...args Arguments to pass to object construtor.
	/// @return Unique pointer.
	template<Type::Derived<DataType> TDerived, class... Args>
	constexpr static SelfType create(Args&&... args) {
		return SelfType(new TDerived(forward<Args>(args)...));
	}

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

CTL_NAMESPACE_END

#endif