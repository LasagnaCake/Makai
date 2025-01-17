#ifndef CTL_ASYNC_THREAD_H
#define CTL_ASYNC_THREAD_H

#include <thread>
#include "../namespace.hpp"
#include "../os/time.hpp"
#include "../templates.hpp"

CTL_NAMESPACE_BEGIN

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

		friend class ::CTL::Thread;
	};

	/// @brief Starts a thread of execution.
	/// @tparam F Process type.
	/// @tparam ...Args Argument types.
	/// @param f Process (function) to execute.
	/// @param ...args Values to pass to function.
	template<class F, class... Args>
	explicit Thread(F&& f, Args... args):
		BaseType(move(f), token(), args...) {
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
		std::this_thread::sleep_for<T>(time);
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
