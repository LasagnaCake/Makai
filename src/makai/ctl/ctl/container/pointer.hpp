#ifndef CTL_CONTAINER_POINTER_H
#define CTL_CONTAINER_POINTER_H

#include "../namespace.hpp"
#include "../templates.hpp"
#include "../typeinfo.hpp"
#include "../ctypes.hpp"
#include "../typetraits/traits.hpp"
#include "../algorithm/strconv.hpp"
#include "error.hpp"
#include "function.hpp"
#include "map.hpp"

CTL_NAMESPACE_BEGIN

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"

// TODO: make this thread-safe

/// @brief Container-specific type constraints.
namespace Type::Container {
	/// Type must be a pointable type.
	template <typename T> concept Pointable = Safe<T>;
}

/// @brief Base classes for other classes.
namespace Base {
	/// @brief Reference counter.
	/// @tparam T Reference type.
	template <class T>
	struct ReferenceCounter {
		/// @brief Reference data.
		struct Reference {
			/// @brief Whether the reference exists.
			bool	exists	= false;
			/// @brief How many references are being held.
			ssize	count	= 0;

			/// @brief Empty constructor.
			constexpr Reference() = default;

			/// @brief Basic constructor. 
			constexpr Reference(bool const exists, int64 const count = 0): exists(exists), count(count) {}
		};

		/// @brief Underlying database type.
		using Database = Map<T, Reference>;

		/// @brief Checks whether the reference is bound.
		/// @param ptr Reference to check.
		/// @return Whether it is bound.
		inline static bool isBound(T const& ptr) {return database.contains(ptr);}

	protected:
		/// @brief Underlying reference database.
		inline static Database database;
	};
}

