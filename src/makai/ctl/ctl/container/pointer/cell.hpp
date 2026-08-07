#ifndef CTL_CONTAINER_POINTER_CELL_H
#define CTL_CONTAINER_POINTER_CELL_H

#include "../../namespace.hpp"
#include "../../templates.hpp"
#include "../../typeinfo.hpp"
#include "../../ctypes.hpp"
#include "../../order.hpp"
#include "../../cpperror.hpp"
#include "../../typetraits/traits.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Type-exclusive shared pointer.
/// @tparam TData Type of data pointed to.
/// @note
///		Differences between this and `Shared<T>`:
///		- `Shared<T>` handles references for any type (better suited for classes with virtual members)
///		- `Shared<T>` `Shared<T>` is slower (Global sync lock, `Cell<T>` has no sync lock)
/// @caution
/// 	*This class is thread-unsafe!*
/// 	If you need thread safety, use `AtomicCell<T>`.
template <class TData>
struct Cell:
	Typed<TData>,
	SelfIdentified<Cell<TData>>,
	Ordered {
	using Typed				= ::CTL::Typed<TData>;
	using SelfIdentified	= ::CTL::SelfIdentified<Cell<TData>>;

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

	/// @brief Value wrapper.
	struct Wrapper {
		/// @brief Underlying value.
		DataType	value;
		/// @brief Count of reference to value.
		usize		refs;
	};

	/// @brief Empty constructor.
	constexpr Cell()			{}
	/// @brief Empty constructor.
	constexpr Cell(nulltype)	{}

	/// @brief Copy constructor (`Cell`).
	/// @param obj Cell to reference.
	constexpr Cell(SelfType const& other): wrapper(other.wrapper) {
		if (!exists()) return;
		wrapper->acquire();
	}

	/// @brief Move constructor (`Cell`).
	/// @param obj Cell to reference.
	constexpr Cell(SelfType&& other): wrapper(move(other.wrapper)) {
		other.wrapper = nullptr;
	}

	/// @brief Copy assignment operator (`Cell`).
	/// @param obj Cell to reference.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType const& other) {
		if (wrapper == other.wrapper) return *this;
		//unbind();
		wrapper = other.wrapper;
		if (!other.exists()) return *this;
		auto const _ = wrapper->lock();
		wrapper->acquire();
		return *this;
	}

	/// @brief Move assignment operator.
	/// @param obj Cell to reference.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType&& other) {
		swap(wrapper, other.wrapper);
		return *this;
	}

	/// @brief `swap` algorithm.
	friend constexpr void swap(SelfType& a, SelfType& b) noexcept {
		swap(a.wrapper, b.wrapper);
	}

	/// @brief Destructor.
	constexpr ~Cell() {
		unbind();
	}

	/// @brief Returns a pointer to the underlying value.
	/// @return Pointer to value.
	constexpr PointerType operator->() const {
		if (!exists()) emptyError();
		return &wrapper->value;
	}

	/// @brief Returns a reference to the underlying value.
	/// @return Reference to value.
	constexpr ReferenceType operator*() const {
		if (!exists()) emptyError();
		return wrapper->value;
	}

	/// @brief Creates a cell.
	/// @tparam ...Args Argument types.
	/// @param ...args Arguments to pass to object construtor.
	/// @return Cell to value.
	template <class... TArgs>
	constexpr static SelfType create(TArgs... args) {
		SelfType cell;
		cell.wrapper = new Wrapper{.value = DataType(args...), .refs = 1};
		return cell;
	}

	/// @brief Modifies the underlying value.
	/// @tparam TFunction Operation type.
	/// @tparam op Operation to perform.
	/// @return Reference to self.
	template<Type::Functional<DataType(DataType const&)> TFunction>
	constexpr SelfType& modify(TFunction const& op) {
		if (!exists()) return *this;
		wrapper->value = op(wrapper->value);
		return (*this);
	}

	/// @brief Atomically performs an operation synchronized to the underlying value.
	/// @tparam TFunction Operation type.
	/// @tparam op Operation to perform.
	/// @return Reference to self.
	template<Type::Functional<void(DataType const&)> TFunction>
	constexpr SelfType& perform(TFunction const& op) {
		if (!exists()) return *this;
		op(wrapper->value);
		return (*this);
	}

	/// @brief Equality comparison operator (`Cell`).
	/// @param obj `Cell` to compare to.
	/// @return Whether they're equal.
	constexpr bool operator==(SelfType const& other) const			{return wrapper == other.wrapper;	}
	/// @brief Threeway comparison operator (`Cell`).
	/// @param obj `Cell` to compare to.
	/// @return Order between objects.
	constexpr OrderType operator<=>(SelfType const& other) const	{return wrapper <=> other.wrapper;	}

	/// @brief Returns whether the object exists.
	/// @return Whether object exists.
	constexpr bool exists()		const {return (wrapper && wrapper->refs);		}
	/// @brief Returns whether the object exists.
	/// @return Whether object exists.
	constexpr operator bool()	const {return exists();							}

	/// @brief Returns whether this cell is the sole owner of the bound object.
	/// @return Whether this cell is the sole owner of the bound object.
	constexpr bool unique()		const {return (wrapper && wrapper->refs == 1);	}

private:
	constexpr void unbind() {
		if (!exists()) return;
		wrapper->release();
		if (!wrapper->refs)
			delete wrapper;
		wrapper = nullptr;
	}

	[[noreturn]] constexpr static void emptyError() {
		throw NullPointerException("Atomic cell is empty!");
	}
	// Wrapper.
	ptr<Wrapper>	wrapper = nullptr;
};

CTL_NAMESPACE_END

#endif
