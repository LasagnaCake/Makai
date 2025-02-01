#ifndef CTL_ASYNC_CO_PROMISE_H
#define CTL_ASYNC_CO_PROMISE_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"
#include "context.hpp"
#include "suspend.hpp"
#include "../../cpperror.hpp"

// Based off of: https://www.scs.stanford.edu/~dm/blog/c++-coroutines.html#compiling-code-using-coroutines

CTL_NAMESPACE_BEGIN

/// @brief Cooperative routine facilities.
namespace Co {
	template<class TData = void, bool S = false>
	struct Promise;

	/// @brief Coroutine return type.
	/// @tparam S Whether the coroutine should start suspended.
	template<bool S>
	struct Promise<void, S> {
		struct promise_type;
		/// @brief Context type.
		using ContextType = Context<promise_type>;
		
		/// @brief Whether the coroutine should start suspended.
		constexpr static bool START_SUSPENDED = S;

		/// @brief STL promise implementation type.
		struct promise_type {
			/// @brief Returns the promise associated with this type.
			/// @return Associated promise.
			Promise get_return_object() {
				return ContextType::from_promise(*this);
			}

			/// @brief Initial suspend.
			/// @return Suspend state.
			Suspend<S> initial_suspend()			{return {};	}
			/// @brief Final suspend.
			/// @return Suspend state.
			AlwaysSuspend final_suspend() noexcept	{return {};	}
			/// @brief Yields void.
			/// @return Suspend state.
			AlwaysSuspend yield_value(void)			{return {};	}
			/// @brief Returns void.
			void return_void()						{			}

			/// @brief Specialized suspender.
			struct SuspendType {
				bool await_ready()		{return promise.finished();	}
				void await_suspend()	{							}
				void await_resume()		{							}
				Promise promise;
			};

			/// @brief `co_await` support for promises.
			SuspendType await_transform(AlwaysSuspend) {
				return SuspendType{get_return_object()};
			}

			/// @brief `co_await` support for specialized suspensions.
			template<Type::Different<AlwaysSuspend> T>
			T await_transform(T const& sus) {
				return sus;
			}
			
			/// @brief Unhandle exception processor.
			void unhandled_exception() {
				throw Exception(*Exception::current());
			}
		};

		/// @brief Awaits the coroutine to finish executing.
		void await() const {
			while (!finished())
				process();
		}

		/// @brief Returns whether the coroutine is done executing.
		/// @return Whether coroutine is done executing.
		bool finished() const {
			if (context)
				return context.done();
			return true;
		}

		/// @brief Returns process to the coroutine.
		/// @return Whether coroutine is still processing.
		bool process() const {
			if (finished()) return false;
			context();
			return true;
		}

		/// @brief Default constructor.
		Promise() {}
		
		/// @brief Constructs the coroutine.
		/// @param context Coroutine context.
		Promise(ContextType context): context(context) {}
		/// @brief Copy constructor (deleted).
		Promise(Promise const&) = delete;
		/// @brief Destructor.
		~Promise() {if (context) context.destroy();}

		/// @brief Returns whether the coroutine is still processing.
		operator bool() const {return !finished();}

		/// @brief Returns process to the coroutine.
		/// @return Whether coroutine is still processing.
		bool operator()() {return process();}

		/// @brief Coroutine context.
		ContextType context;
		/// @brief Returns the coroutine context.
		operator Context<promise_type>() const	{return context;}
		/// @brief Returns the coroutine context.
		operator Context<>() const				{return context;}
	};

	/// @brief Coroutine return type.
	/// @tparam TData Return value type.
	/// @tparam S Whether the coroutine should start suspended.
	template<class TData, bool S>
	struct Promise: Typed<TData> {
		using Typed = ::CTL::Typed<TData>;
		
		using
			typename Typed::DataType,
			typename Typed::ReferenceType
		;

		struct promise_type;
		/// @brief Context type.
		using ContextType = Context<promise_type>;

		/// @brief Whether the coroutine should start suspended.
		constexpr static bool START_SUSPENDED = S;

		/// @brief STL promise implementation type.
		struct promise_type {
			/// @brief Result value.
			DataType value;
			
			/// @brief Returns the promise associated with this type.
			/// @return Associated promise.
			Promise get_return_object() {
				return ContextType::from_promise(*this);
			}

			/// @brief Initial suspend.
			/// @return Suspend state.
			Suspend<S> initial_suspend()			{return {};	}
			/// @brief Final suspend.
			/// @return Suspend state.
			AlwaysSuspend final_suspend() noexcept	{return {};	}
			
			/// @brief Unhandle exception processor.
			void unhandled_exception() {
				throw Exception(*Exception::current());
			}

			/// @brief Specialized suspender.
			struct SuspendType {
				virtual bool await_ready()		{return promise.finished();	}
				virtual void await_suspend()	{							}
				virtual DataType await_resume()	{return promise.value();	}
				Promise promise;
			};

			/// @brief `co_await` support.
			SuspendType await_transform(AlwaysSuspend) {
				return SuspendType{get_return_object()};
			}

			/// @brief Yields a value from the coroutine.
			/// @tparam TFrom Value type.
			/// @param v Value to yield.
			/// @return Suspend state.
			template<Type::Convertible<DataType> TFrom>
			AlwaysSuspend yield_value(TFrom&& v) {
				value = v;
				return {};
			}

			/// @brief Returns a value from the coroutine.
			/// @tparam TFrom Value type.
			/// @param v Value to return.
			/// @return Result value.
			template<Type::Convertible<DataType> TFrom>
			DataType return_value(TFrom&& v) {
				value = v;
				return value;
			}
		};
		
		/// @brief Coroutine context.
		ContextType context;

		/// @brief Default constructor.
		Promise() {}

		/// @brief Constructs the coroutine.
		/// @param context Coroutine context.
		Promise(ContextType context): context(context) {}
		/// @brief Copy constructor (deleted).
		Promise(Promise const&) = delete;

		/// @brief Destructor.
		~Promise() {context.destroy();}

		/// @brief Returns the current stored value.
		/// @return Current stored value.
		DataType value() const {return context.promise().value;}

		/// @brief Returns process to the coroutine.
		/// @return Whether coroutine is still processing.
		bool process() const {
			if (finished()) return false;
			context();
			return true;
		}

		/// @brief Fetches the next value, and returns it.
		/// @return Next value.
		DataType next() const {process(); return value();}

		/// @brief Awaits the coroutine to finish executing.
		/// @return Last value returned.
		DataType await() const {
			while (process());
			return value();
		}

		/// @brief Returns whether the coroutine is done executing.
		/// @return Whether coroutine is done executing.
		bool finished() const {
			if (context)
				return context.done();
			return true;
		}

		/// @brief Returns whether the coroutine is still processing.
		operator bool() const {return !finished();	}

		/// @brief Returns process to the coroutine.
		/// @return Whether coroutine is still processing.
		bool operator()() {return process();}

		/// @brief Returns the coroutine context.
		operator Context<promise_type>() const	{return context;}
		/// @brief Returns the coroutine context.
		operator Context<>() const				{return context;}
	};

	/// @brief `Promise` analog for genarator coroutines — coroutines that start suspended.
	template<class T = void>
	using Generator = Promise<T, true>;

	/// @brief `Promise` analog for conventional asynchronous coroutines.
	template<class T = void>
	using Task = Promise<T, false>;

	/// @brief `Promise` analog for "pure coroutines" (`void` return, no initial suspend).
	using Routine = Task<>;
}

CTL_NAMESPACE_END

#endif