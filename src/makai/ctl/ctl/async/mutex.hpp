#ifndef CTL_ASYNC_MUTEX_H
#define CTL_ASYNC_MUTEX_H

#include "../namespace.hpp"
#include "../templates.hpp"
#include "../algorithm/swap.hpp"

#ifdef CTL_ON_WINDOWS
#include <windows.h>
#include <synchapi.h>
#else
#include <pthread.h>
#include <cerrno>
#endif

CTL_NAMESPACE_BEGIN

/// @brief Mutex (Mutual Exclusion) synchronization barrier.
struct Mutex: SelfIdentified<Mutex> {
	using SelfIdentified	= SelfIdentified<Mutex>;

	using
		typename SelfIdentified::SelfType
	;

	struct Impl {
		bool	locked = false;
		usize	rcount = 1;
		#ifdef CTL_ON_WINDOWS
		HANDLE	mutex = nullptr;
		#else
		pthread_mutex_t	mutex;
		#endif
	};

	/// @brief Empty constuctor.
	Mutex() {
		mutex = new Impl;
		#ifdef CTL_ON_WINDOWS
		mutex->mutex = CreateMutexA(NULL, FALSE, NULL);
		#else
		pthread_mutexattr_t pat;
		pthread_mutexattr_init(&pat);
		pthread_mutexattr_settype(&pat, PTHREAD_MUTEX_ERRORCHECK);
		pthread_mutex_init(&mutex->mutex, &pat);
		pthread_mutexattr_destroy(&pat);
		#endif
	}

	~Mutex() {
		unbind();
	}

	Mutex(Mutex const& other)						{clone(other.mutex);										}
	Mutex& operator=(Mutex const& other)			{clone(other.mutex); return *this;							}
	Mutex(Mutex&& other): mutex(move(other.mutex))	{other.mutex = nullptr;										}
	Mutex& operator=(Mutex&& other)					{mutex = other.mutex; other.mutex = nullptr; return *this;	}

	/// @brief Captures the mutex. If mutex is captured by another thread, waits for it to be released.
	/// @return Reference to self.
	SelfType& capture()	{
		if (!mutex) return *this;
		#ifdef CTL_ON_WINDOWS
		SignalObjectAndWait(mutex->mutex, mutex->mutex, INFINITE, FALSE);
		#else
		pthread_mutex_lock(&mutex->mutex);
		#endif
		mutex->locked = true;
		return *this;
	}

	/// @brief Captures the mutex. If mutex is captured by another thread, waits for it to be released.
	/// @return Reference to self.
	SelfType& lock() {return capture();}

	/// @brief Tries to capture the mutex. Fails if mutex is captured by another thread.
	/// @return Whether mutex caputure was successful.
	bool tryCapture() {
		if (!mutex) return false;
		#ifdef CTL_ON_WINDOWS
		if (SignalObjectAndWait(mutex->mutex, mutex->mutex, 0, FALSE) == WAIT_TIMEOUT)
			return false;
		#else
		if (pthread_mutex_trylock(&mutex->mutex) == EBUSY)
			return false;
		#endif
		mutex->locked = true;
		return true;
	}

	/// @brief Captures the mutex. If mutex is captured by another thread, waits for it to be released.
	/// @return Reference to self.
	bool tryLock() {return tryCapture();}

	/// @brief Releases the captured mutex, if mutex is captured by the current hread.
	/// @return Reference to self.
	SelfType& release() {
		if (!mutex) return *this;
		#ifdef CTL_ON_WINDOWS
		if (ReleaseMutex(mutex->mutex))
			mutex->locked = false;
		#else
		pthread_mutex_unlock(&mutex->mutex);
		#endif
		return *this;
	}

	/// @brief Releases the captured mutex, if mutex is captured by the current hread.
	/// @return Reference to self.
	SelfType& unlock() {return release();}

	/// @brief Waits for the mutex to be released.
	/// @return Reference to self.
	SelfType& wait() {
		if (!mutex) return *this;
		#ifdef CTL_ON_WINDOWS
		WaitForSingleObject(mutex->mutex, INFINITE);
		#else
		lock().unlock();
		#endif
		return *this;
	}

	bool captured() const {
		return mutex ? mutex->locked : false;
	}

	bool locked() const {
		return captured();
	}

private:
	void clone(ref<Impl> const other) {
		unbind();
		mutex = other;
		++mutex->rcount;
	}

	void unbind() {
		if (!mutex) return;
		if (mutex && mutex->rcount)
			--mutex->rcount;
		if (!mutex->rcount) {
			#ifdef CTL_ON_WINDOWS
			CloseHandle(mutex->mutex);
			#else
			pthread_mutex_destroy(&mutex->mutex);
			#endif
			delete mutex;
		}
		mutex = nullptr;
	}

	ref<Impl> mutex = nullptr;
};

CTL_NAMESPACE_END

#endif // CTL_ASYNC_MUTEX_H
