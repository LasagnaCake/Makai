#ifndef CTL_CONTAINER_POINTER_ARC_H
#define CTL_CONTAINER_POINTER_ARC_H

#include "../../namespace.hpp"
#include "../../templates.hpp"
#include "../../typeinfo.hpp"
#include "../../ctypes.hpp"
#include "../../typetraits/traits.hpp"
#include "../../async/lock.hpp"
#include "../map/map.hpp"

CTL_NAMESPACE_BEGIN

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

	struct Wrapper {
		Mutex		oplock;
		DataType	value;
		usize		refs;

		constexpr auto lock()		{return ScopeLock<Mutex>(oplock);	}
		constexpr void acquire()	{++refs;							}
		constexpr void release()	{if (refs) --refs;					}
	};

	constexpr AtomicCell():			wrapper(nullptr) {}
	constexpr AtomicCell(nulltype):	wrapper(nullptr) {}

	constexpr AtomicCell(SelfType const& other): wrapper(other.wrapper) {
		if (!exists()) return;
		auto const _ = wrapper->lock();
		wrapper->acquire();
	}

	constexpr AtomicCell(SelfType&& other): wrapper(move(other.wrapper)) {
		other.wrapper = nullptr;
	}

	constexpr SelfType& operator=(SelfType const& other) {
		if (wrapper == other.wrapper) return *this;
		unbind();
		wrapper = other.wrapper;
		if (!other.exists()) return *this;
		auto const _ = wrapper->lock();
		wrapper->acquire();
		return *this;
	}

	constexpr SelfType& operator=(SelfType&& other) {
		if (wrapper == other.wrapper) {
			other.wrapper = nullptr;
			return *this;
		}
		unbind();
		wrapper = move(other.wrapper);
		other.wrapper = nullptr;
		return *this;
	}

	constexpr ~AtomicCell() {
		unbind();
	}

	constexpr PointerType operator->() const {
		if (!exists()) emptyError();
		auto const _ = wrapper->lock();
		return &wrapper->value;
	}

	constexpr ReferenceType operator*() const {
		if (!exists()) emptyError();
		auto const _ = wrapper->lock();
		return wrapper->value;
	}

	template <class... TArgs>
	constexpr static AtomicCell create(TArgs... args) {
		AtomicCell cell;
		cell.wrapper = new Wrapper{.value = DataType(args...), .refs = 1};
		return cell;
	}

	constexpr bool operator==(AtomicCell const& other) const		{return wrapper == other.wrapper;	}
	constexpr OrderType operator<=>(AtomicCell const& other) const	{return wrapper <=> other.wrapper;	}

	constexpr bool exists()		const {return (wrapper && wrapper->refs);		}
	constexpr operator bool()	const {return exists();							}

	constexpr bool unique()		const {return (wrapper && wrapper->refs == 1);	}

	constexpr ScopeLock<Mutex>	lock()	{return ScopeLock<Mutex>(mtx);	}
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
	ptr<Wrapper>	wrapper;
	Mutex			mtx;
};

CTL_NAMESPACE_END

#endif
