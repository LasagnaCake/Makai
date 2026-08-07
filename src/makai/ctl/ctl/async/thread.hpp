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
#else
#include <pthread.h>
#endif

CTL_NAMESPACE_BEGIN

namespace New {
	struct Thread: SelfIdentified<Thread> {
		using SelfIdentified	= ::CTL::SelfIdentified<Thread>;

		struct IExecutor {
			virtual ~IExecutor() {}

			pointer outp		= nullptr;
			bool	running		= false;
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

			static pointer invoke(pointer ep) {
				if (!ep) return nullptr;
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
					return nullptr;
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
				return (pointer)&exec.result;
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
				#else
				pthread_cancel(thread);
				#endif
				return *this;
			}

			Impl& join() {
				if (!running()) return *this;
				#ifdef CTL_ON_WINDOWS
				#else
				pthread_join(thread, NULL);
				#endif
				return *this;
			}

			Impl& detach() {
				if (!running()) return *this;
				#ifdef CTL_ON_WINDOWS
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
			#else
			pthread_t		thread;
			#endif
			ref<IExecutor>	exec	= nullptr;
			usize			id		= count++;
		private:
			inline static usize count = 0;
		};

		template<class T>
		struct Promise;

		template<>
		struct Promise<void> {
			Promise& await() {
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

		template <Type::NonVoid T>
		struct Promise<T> {
			Promise& await() {
				if (thread) thread->join();
				return *this;
			}

			Promise const& await() const {
				if (thread) thread->join();
				return *this;
			}

			/// @brief Returns whether the function is done processing.
			/// @return Whether the function is done processing.
			bool ready() {
				return !thread or thread->hasResult();
			}

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

		Thread() noexcept {}

		template <class TReturn, class... TArgs>
		Thread(Executor<TReturn(TArgs...)>::Caller const& call, TArgs... args) {
			invoke(call, args...);
		}

		~Thread() {}

		template <class TReturn, class... TArgs>
		Promise<TReturn> invoke(Executor<TReturn(TArgs...)>::Caller const& call, TArgs... args) {
			if (running())
				detach();
			return tryInvoke(call, args...).value();
		}

		template <class TReturn, class... TArgs>
		Nullable<Promise<TReturn>> tryInvoke(Executor<TReturn(TArgs...)>::Caller const& call, TArgs... args) {
			if (running()) return nullptr;
			thread = thread.create();
			thread->exec = new Executor<TReturn(TArgs...)>(
				call,
				{args...}
			);
			#ifdef CTL_ON_WINDOWS
			#else
			pthread_create(&thread->thread, NULL, Executor<TReturn(TArgs...)>::invoke, (pointer)&thread->exec);
			#endif
			return Promise<TReturn>(thread);
		}

		bool running() const {
			return thread && thread->running();
		}

		bool hasResult() const {
			return thread && thread->hasResult();
		}

		Thread& stop() {
			if (running()) return *this;
			thread->stop();
			return *this;
		}

		Thread& join() {
			if (!running()) return *this;
			thread->join();
			return *this;
		}

		Thread& detach() {
			if (!running()) return *this;
			thread = nullptr;
			return *this;
		}

		static void yield() noexcept {
		}

		Thread(Thread&& other)		= default;
		Thread(Thread const& other)	= default;

		Thread& operator=(Thread&& other)		= default;
		Thread& operator=(Thread const& other)	= default;

		Nullable<usize> id() const {if (thread) return thread->id; return null;};

		bool operator==(Thread const& other) const	{return id() == other.id();		}
		auto operator<=>(Thread const& other) const	{return id() <=> other.id();	}

	private:
		AtomicCell<Impl> thread;
	};
}

/// @brief Execution thread.
struct Thread:
	public SelfIdentified<Thread>,
	public Derived<std::thread>,
	private std::thread {
	using SelfIdentified	= ::CTL::SelfIdentified<Thread>;
	using Derived			= ::CTL::Derived<std::thread>;

	using typename Derived::BaseType;

	using typename SelfIdentified::SelfType;

	using BaseType::swap;

	/// @brief Default constructor.
	Thread() noexcept:					BaseType(), exect(*this)												{}
	/// @brief Move constructor.
	/// @param other `Thread` to move.
	Thread(SelfType&& other) noexcept:	BaseType((BaseType&&)CTL::move(other)), exect(CTL::move(other.exect))	{}
	/// @brief Copy constructor (deleted).
	Thread(SelfType const& other)		= delete;

	/// @brief Destructor.
	~Thread() {exect.requestStop();}

	/// @brief Process execution controller.
	struct ExecutionSource {
		/// @brief Binds the execution source to a thread.
		/// @param source Thread to bind to.
		constexpr ExecutionSource(Thread& source):				stop(false), source(source)				{}
		/// @brief Move constructor.
		/// @param source `ExecutionSource` to move.
		constexpr ExecutionSource(ExecutionSource&& other):		stop(other.stop), source(other.source)	{}
		/// @brief Copy constructor (deleted).
		constexpr ExecutionSource(ExecutionSource const& other)	= delete;

		/// @brief Returns whether the thread's process should stop running.
		/// @return Whether the process should stop.
		constexpr bool shouldStop() const	{return stop;			}
		/// @brief Returns whether the thread's process should stop running.
		/// @return Whether the process should stop.
		constexpr operator bool() const		{return shouldStop();	}

		/// @brief Requests the process to stop.
		/// @return Reference to self.
		constexpr ExecutionSource& requestStop() {
			stop = true;
			return *this;
		}

		/// @brief Returns the thread that the execution source is bound to.
		/// @return Thread that the execution source is bound to.
		constexpr Thread& thread() {return source;}

	private:
		/// @brief Whether the thread's process should stop.
		bool	stop;
		/// @brief Thread the source is bound to.
		Thread&	source;
	};

	/// @brief Execution token.
	struct ExecutionToken {
		/// @brief Binds a token to an execution source.
		/// @param source Source to bind to.
		constexpr ExecutionToken(ref<ExecutionSource>&& source):	source(source)	{}
		/// @brief Binds a token to an execution source.
		/// @param source Source to bind to.
		constexpr ExecutionToken(ExecutionSource& source):			source(&source)	{}
		/// @brief Deleted.
		constexpr ExecutionToken(ref<ExecutionSource> const source)	= delete;
		/// @brief Deleted.
		constexpr ExecutionToken(ExecutionSource&& source)			= delete;

		/// @brief Returns the execution source bound to.
		/// @return Pointer to execution source bound to.
		constexpr ref<ExecutionSource> operator->()	{return source;		}
		/// @brief Returns the execution source bound to.
		/// @return Reference to execution source bound to.
		constexpr ExecutionSource& operator*()		{return *source;	}

	private:
		/// @brief Execution source associated with the token.
		ref<ExecutionSource> const source;
	};

	/// @brief Unique thread identifier.
	struct ID:
		private std::thread::id,
		public ::CTL::Ordered,
		public ::CTL::Derived<std::thread::id> {
	private:
		using Derived = ::CTL::Derived<std::thread::id>;

		using typename Derived::BaseType;

	public:
		/// @brief Returns the ID as its underlying STL counterpart.
		/// @return ID as `std::thread::id`.
		BaseType const& base() const {return (*this);}
		/// @brief Default constructor.
		ID(): BaseType() {}

		/// @brief Copy constructor (defaulted).
		ID(ID const& other)	= default;
		/// @brief Move constructor (defaulted).
		ID(ID&& other)		= default;

		/// @brief Copy assignment operator (defaulted).
		ID& operator=(ID const& other)	= default;
		/// @brief Move assignment operator (defaulted).
		ID& operator=(ID&& other)		= default;

		/// @brief Threeway comparison operator.
		/// @param other `ID` to compare with.
		/// @return Order between `ID`s.
		OrderType operator<=>(ID const& other) const	{return OrderType(base() <=> other.base());	}
		/// @brief Equality comparison operator.
		/// @param other `ID` to compare with.
		/// @return Whether they're equal.
		bool operator==(ID const& other) const			{return base() == other.base();				}

	private:
		/// @brief Move constructor.
		/// @param other `std::thread::id` to move.
		ID(BaseType&& other): BaseType(CTL::move(other))		{}
		/// @brief Copy constructor.
		/// @param other `std::thread::id` to copy from.
		ID(BaseType const& other): BaseType(CTL::move(other))	{}

		friend struct ::CTL::Thread;
	};

	/// @brief Starts a thread of execution.
	/// @tparam F Process type.
	/// @tparam ...Args Argument types.
	/// @param f Process (function) to execute.
	/// @param ...args Values to pass to function.
	template<class F, class... Args>
	explicit Thread(F&& f, Args&&... args):
		BaseType(move(f), token(), forward<Args>(args)...) {
	}

	/// @brief Returns the thread's ID.
	/// @return Thread ID.
	ID id() const noexcept {
		return thread::get_id();
	}

	/// @brief Returns the current thread's ID.
	/// @return The ID of the current thread.
	static ID current() noexcept {
		return std::this_thread::get_id();
	}

	/// @brief Yields the thread it is called in.
	static void yield() noexcept {
		std::this_thread::yield();
	}

	/// @brief Sleeps the thread it is called in, for a specific amount of time.
	/// @tparam T Time unit.
	/// @param time Time to sleep for.
	template<class T = OS::Time::Millis>
	static void wait(usize const time) {
		std::this_thread::sleep_for(T{time});
	}

	/// @brief Attaches the thread to the called thread, waiting for its execution to finish.
	SelfType& join() {
		thread::join();
		return *this;
	}

	/// @brief Attaches the thread from the called thread.
	SelfType& detach() {
		thread::detach();
		return *this;
	}

	/// @brief Gets an execution token for the thread.
	/// @return Execution token.
	ExecutionToken token()		{return exect;}
	/// @brief Gets the thread's execution source.
	/// @return Reference to execution source.
	ExecutionSource& source()	{return exect;}

	/// @brief Returns whether the thread is running.
	/// @return Whether the thread is running.
	bool running() const {
		return thread::joinable();
	}

	/// @brief Returns whether the thread is running.
	/// @return Whether the thread is running.
	operator bool() const {return running();}

private:
	/// @brief Execution source of the thread.
	ExecutionSource exect;
};

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
