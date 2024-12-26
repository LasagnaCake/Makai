#ifndef CTL_ASYNC_MUTEX_H
#define CTL_ASYNC_MUTEX_H

#include "../namespace.hpp"
#include "../templates.hpp"
#include "../container/nullable.hpp"
#include "thread.hpp"
#include "atomic.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Mutex synchronization barrier.
class Mutex:
	Async::Base::Yieldable,
	SelfIdentified<Mutex>,
	Derived<Async::Base::Yieldable>,
	Ordered {
public:
	using SelfIdentified	= SelfIdentified<Mutex>;
	using Derived			= Derived<Async::Base::Yieldable>;

	using
		typename SelfIdentified::SelfType
	;

	using typename Derived::BaseType;

	/// @brief Empty constuctor.
	Mutex() {}

	/// @brief Captures the mutex. If mutex is captured by another thread, waits for it to be released.
	/// @return Reference to self.
	SelfType& capture()	{
		Thread::ID const thisThread = Thread::current();
		if (!isCurrentOwner())
			wait();
		current = thisThread;
		return *this;
	}

	/// @brief Tries to capture the mutex. Fails if mutex is captured by another thread.
	/// @return Whether mutex caputure was successful.
	bool tryCapture() {
		Thread::ID const thisThread = Thread::current();
		if (!isCurrentOwner()) return false;
		current = thisThread;
		return true;
	}

	/// @brief Releases the captured mutex, if mutex is captured by the current hread.
	/// @return Reference to self.
	SelfType& release()	{
		if (isCurrentOwner())
			current = nullptr;
		return *this;
	}
	
	/// @brief Waits for the mutex to be released.
	/// @return Reference to self.
	SelfType& wait() {
		if (!isCurrentOwner())
			while(captured())
				asyncYield();
		return *this;
	}

	/// @brief Returns whether the mutex is currently captured.
	/// @return Whether the mutex is currently captured.
	bool captured() const							{return current.value().exists();	}
	/// @brief Returns whether the mutex is owned by the given thread.
	/// @param id Thread to check.
	/// @return Whether the mutex is owned by the thread.
	bool ownedBy(Thread::ID const& id) const		{return equals(id);					}
	/// @brief Returns whether current thread owns the mutex.
	/// @return Whether the current thread owns the mutex.
	bool isCurrentOwner() const						{return equals(Thread::current());	}

	/// @brief Returns whether the mutex is owned by the given thread.
	/// @param id Thread to check.
	/// @return Whether the mutex is owned by the thread.
	bool operator==(Thread::ID const& id) const			{return ownedBy(id);	}
	// How do I explain this?
	OrderType operator<=>(Thread::ID const& id) const	{return compare(id);	}

	/// @brief Returns whether the mutex is currently captured.
	/// @return Whether the mutex is currently captured.
	operator bool() {return captured();}

	/// @brief Returns the mutex's current owner.
	/// @return Mutex's current owner.
	Nullable<Thread::ID> currentOwner() const	{return current.value();	}
	/// @brief Returns the mutex's current owner.
	/// @return Mutex's current owner.
	Nullable<Thread::ID> currentOwner()			{return current.value();	}

private:
	bool equals(Thread::ID const& id) const {
		if (captured())
			return current.value().value() == id;
		return false;
	}

	OrderType compare(Thread::ID const& id)	const {
		if (captured())
			return current.value().value() <=> id;
		return Order::LESS;
	}

	/// @brief The mutex's current owner.
	Atomic<Nullable<Thread::ID>> current;
};

CTL_NAMESPACE_END

#endif // CTL_ASYNC_MUTEX_H
