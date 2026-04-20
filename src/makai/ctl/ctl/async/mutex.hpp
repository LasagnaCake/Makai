#ifndef CTL_ASYNC_MUTEX_H
#define CTL_ASYNC_MUTEX_H

#include "../namespace.hpp"
#include "../templates.hpp"

#ifdef CTL_ON_WINDOWS
#include <windows.h>
#include <synchapi.h>
#else
#endif

CTL_NAMESPACE_BEGIN

/// @brief Mutex (Mutual Exclusion) synchronization barrier.
struct Mutex: SelfIdentified<Mutex> {
	using SelfIdentified	= SelfIdentified<Mutex>;

	using
		typename SelfIdentified::SelfType
	;

	/// @brief Empty constuctor.
	Mutex() {
		#ifdef CTL_ON_WINDOWS
		mutex = CreateMutexA(NULL, FALSE, NULL);
		#else
		#endif
	}

	/// @brief Captures the mutex. If mutex is captured by another thread, waits for it to be released.
	/// @return Reference to self.
	SelfType& capture()	{
		#ifdef CTL_ON_WINDOWS
		SignalObjectAndWait(mutex, mutex, INFINITE, FALSE);
		#else
		#endif
		return *this;
	}

	/// @brief Tries to capture the mutex. Fails if mutex is captured by another thread.
	/// @return Whether mutex caputure was successful.
	bool tryCapture() {
		#ifdef CTL_ON_WINDOWS
		if (SignalObjectAndWait(mutex, mutex, 0, FALSE) == WAIT_TIMEOUT)
			return false;
		#else
		#endif
		return true;
	}

	/// @brief Releases the captured mutex, if mutex is captured by the current hread.
	/// @return Reference to self.
	SelfType& release()	{
		#ifdef CTL_ON_WINDOWS
		ReleaseMutex(mutex);
		#else
		#endif
		return *this;
	}

	/// @brief Waits for the mutex to be released.
	/// @return Reference to self.
	SelfType& wait() {
		#ifdef CTL_ON_WINDOWS
		WaitForSingleObject(mutex, INFINITE);
		#else
		#endif
		return *this;
	}

private:
	#ifdef CTL_ON_WINDOWS
	HANDLE mutex;
	#else
	#endif
};

CTL_NAMESPACE_END

#endif // CTL_ASYNC_MUTEX_H
