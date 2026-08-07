#ifndef CTL_ASYNC_THREAD_H
#define CTL_ASYNC_THREAD_H

#include <thread>
#include "../namespace.hpp"
#include "../os/time.hpp"
#include "../templates.hpp"
#include "../order.hpp"
#include "../container/pointer/atomiccell.hpp"
#include "../container/tuple.hpp"
#include "../container/function.hpp"
#include "../container/nullable.hpp"

#ifdef CTL_ON_WINDOWS
#include <processthreadsapi.h>
#else
#include <pthread.h>
#include <time.h>
#endif

CTL_NAMESPACE_BEGIN

/// @brief Execution thread.
struct Thread: SelfIdentified<Thread> {
	using SelfIdentified	= ::CTL::SelfIdentified<Thread>;

private:
	struct IExecutor {
		virtual ~IExecutor() {}

		// Output value.
		pointer outp		= nullptr;
		// Output Whether the object is running.
		bool	running		= false;
		// Output Whether the object has a result.
		bool	hasResult	= false;
	};

	template <class T>
	struct Executor;

	template <class TReturn, class... TArgs>
	struct Executor<TReturn(TArgs...)>: IExecutor {
		struct None {};
		using Caller	= Function<TReturn(TArgs...)>;
		using Arguments	= Meta::If<sizeof...(TArgs), Tuple<TArgs...>, None>;
		using Result	= Meta::If<Type::NonVoid<TReturn>, Decay::AsArgument<TReturn>, None>;

		#ifdef CTL_ON_WINDOWS
		using ThreadResponse = DWORD;
		#else
		using ThreadResponse = pointer;
		#endif

		static ThreadResponse finished() {return {};}

		static ThreadResponse invoke(pointer ep) {
			if (!ep) return finished();
			auto& exec = *(ref<Executor>)ep;
			exec.outp = &exec.result;
			exec.running	= true;
			exec.hasResult	= false;
			if constexpr (Type::Equal<Result, None>) {
				invokeFromTuple(
					exec.func,
					exec.args
				);
				exec.running	= false;
				exec.hasResult	= true;
				return finished();
			} else if constexpr (Type::Equal<Result, AsReference<AsNonReference<Result>>>)
				exec.result = &invokeFromTuple(
					exec.func,
					exec.args
				);
			else exec.result = invokeFromTuple(
				exec.func,
				exec.args
			);
			exec.running	= false;
			exec.hasResult	= true;
			return finished();
		}

		Executor(Caller&& func, Arguments&& args): func(move(func)), args(move(args)) {}

		Caller		func;
		Arguments	args;
		Result		result;
	};

	struct Impl {
		Impl& stop() {
			if (running()) return *this;
			#ifdef CTL_ON_WINDOWS
			TerminateThread(thread, -1);
			#else
			pthread_cancel(thread);
			#endif
			return *this;
		}

		Impl& join() {
			if (!running()) return *this;
			#ifdef CTL_ON_WINDOWS
			WaitForSingleObject(thread, INFINITE);
			#else
			pthread_join(thread, NULL);
			#endif
			return *this;
		}

		Impl& detach() {
			if (!running()) return *this;
			#ifdef CTL_ON_WINDOWS
			CloseHandle(thread);
			#else
			pthread_detach(thread);
			#endif
			return *this;
		}

		~Impl() {
			detach();
		}

		bool running() const {
			return exec && exec->running;
		}

		bool hasResult() const {
			return exec && exec->hasResult;
		}

		#ifdef CTL_ON_WINDOWS
		HANDLE			thread;
		#else
		pthread_t		thread;
		#endif
		ref<IExecutor>	exec	= nullptr;
		usize			id		= count++;
	private:
		inline static usize count = 0;
	};
public:

	/// @brief Thread promise object.
	/// @tparam T Result type.
	template<class T>
	struct Promise;

	/// @brief Thread promise object.
	/// @tparam T Result type.
	template<Type::Void T>
	struct Promise<T> {
		/// @brief Waits for the thread to finish.
		Promise& await() {
			if (thread) thread->join();
			return *this;
		}

		/// @brief Waits for the thread to finish.
		Promise const& await() const {
			if (thread) thread->join();
			return *this;
		}

		/// @brief Returns whether the function is done processing.
		/// @return Whether the function is done processing.
		bool ready() {
			return !thread or thread->hasResult();
		}

		/// @brief Returns whether awaiting is necessary.
		/// @return Whether to await.
		bool await_ready()			{return ready();	}
		/// @brief Returns the suspension state.
		void await_suspend()		{					}
		/// @brief Returns the result of the await.
		/// @return Await result.
		void await_resume()			{					}

	private:
		friend struct Thread;
		AtomicCell<Impl> thread;
	};

	/// @brief Thread promise object.
	/// @tparam T Result type.
	template <Type::NonVoid T>
	struct Promise<T> {
		/// @brief Waits for the thread to finish.
		Promise& await() {
			if (thread) thread->join();
			return *this;
		}

		/// @brief Waits for the thread to finish.
		Promise const& await() const {
			if (thread) thread->join();
			return *this;
		}

		/// @brief Returns whether the function is done processing.
		/// @return Whether the function is done processing.
		bool ready() {
			return !thread or thread->hasResult();
		}

