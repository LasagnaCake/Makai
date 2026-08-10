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
// For debugging purposes
#include <stdio.h>

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
		constexpr auto lock()		{return ScopeLock<Mutex>(oplock);												}
		/// @brief Increments the reference counter, if applicable.
		constexpr void acquire()	{if (refs < Limit::MAX<usize>) ++refs; printf("& References =  %zu\n", refs);	}
		/// @brief Decrements the reference counter, if applicable.
		constexpr void release()	{if (refs) --refs; printf("& References =  %zu\n", refs);						}
		/// @brief Returns whether there are no more references left.
		constexpr bool dead() const	{return !refs;																	}
	};

	/// @brief Empty constructor.
	AtomicCell()			{}
	/// @brief Empty constructor.
	AtomicCell(nulltype)	{}

	/// @brief Copy constructor (`AtomicCell`).
	/// @param obj Cell to reference.
	AtomicCell(SelfType const& other): wrapper(other.wrapper) {
		if (!other.exists()) return;
		auto const _ = wrapper->lock();
		wrapper->acquire();
	}

	/// @brief Move constructor (`AtomicCell`).
	/// @param obj Cell to reference.
	AtomicCell(SelfType&& other): wrapper(displace(other.wrapper)) {}

	/// @brief Copy assignment operator.
	/// @param obj Cell to reference.
	/// @return Reference to self.
	SelfType& operator=(SelfType const& other) {
		if (wrapper == other.wrapper) return *this;
		unbind();
		if (!other.exists()) return *this;
		wrapper = other.wrapper;
		auto const _ = wrapper->lock();
		wrapper->acquire();
		return *this;
	}

	/// @brief Move assignment operator.
	/// @param obj Cell to reference.
	/// @return Reference to self.
	SelfType& operator=(SelfType&& other) {
		wrapper = displace(other.wrapper);
		return *this;
	}

	/// @brief Destructor.
	~AtomicCell() {
		unbind();
	}

	/// @brief Returns a pointer to the underlying value.
	/// @return Pointer to value.
	/// @throw `NullPointerException` if object does not exist.
	PointerType operator->() const {
		if (!exists()) emptyError();
		auto const _ = wrapper->lock();
		return &wrapper->value;
	}

	/// @brief Returns a reference to the underlying value.
	/// @return Reference to value.
	/// @throw `NullPointerException` if object does not exist.
	ReferenceType operator*() const {
		if (!exists()) emptyError();
		auto const _ = wrapper->lock();
		return wrapper->value;
	}

	/// @brief Creates a cell.
	/// @tparam ...Args Argument types.
	/// @param ...args Arguments to pass to object construtor.
	/// @return Cell to value.
	template <class... TArgs>
	static SelfType create(TArgs... args) {
		SelfType cell;
		cell.wrapper = new Wrapper{.value = move(DataType(args...)), .refs = 1};
		return cell;
	}

	/// @brief Atomically modifies the underlying value.
	/// @tparam TFunction Operation type.
	/// @tparam op Operation to perform.
	/// @return Reference to self.
	template<Type::Functional<DataType(DataType const&)> TFunction>
	SelfType& modify(TFunction const& op) {
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
	SelfType& perform(TFunction const& op) {
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
	SelfType& perform(TFunction const& op) {
		if (!exists())
			return (op(), *this);
		auto const _ = wrapper->lock();
		op();
		return (*this);
	}

	/// @brief Creates a synchronization barrier bound to the cell's value.
	/// @return Sync barrier.
	/// @throw `NullPointerException` if object does not exist.
	[[nodiscard]]
	ScopeLock<Mutex> sync() 		{return exists() ? wrapper->lock() : lock();}
	/// @brief Creates a synchronization barrier bound to the cell's value.
	/// @return Sync barrier.
	/// @throw `NullPointerException` if object does not exist.
	[[nodiscard]]
	ScopeLock<Mutex> sync() const	{if (exists()) return wrapper->lock(); emptyError();}

	/// @brief Equality comparison operator (`Cell`).
	/// @param obj `Cell` to compare to.
	/// @return Whether they're equal.
	bool operator==(SelfType const& other) const		{return wrapper == other.wrapper;	}
	/// @brief Threeway comparison operator (`Cell`).
	/// @param obj `Cell` to compare to.
	/// @return Order between objects.
	OrderType operator<=>(SelfType const& other) const	{return wrapper <=> other.wrapper;	}

	/// @brief Returns whether the object exists.
	/// @return Whether object exists.
	bool exists() const			{return (wrapper && !wrapper->dead());		}
	/// @brief Returns whether the object exists.
	/// @return Whether object exists.
	operator bool() const		{return exists();							}

	/// @brief Returns whether this cell is the sole owner of the bound object.
	/// @return Whether this cell is the sole owner of the bound object.
	bool unique() const			{return (wrapper && wrapper->refs == 1);	}

	/// @brief Creates a scope-bound lock bound to the cells own mutex.
	/// @return Scope lock.
	[[nodiscard]]
	ScopeLock<Mutex>	lock()	{return ScopeLock<Mutex>(mtx);	}
	/// @brief Returns the cell's own mutex.
	/// @return Mutex.
	Mutex&				mutex()	{return mtx;					}

	/// @brief `swap` algorithm.
	friend void swap(SelfType& a, SelfType& b) noexcept {
		swap(a.wrapper, b.wrapper);
	}

private:
	void unbind() {
		printf("<cell>\n");
		if (!exists()) return;
		wrapper->oplock.lock();
		wrapper->release();
		if (wrapper->dead()) {
			wrapper->oplock.unlock();
			delete displace(wrapper);
		} else wrapper->oplock.unlock();
		printf("</cell>\n");
	}

	[[noreturn]] static void emptyError() {
		throw NullPointerException("Atomic cell is empty!");
	}

	// Underlying wrapper to data.
	ptr<Wrapper>	wrapper = nullptr;
	// Cell-local mutex.
	Mutex			mtx;
};

CTL_NAMESPACE_END

#endif
