#ifndef CTL_ASYNC_TASK_H
#define CTL_ASYNC_TASK_H

#include "../namespace.hpp"
#include "../container/functor.hpp"
#include "promise.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Asynchronous function.
/// @tparam F Function type.
template<class F>
class Task;

/// @brief Asynchronous function wrapper.
/// @tparam R Return type.
/// @tparam ...Args Argument types.
template<typename R, typename... Args>
class Task<R(Args...)>:
	Functional<Promise<R>(Thread::ExecutionToken, Args...)>,
	Returnable<Promise<R>>,
	Argumented<Args...>,
	SelfIdentified<Task<R(Args...)>> {
public:
	using Functional		= ::CTL::Functional<Promise<R>(Thread::ExecutionToken&, Args...)>;
	using Returnable		= ::CTL::Returnable<Promise<R>>;
	using SelfIdentified	= ::CTL::SelfIdentified<Task<R(Args...)>>;

	using
		typename Functional::FunctionType
	;

	using
		typename Returnable::ReturnType
	;
	
	using
		typename SelfIdentified::SelfType
	;

	/// @brief Thread execution token type.
	using ExecutionTokenType = Thread::ExecutionToken;

	/// @brief Function wrapper type.
	using FunctorType	= Functor<FunctionType>;
	/// @brief Promise type.
	using PromiseType	= Promise<ReturnType>;
	/// @brief Nullable result type.
	using NullableType	= Nullable<ReturnType>;

	/// @brief Empty constructor.
	constexpr Task() {}

	/// @brief Copy constructor.
	/// @param other `Task` to copy from.
	constexpr Task(SelfType const& other):
	executor(other.executor),
	result(other.result),
	target(other.target) {
	}

	/// @brief Move constructor.
	/// @param other `Task` to move.
	constexpr Task(SelfType&& other):
	executor(CTL::move(other.executor)),
	result(CTL::move(other.result)),
	target(CTL::move(other.target)) {
	}

	/// @brief Binds a function to run asynchronously.
	/// @param f Function to bind.
	/// @note Only exists if function takes any number of arguments.
	constexpr Task(FunctorType const& f) requires (sizeof...(Args) > 0): target(f)	{				}
	/// @brief Binds a function and runs it asynchronously.
	/// @param f Function to bind.
	/// @param ...args Values to pass to function.
	constexpr Task(FunctorType const& f, Args... args): SelfType(f)					{run(args...);	}
	/// @brief Binds and starts a task, if not already running.
	/// @param other Task to bind.
	/// @param ...args Values to pass to task.
	constexpr Task(SelfType const& other, Args... args): SelfType(other)			{run(args...);	}
	/// @brief Binds and starts a task, if not already running.
	/// @param other Task to bind.
	/// @param ...args Values to pass to task.
	constexpr Task(SelfType&& other, Args... args): SelfType(other)					{run(args...);	}

	/// @brief Binds and runs a function asynchronously, if not already running.
	/// @param f Function to bind.
	/// @param ...args Values to pass to function.
	/// @return Promise to result.
	PromiseType invoke(FunctorType const& f, Args... args) {
		if (running())
			return getPromise();
		executor = f;
		return run(args...);
	}

	/// @brief Runs the bound function asynchronously, if not already running. If no function is bound, does nothing.
	/// @param ...args Values to pass to function.
	/// @return Promise to result.
	PromiseType run(Args&&... args) requires Type::NonVoid<ReturnType> {
		if (!running()) {
			rebindResult();
			executor
			.unbind()
			.bind(
				new Thread(
					[this, ...args = args] (ExecutionTokenType& exect) {
						result = nullptr;
						result = target.value()(exect, args...);
					}
				)
			);
		}
		return getPromise();
	}

	/// @brief Runs the bound function asynchronously, if not already running. If no function is bound, does nothing.
	/// @param ...args Values to pass to function.
	/// @return Promise to result.
	PromiseType run(Args... args) requires Type::Void<ReturnType> {
		if (!running()) {
			rebindResult();
			executor
			.unbind()
			.bind(
				new Thread(
					target.value(),
					args...
				)
			);
		}
		return getPromise();
	}

	/// @brief Waits for the function to finish.
	/// @return Result of the function, or null.
	NullableType await() requires Type::NonVoid<ReturnType> {
		if (running())
			executor->join();
		return result;
	}

	/// @brief Waits for the function to finish.
	/// @return Reference to self.
	SelfType& await() requires Type::Void<ReturnType> {
		if (running())
			executor->join();
		return *this;
	}

	/// @brief Returns the result of the function. If function is not yet finished, returns null.
	/// @return Result of the function, or null if not finished.
	NullableType value() requires Type::NonVoid<ReturnType> {
		if (running())
			return nullptr;
		return result->value();
	}

	/// @brief Returns a promise to the result.
	/// @return Promise to the result.
	PromiseType getPromise() requires Type::NonVoid<ReturnType>	{return PromiseType(result.asWeak(), executor);	}
	/// @brief Returns a promise to the result.
	/// @return Promise to the result.
	PromiseType getPromise() requires Type::Void<ReturnType>	{return PromiseType(executor);					}

	/// @brief Returns whether the function is still running.
	/// @return Whether the function is still running.
	bool running() const {
		return executor() && executor->running();
	}

	/// @brief Stops the running function.
	/// @param kindly Whether to kindly request for the function to stop, or just terminate it forcibly.
	/// @return Reference to self.
	SelfType& stop(bool const kindly = false) {
		if (kindly)
			executor->source().requestStop();
		else executor.destroy();
		return *this;
	}


private:
	void rebindResult() {
		result.unbind().bind(new Atomic<NullableType>());
	}

	/// @brief Result of the function.
	Instance<Atomic<NullableType>>	result = new Atomic<NullableType>();
	/// @brief Underlying function to run.
	Atomic<FunctorType>				target;
	/// @brief Thread to run the function in.
	Instance<Thread>				executor;
};

CTL_NAMESPACE_END

#endif // CTL_ASYNC_TASK_H
