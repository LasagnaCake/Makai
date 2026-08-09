#ifndef CTL_CONTAINER_POINTER_ATOMICCELL_H
#define CTL_CONTAINER_POINTER_ATOMICCELL_H

#include "../../namespace.hpp"
#include "../../templates.hpp"
#include "../../typeinfo.hpp"
#include "../../ctypes.hpp"
#include "../../order.hpp"
#include "../../cpperror.hpp"
#include "../../typetraits/traits.hpp"
#include "../../async/lock.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Type-exclusive thread-safe shared pointer.
/// @tparam TData Type of data pointed to.
/// @note
///		Differences between this and `Shared<T>`:
///		- `Shared<T>` handles references for any type (better suited for classes with virtual members)
///		- `Shared<T>` `Shared<T>` is slower (Global sync lock vs per-value sync lock)
template <class TData>
struct AtomicCell:
	Typed<TData>,
	SelfIdentified<AtomicCell<TData>>,
	Ordered {
	using Typed				= ::CTL::Typed<TData>;
	using SelfIdentified	= ::CTL::SelfIdentified<AtomicCell<TData>>;

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
		/// @brief Thread synchronization barrier.
		Mutex		oplock;
		/// @brief Underlying value.
		DataType	value;
		/// @brief Count of reference to value.
		usize		refs;

		/// @brief Creates a scope-bound lock.
		/// @return Scope lock.
		[[nodiscard]]
		constexpr auto lock()		{return ScopeLock<Mutex>(oplock);		}
		/// @brief Increments the reference counter, if applicable.
		constexpr void acquire()	{if (refs < Limit::MAX<usize>) ++refs;	}
		/// @brief Decrements the reference counter, if applicable.
		constexpr void release()	{if (refs) --refs;						}
	};

	/// @brief Empty constructor.
	constexpr AtomicCell()			{}
	/// @brief Empty constructor.
	constexpr AtomicCell(nulltype)	{}

	/// @brief Copy constructor (`AtomicCell`).
	/// @param obj Cell to reference.
	constexpr AtomicCell(SelfType const& other): wrapper(other.wrapper) {
		if (!exists()) return;
		auto const _ = wrapper->lock();
		wrapper->acquire();
	}

	/// @brief Move constructor (`AtomicCell`).
	/// @param obj Cell to reference.
	constexpr AtomicCell(SelfType&& other): wrapper(displace(other.wrapper)) {}

	/// @brief Copy assignment operator.
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
	~AtomicCell() {
		unbind();
	}

	/// @brief Returns a pointer to the underlying value.
	/// @return Pointer to value.
	/// @throw `NullPointerException` if object does not exist.
	constexpr PointerType operator->() const {
		if (!exists()) emptyError();
		auto const _ = wrapper->lock();
		return &wrapper->value;
	}

	/// @brief Returns a reference to the underlying value.
	/// @return Reference to value.
	/// @throw `NullPointerException` if object does not exist.
	constexpr ReferenceType operator*() const {
		if (!exists()) emptyError();
		auto const _ = wrapper->lock();
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

	/// @brief Atomically modifies the underlying value.
	/// @tparam TFunction Operation type.
	/// @tparam op Operation to perform.
	/// @return Reference to self.
	template<Type::Functional<DataType(DataType const&)> TFunction>
	constexpr SelfType& modify(TFunction const& op) {
		if (!exists()) return *this;
		auto const _ = wrapper->lock();
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
		auto const _ = wrapper->lock();
		op(wrapper->value);
		return (*this);
	}

	/// @brief Atomically performs an operation synchronized to the underlying value.
	/// @tparam TFunction Operation type.
	/// @tparam op Operation to perform.
	/// @return Reference to self.
	template<Type::Functional<void()> TFunction>
	constexpr SelfType& perform(TFunction const& op) {
		if (!exists()) {
			op();
			return *this;
		}
		auto const _ = wrapper->lock();
		op();
		return (*this);
	}

	/// @brief Creates a synchronization barrier bound to the cell's value.
	/// @return Sync barrier.
	/// @throw `NullPointerException` if object does not exist.
	[[nodiscard]]
	constexpr ScopeLock<Mutex> sync() 		{return exists() ? wrapper->lock() : lock(); }
	/// @brief Creates a synchronization barrier bound to the cell's value.
	/// @return Sync barrier.
	/// @throw `NullPointerException` if object does not exist.
	[[nodiscard]]
	constexpr ScopeLock<Mutex> sync() const	{if (exists()) return wrapper->lock(); emptyError();}

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

	/// @brief Creates a scope-bound lock bound to the cells own mutex.
	/// @return Scope lock.
	[[nodiscard]]
	constexpr ScopeLock<Mutex>	lock()	{return ScopeLock<Mutex>(mtx);	}
	/// @brief Returns the cell's own mutex.
	/// @return Mutex.
	constexpr Mutex&			mutex() {return mtx;					}

private:
	constexpr void unbind() {
		if (!exists()) return;
		wrapper->oplock.capture();
		wrapper->release();
		if (!wrapper->refs) {
			wrapper->oplock.release();
			delete wrapper;
		} else wrapper->oplock.release();
		wrapper = nullptr;
	}

	[[noreturn]] constexpr static void emptyError() {
		throw NullPointerException("Atomic cell is empty!");
	}
	ptr<Wrapper>	wrapper = nullptr;
	Mutex			mtx;
};

CTL_NAMESPACE_END

#endif