#define CTL_PTR_ASSERT_STRONG_MOVE	static_assert(!WEAK,	"It is forbidden to move construct a weak pointer from a strong pointer!")
#define CTL_PTR_ASSERT_STRONG		static_assert(!WEAK,	"It is forbidden to implicitly convert a strong pointer to a weak pointer!")
#define CTL_PTR_ASSERT_WEAK			static_assert(WEAK,		"It is forbidden to implicitly convert a weak pointer to a strong pointer!")
#define CTL_PTR_IF_STRONG			if constexpr(!WEAK)
/// @brief Smart pointer, with automatic reference counting.
/// @tparam T Type of data pointed to.
/// @tparam W Whether the given `Pointer` is weak.
/// @note
///		Differences between strong and weak pointers:
/// @note
///		- Weak pointer:
///			Only points to some object in memory, and does not take ownership of it.
///			Does not affect the reference counter.
/// @note
///		- Strong pointer:
///			Takes full ownership of the object pointed to it, and can destroy (delete) it.
///			Will throw if object is no longer usable, either via releasing the pointer to it,
///			or when another strong pointer destroys it.
///			Changes the reference counter when an object is bound, unbound, destroyed, etc.
/// @note
///			Both types will throw if object is no longer usable,
///			either via releasing the pointer to it, or when a strong pointer destroys it.
template <Type::Container::Pointable T, bool W>
class Pointer:
	private Base::ReferenceCounter<void*>,
	Derived<Base::ReferenceCounter<void*>>,
	Typed<T>,
	SelfIdentified<Pointer<T, W>>,
	Ordered {
public:
	/// @brief Whether the pointer is a strong or weak pointer.
	constexpr static bool WEAK = W;

	using ReferenceCounter::isBound;

	using Typed				= ::CTL::Typed<T>;
	using SelfIdentified	= ::CTL::SelfIdentified<Pointer<T, WEAK>>;

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

	/// @brief New smart pointer type.
	/// @tparam NW Whether the pointer is weak.
	template<bool NW>
	using NewPointerType = Pointer<DataType, NW>;

	/// @brief Opposite smart pointer type.
	using OtherType = NewPointerType<!WEAK>;

	/// @brief Operation type.
	using OperationType = Decay::AsFunction<DataType(ConstReferenceType)>;

	/// @brief Empty constructor.
	constexpr Pointer() {}
	
	/// @brief Move constructor (strong pointer).
	/// @param other Strong pointer to bind.
	constexpr Pointer(NewPointerType<false>&& other)		{CTL_PTR_ASSERT_STRONG_MOVE;	bind(other);}
	/// @brief Move constructor (weak pointer).
	/// @param other Weak pointer to bind.
	constexpr Pointer(NewPointerType<true>&& other)			{CTL_PTR_ASSERT_WEAK;			bind(other);}

	/// @brief Copy constructor (strong pointer).
	/// @param other Strong pointer to bind.
	constexpr Pointer(NewPointerType<false> const& other)	{								bind(other);}
	/// @brief Move constructor (weak pointer).
	/// @param other Weak pointer to bind.
	constexpr Pointer(NewPointerType<true> const& other)	{CTL_PTR_ASSERT_WEAK;			bind(other);}

	/// @brief Copy constructor (raw pointer).
	/// @param obj Pointer to bind.
	constexpr Pointer(PointerType const& obj) {bind(obj);}

	/// @brief Destructor.
	constexpr ~Pointer() {if (exists()) unbind();}

	/// @brief Returns the amount of references holding the current object.
	/// @return Reference count.
	constexpr ssize count() const {
		if (!exists()) return 0;
		return database[ref].count;
	}

	/// @brief Sets the pointer as a reference to an object.
	/// @param obj Object to reference.
	/// @return Reference to self.
	constexpr SelfType& bind(PointerType const& obj) {
		if (ref == obj) return (*this);
		unbind();
		if (!obj) return (*this);
		attach(obj);
		return (*this);
	}

	/// @brief Sets the pointer as a reference to an object.
	/// @param ptr Object to reference.
	/// @return Reference to self.
	constexpr SelfType& bind(SelfType const& ptr) {
		if (ref == ptr) return (*this);
		unbind();
		if (!ptr) return (*this);
		attach(ptr.ref);
		return (*this);
	}

	/// @brief Sets the pointer as a reference to an object.
	/// @param ptr Object to reference.
	/// @return Reference to self.
	constexpr SelfType& bind(OtherType const& ptr) {
		CTL_PTR_ASSERT_WEAK;
		if (ref == ptr) return (*this);
		unbind();
		if (!ptr) return (*this);
		attach(ptr.ref);
		return (*this);
	}
	
	/// @brief Removes the pointer as a reference to a bound object.
	/// @return Reference to self.
	constexpr SelfType& unbind() {
		if (!exists()) return (*this);
		CTL_PTR_IF_STRONG {
			if ((count() < 2))
				destroy();
			else database[(void*)ref].count--;
		}
		ref = nullptr;
		return (*this);
	}

	/// @brief Destroys (deletes) the bound object.
	/// @return Reference to self.
	constexpr SelfType& destroy() requires (!WEAK) {
		if (!exists()) return (*this);
		release();
		delete ref;
		ref = nullptr;
		return (*this);
	}

	/// @brief Detaches the bound object from the reference system.
	/// @return Reference to self.
	/// @note Requires smart pointer type to be strong.
	constexpr SelfType& release() requires (!WEAK) {
		if (!exists())
			detach(ref);
		return (*this);
	}

	/// @brief Detaches a given object from the reference system.
	/// @param ptr Object to detach.
	/// @note Requires smart pointer type to be strong.
	constexpr static void detach(PointerType const& ptr)
	requires (!WEAK) {
		if (isBound(ptr))
			database[(void*)ptr] = {false, 0};
	}

	/// @brief Returns whether the object exists.
	/// @return Whether the object exists.
	constexpr bool exists() const {
		if (!ref) return false;
		CTL_PTR_IF_STRONG	return (database[(void*)ref].count > 0);
		else				return (database[(void*)ref].exists);
	}

	/// @brief Returns whether this pointer is the sole owner of the bound object.
	/// @return Whether this pointer is the sole owner of the bound object.
	/// @note Requires smart pointer type to be strong.
	constexpr bool unique() const requires (!WEAK) {return count() == 1;}
	
	/// @brief Returns whether the object exists.
	/// @return Whether the object exists.
	constexpr bool operator()() const {return exists();}

//	constexpr PointerType operator&()		{return raw();}
	/// @brief Returns a raw pointer to the bound object.
	/// @return Raw pointer to bound object.
	constexpr PointerType operator&() const {return raw();}

	/// @brief Casts the smart pointer to point to a new type.
	/// @tparam New object type.
	/// @return Smart pointer to new object type.
	template<Type::Container::Pointable TNew>
	constexpr Pointer<TNew, WEAK>		as() const		{return	static_cast<TNew*>(raw());			}
	/// @brief Returns a weak pointer to the bound object.
	/// @return Weak pointer to object.
	constexpr Pointer<DataType, true>	asWeak() const	{return	raw();								}
	/// @brief Returns a raw pointer to the bound object.
	/// @return Raw pointer to bound object.
	constexpr PointerType				raw() const		{return	exists() ? getPointer() : nullptr;	}
//	constexpr PointerType				raw() const		{return	getPointer();						}
//	constexpr ConstPointerType			raw() const		{return	getPointer();						}
	
	/// @brief Returns a raw pointer to the bound object.
	/// @return Raw pointer to bound object.
	/// @note Conversion is explicit if smart pointer type is strong.
	constexpr explicit(!WEAK) operator PointerType const() const	{return raw();		}
	/// @brief Returns a raw pointer to the bound object.
	/// @return Raw pointer to bound object.
	/// @note Conversion is explicit if smart pointer type is strong.
	constexpr explicit(!WEAK) operator PointerType()				{return raw();		}

	/// @brief Returns whether the bound object exists.
	/// @return Whether the bound object exists.
	constexpr operator bool() const	{return exists();	}

	template<Type::Functional<OperationType> TFunction>
	constexpr Pointer& modify(TFunction const& op)		{T& ref = *getPointer(); ref = op(ref); return (*this);	}
	template<Type::Functional<OperationType> TFunction>
	constexpr Pointer& operator()(TFunction const& op)	{return modify(op);										}

	/// @brief Returns whether the bound object doesn't exist.
	/// @return Whether the bound object doesn't exist.
	constexpr bool operator!() const	{return	!exists();			}
	
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
	constexpr SelfType& operator=(PointerType const& obj)	{bind(obj); return (*this);					}
	/// @brief Assignment operator.
	/// @param obj Object to reference.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType const& other)	{bind(other.ref); return (*this);			}
//	constexpr SelfType& operator=(DataType const& v) const	{*getPointer() = v; return (*this);			}
//	constexpr SelfType& operator=(DataType const& v) const	{if (exists()) *ref = v; return (*this);	}

	/// @brief Returns the value pointed to.
	/// @return Reference to object being pointed to.
	constexpr ReferenceType value() const {
		if (!exists()) nullPointerError();
		return (*ref);
	}

	/// @brief Returns the value pointed to.
	/// @return Value being pointed to.
	constexpr explicit operator DataType() const	{return value();		}

	/// @brief Pointer member access operator.
	/// @return Underlying pointer.
	constexpr PointerType operator->()				{return getPointer();	}
	/// @brief Pointer member access operator.
	/// @return Underlying pointer.
	constexpr PointerType const operator->() const	{return getPointer();	}
	/// @brief Dereference operator.
	/// @return Reference to underlying object.
	constexpr ReferenceType operator*() const		{return value();		}

private:
	constexpr void attach(PointerType const& p) {
		if (!p) return;
		ref = p;
		database[(void*)p].exists = true;
		CTL_PTR_IF_STRONG database[(void*)p].count++;
	}

	friend SelfType;
	friend OtherType;

	using ReferenceCounter::database;

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
		throw Error::NullPointer(
			toString("Pointer reference of type '", TypeInfo<T>::name(), "' does not exist!"),
			"Pointer might be null or nonexistent.",
			"none",
			CTL_CPP_UNKNOWN_SOURCE
		);
	}
};
#undef CTL_PTR_ASSERT_STRONG_MOVE
#undef CTL_PTR_ASSERT_STRONG
#undef CTL_PTR_ASSERT_WEAK
#undef CTL_PTR_IF_STRONG

/// @brief `Pointer` analog for a weak pointer.
/// @tparam T Type of data pointed to.
template <Type::Container::Pointable T>
using WeakPointer	= Pointer<T, true>;

/// @brief `Pointer` analog for a strong pointer.
/// @tparam T Type of data pointed to.
template <Type::Container::Pointable T>
using StrongPointer	= Pointer<T, false>;

/// @brief `StrongPointer` analog for a managed instance of an object.
/// @tparam T Type of data pointed to.
template<Type::Container::Pointable T>
using Instance	= StrongPointer<T>;

/// @brief `WeakPointer` analog for a handle to an object.
/// @tparam T Type of data pointed to.
template<Type::Container::Pointable T>
using Handle	= WeakPointer<T>;

#pragma GCC diagnostic pop

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_POINTER_H
