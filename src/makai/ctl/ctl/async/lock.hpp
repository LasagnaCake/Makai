#ifndef CTL_ASYNC_LOCK_H
#define CTL_ASYNC_LOCK_H

#include "../namespace.hpp"
#include "../templates.hpp"
#include "../typetraits/traits.hpp"
#include "mutex.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Basic mutex lock.
/// @tparam TMutex Mutex type.
template<Type::Derived<Mutex> TMutex = Mutex>
class BaseLock:
	public Typed<TMutex>,
	public SelfIdentified<BaseLock<TMutex>> {
public:
	using Typed	= ::CTL::Typed<TMutex>;

	using typename Typed::ReferenceType;

	/// @brief Binds a mutex to this lock.
	/// @param mutex Mutex to bind.
	BaseLock(ReferenceType mutex): mutex(mutex) {}

protected:
	/// @brief Mutex associated with this lock.
	ReferenceType mutex;
};

/// @brief Scope-based mutex lock. Capturs when entering the scope, releases when leaving it.
/// @tparam TMutex Mutex type.
/// @note Cannot be directly heap-allocated.
template<Type::Derived<Mutex> TMutex = Mutex>
class ScopeLock:
	public BaseLock<TMutex>,
	public Derived<BaseLock<TMutex>>,
	public SelfIdentified<ScopeLock<TMutex>> {
public:
	using Derived = ::CTL::Derived<BaseLock<TMutex>>;

	using typename Derived::BaseType;

	using typename BaseType::ReferenceType;

	/// @brief Binds and captures a mutex.
	/// @param mutex Mutex to capture.
	ScopeLock(ReferenceType mutex): BaseType(mutex)	{mutex.capture();}
	/// @brief Releases a mutex.
	~ScopeLock()									{mutex.release();}
	
protected:
	using BaseType::mutex;

private:
	void* operator new(usize);
	void operator delete(pointer);
	void* operator new[](usize);
	void operator delete[](pointer);
};

CTL_NAMESPACE_END

#endif // CTL_ASYNC_LOCK_H