		/// @brief Returns the result of the promise, if thread exists and is done processing.
		/// @return Promise result, or null.
		Nullable<T> result() const {
			if (!thread) return null;
			if (!thread->hasResult()) return null;
			return value();
		}

		/// @brief Returns whether awaiting is necessary.
		/// @return Whether to await.
		bool await_ready()			{return ready();	}
		/// @brief Returns the suspension state.
		void await_suspend()		{					}
		/// @brief Returns the result of the await.
		/// @return Await result.
		Nullable<T> await_resume()	{return result();	}

	private:
		T value() const {
			await();
			if constexpr (Type::Equal<T, AsReference<AsNonReference<T>>>)
				return **(T**)thread->exec->outp;
			else return *(T*)thread->exec->outp;
		}

		friend struct Thread;
		AtomicCell<Impl> thread;
	};

	/// @brief Empty constructor.
	Thread() noexcept {}

	/// @brief Creates a thread with the given function.
	/// @param call Function to invoke.
	/// @param ...args Arguments to pass to function.
	/// @return Promise to result.
	template <class TReturn, class... TArgs>
	Thread(Executor<TReturn(TArgs...)>::Caller const& call, TArgs... args) {
		invoke(call, args...);
	}

	/// @brief Destructor.
	~Thread() {}

	/// @brief Invokes a thread with the given function.
	/// @param call Function to invoke.
	/// @param ...args Arguments to pass to function.
	/// @return Promise to result.
	template <class TReturn, class... TArgs>
	Promise<TReturn> invoke(Executor<TReturn(TArgs...)>::Caller const& call, TArgs... args) {
		if (running())
			detach();
		return tryInvoke(call, args...).value();
	}

	/// @brief Tries to invoke a thread with the given function. Fails if a thread is already running.
	/// @param call Function to invoke.
	/// @param ...args Arguments to pass to function.
	/// @return Promise to result, or null if this object already has a thread.
	template <class TReturn, class... TArgs>
	Nullable<Promise<TReturn>> tryInvoke(Executor<TReturn(TArgs...)>::Caller const& call, TArgs... args) {
		if (running()) return nullptr;
		thread = thread.create();
		thread->exec = new Executor<TReturn(TArgs...)>(
			call,
			{args...}
		);
		#ifdef CTL_ON_WINDOWS
		thread->thread = CreateThread(
			NULL,
			-1,
			Executor<TReturn(TArgs...)>::invoke,
			(pointer)&thread->exec,
			NULL
		);
		#else
		pthread_create(
			&thread->thread,
			NULL,
			Executor<TReturn(TArgs...)>::invoke,
			(pointer)&thread->exec
		);
		#endif
		return Promise<TReturn>(thread);
	}

	/// @brief Returns whether the thread is still processing.
	bool running() const {
		return thread && thread->running();
	}

	/// @brief Returns whether the thread has yielded a result.
	bool hasResult() const {
		return thread && thread->hasResult();
	}

	/// @brief Stops the thread's execution.
	Thread& stop() {
		if (running()) return *this;
		thread->stop();
		return *this;
	}

	/// @brief Attaches the thread to the called thread, waiting for its execution to finish.
	Thread& join() {
		if (!running()) return *this;
		thread->join();
		return *this;
	}

	/// @brief Detaches the thread from the called thread.
	Thread& detach() {
		if (!running()) return *this;
		thread = nullptr;
		return *this;
	}

	static void yield() noexcept {
		wait(0);
	}

	static void wait(usize const millis) noexcept {
		#ifdef CTL_ON_WINDOWS
		Sleep(millis);
		#else
		timespec spec;
		spec.tv_sec = millis / 1000;
    	spec.tv_nsec = (millis % 1000) * 1000000;
     	nanosleep(&ts, &ts);
		#endif
	}

	/// @brief Move constructor (defaulted).
	Thread(Thread&& other)		= default;
	/// @brief Copy constructor (defaulted).
	Thread(Thread const& other)	= default;

	/// @brief Move assignment operator (defaulted).
	Thread& operator=(Thread&& other)		= default;
	/// @brief Copy assignment operator (defaulted).
	Thread& operator=(Thread const& other)	= default;

	/// @brief Returns the thread's ID.
	/// @return Thread ID.
	Nullable<usize> id() const {if (thread) return thread->id; return null;};

	/// @brief Equality comparison operator.
	/// @param other `Thread` to compare with.
	/// @return Whether they're equal.
	bool operator==(Thread const& other) const	{return id() == other.id();		}
	/// @brief Threeway comparison operator.
	/// @param other `Thread` to compare with.
	/// @return Order between `Thrad`s.
	auto operator<=>(Thread const& other) const	{return id() <=> other.id();	}

private:
	AtomicCell<Impl> thread;
};

/// @brief Thread promise object. Analog for `Thread::Promise<T>`.
/// @tparam T Result type.
template <class T>
using Promise = Thread::Promise<T>;

/// @brief Asynchronous facilities.
namespace Async {}

/// @brief Base classes.
namespace Async::Base {
	/// @brief Thread yieldable.
	struct Yieldable {
	protected:
		/// @brief Yields the thread it is called in.
		static void asyncYield() noexcept {
			Thread::yield();
		}
	};
}

CTL_NAMESPACE_END

#endif // CTL_ASYNC_THREAD_H
