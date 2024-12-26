#ifndef CTL_ASYNC_PROMISE_H
#define CTL_ASYNC_PROMISE_H

#include "../namespace.hpp"
#include "../typetraits/traits.hpp"
#include "../templates.hpp"
#include "../container/pointer.hpp"
#include "atomic.hpp"

CTL_NAMESPACE_BEGIN

template<class F>
class Task;

/// @brief Asynchronous function result.
/// @tparam T Result type.
template<class T>
class Promise;

/// @brief Asynchronous void function result.
template<> class Promise<void>:
	SelfIdentified<Promise<void>> {
	/// @brief Value type.
	using DataType = void;

	/// @brief Waits for the function to finish.
	void await() {if (!ready()) thread->join();	}
	/// @brief Returns whether the function is done processing.
	/// @return Whether the function is done processing.
	bool ready() {return !(thread && *thread);	}

	/// @brief Copy constructor.
	/// @param other `Promise` to copy from.
	constexpr Promise(Promise const& other): Promise(other.thread) {}

private:
	/// @brief Binds a promise to a thread.
	/// @param t Thread to bind to.
	constexpr Promise(Handle<Thread> const& t): thread(t) {}

	/// @brief Bound thread.
	Handle<Thread> thread;

	template<typename F> friend class Task;
};

/// @brief Asynchronous function result.
/// @tparam T Result type.
template<Type::Different<void> T>
class Promise<T>:
	public Typed<T>,
	public SelfIdentified<Promise<T>> {
public:
	using Typed				= Typed<T>;
	using SelfIdentified	= SelfIdentified<Promise<T>>;

	using
		typename Typed::DataType
	;

	using
		typename SelfIdentified::SelfType
	;

	/// @brief Nullable value type.
	using NullableType	= Nullable<DataType>;
	/// @brief Handle-to-value type.
	using HandleType	= Handle<NullableType>;

	/// @brief Waits for the function to finish.
	/// @return Result of the function, or null.
	NullableType await() {
		if (!ready())
			thread->join();
		return value();
	}

	/// @brief Returns the result of the function. If function is not yet finished, returns null.
	/// @return Result of the function, or null if not finished.
	NullableType value() {
		if (ready() && data())
			return *data;
		return nullptr;
	}

	/// @brief Returns whether the function is done processing.
	/// @return Whether the function is done processing.
	bool ready() {
		return !(thread && *thread);
	}
	
	/// @brief Copy constructor.
	/// @param other `Promise` to copy from.
	constexpr Promise(SelfType const& other): Promise(other.data, other.thread) {}

private:
	/// @brief Binds a promise to a thread.
	/// @param v Handle to result's location.
	/// @param t Thread to bind to.
	constexpr Promise(HandleType const& v, Handle<Thread> const& t): data(v), thread(t) {}

	/// @brief Bound thread.
	Handle<Thread>	thread;
	/// @brief Location of the result.
	HandleType		data;

	template<typename F> friend class Task;
};

CTL_NAMESPACE_END

#endif // CTL_ASYNC_PROMISE_H
