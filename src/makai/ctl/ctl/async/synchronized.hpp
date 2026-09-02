#ifndef CTL_ASYNC_SYNCHRONIZED_H
#define CTL_ASYNC_SYNCHRONIZED_H

#include "mutex.hpp"
#include "atomic.hpp"
#include "lock.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Read-Write Lock.
template <Type::Safe T>
struct Synchronized: Typed<T>, SelfIdentified<Synchronized<T>> {
	using Typed				= ::CTL::Typed<T>;
	using SelfIdentified	= ::CTL::SelfIdentified<Synchronized<T>>;

	using
		typename Typed::DataType,
		typename Typed::ConstReferenceType,
		typename Typed::TemporaryType
	;

	using
		typename SelfIdentified::SelfType
	;

	DataType get() const {
		if (!canRead.value()) {
			mutex.lock();
			DataType v = value;
			mutex.unlock();
			return v;
		}
		return value;
	}

	Nullable<DataType> tryGet() const {
		if (!canRead.value())
			return null;
		return value;
	}

	SelfType& set(DataType const& newValue) {
		mutex.lock();
		canRead = false;
		value = newValue;
		canRead = true;
		mutex.unlock();
		return *this;
	}

	operator DataType() const			{return get();		}
	operator Nullable<DataType>() const	{return tryGet();	}

	SelfType& operator=(DataType const& value)	{return set(value);}

	Synchronized(DataType const& value): value(value)	{}
	Synchronized(DataType&& value): value(move(value))	{}

	Synchronized() {}

	Synchronized(SelfType const&)	= delete;
	Synchronized(SelfType&&)		= delete;

	SelfType& operator=(SelfType const&)	= delete;
	SelfType& operator=(SelfType&&)			= delete;

private:
	T				value;
	Mutex mutable	mutex;
	Atomic<bool>	canRead = true;
};

CTL_NAMESPACE_END

#endif // CTL_ASYNC_SYNCHRONIZED_H
