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
		#ifdef CTL_ON_WINDOWS
		HANDLE	mutex = nullptr;
		#else
		pthread_mutex_t	mutex;
		#endif
		bool exists = false;

		friend constexpr void swap(Impl& a, Impl& b) noexcept {
			swap(a.locked, b.locked);
			swap(a.mutex, b.mutex);
			swap(a.exists, b.exists);
		}
	};

	/// @brief Empty constuctor.
	Mutex() {
		#ifdef CTL_ON_WINDOWS
		mutex.mutex = CreateMutexA(NULL, FALSE, NULL);
		#else
		pthread_mutexattr_t pat;
		pthread_mutexattr_init(&pat);
		pthread_mutexattr_settype(&pat, PTHREAD_MUTEX_ERRORCHECK);
		pthread_mutex_init(&mutex.mutex, &pat);
		pthread_mutexattr_destroy(&pat);
		#endif
		mutex.exists = true;
	}

	~Mutex() {unbind();}

	Mutex(Mutex const&) = delete;

	Mutex(Mutex&& other) {
		swap(*this, other);
	}

	Mutex& operator=(Mutex const&) = delete;

	Mutex& operator=(Mutex&& other) {
		swap(*this, other);
		return *this;
	}

	/// @brief Captures the mutex. If mutex is captured by another thread, waits for it to be released.
	/// @return Reference to self.
	SelfType& capture()	{
		if (!mutex.exists) return *this;
		#ifdef CTL_ON_WINDOWS
		SignalObjectAndWait(mutex.mutex, mutex.mutex, INFINITE, FALSE);
		#else
		pthread_mutex_lock(&mutex.mutex);
		#endif
		mutex.locked = true;
		return *this;
	}

	/// @brief Captures the mutex. If mutex is captured by another thread, waits for it to be released.
	/// @return Reference to self.
	SelfType& lock() {return capture();}

	/// @brief Tries to capture the mutex. Fails if mutex is captured by another thread.
	/// @return Whether mutex caputure was successful.
	bool tryCapture() {
		if (!mutex.exists) return false;
		#ifdef CTL_ON_WINDOWS
		if (SignalObjectAndWait(mutex.mutex, mutex.mutex, 0, FALSE) == WAIT_TIMEOUT)
			return false;
		#else
		if (pthread_mutex_trylock(&mutex.mutex) == EBUSY)
			return false;
		#endif
		mutex.locked = true;
		return true;
	}

	/// @brief Captures the mutex. If mutex is captured by another thread, waits for it to be released.
	/// @return Reference to self.
	bool tryLock() {return tryCapture();}

	/// @brief Releases the captured mutex, if mutex is captured by the current hread.
	/// @return Reference to self.
	SelfType& release() {
		if (!mutex.exists) return *this;
		#ifdef CTL_ON_WINDOWS
		if (ReleaseMutex(mutex.mutex))
		#else
		if (pthread_mutex_unlock(&mutex.mutex) != EPERM)
		#endif
		mutex.locked = false;
		return *this;
	}

	/// @brief Releases the captured mutex, if mutex is captured by the current hread.
	/// @return Reference to self.
	SelfType& unlock() {return release();}

	/// @brief Waits for the mutex to be released.
	/// @return Reference to self.
	SelfType& wait() {
		if (!mutex.exists) return *this;
		#ifdef CTL_ON_WINDOWS
		WaitForSingleObject(mutex.mutex, INFINITE);
		#else
		lock().unlock();
		#endif
		return *this;
	}

	bool captured() const {
		return mutex.exists ? mutex.locked : false;
	}

	bool locked() const {
		return captured();
	}

	friend constexpr void swap(Mutex& a, Mutex& b) noexcept {
		swap(a.mutex, b.mutex);
	}

private:
	void unbind() {
		if (!mutex.exists) return;
		#ifdef CTL_ON_WINDOWS
		CloseHandle(mutex.mutex);
		#else
		pthread_mutex_destroy(&mutex.mutex);
		#endif
		mutex.exists = false;
	}

	Impl mutex;
};

CTL_NAMESPACE_END

#endif // CTL_ASYNC_MUTEX_H
