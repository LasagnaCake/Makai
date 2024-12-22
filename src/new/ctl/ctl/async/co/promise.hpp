#ifndef CTL_ASYNC_CO_PROMISE_H
#define CTL_ASYNC_CO_PROMISE_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"
#include "context.hpp"
#include "suspend.hpp"
#include "../../cpperror.hpp"

// Based off of: https://www.scs.stanford.edu/~dm/blog/c++-coroutines.html#compiling-code-using-coroutines

CTL_NAMESPACE_BEGIN

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

			Suspend<S> initial_suspend()			{return {};	}
			AlwaysSuspend final_suspend() noexcept	{return {};	}
			AlwaysSuspend yield_value()				{return {};	}
			void return_void()						{			}
			
			void unhandled_exception() {
				throw Exception(*Exception::current());
			}
		};

		void await() const {
			while (!done())
				process();
		}

		bool done() const {
			return context.done();
		}

		void process() const {context();}
		
		Promise(ContextType context): context(context) {}
		Promise(Promise const&) = delete;
		~Promise() {context.destroy();}

		operator bool() const {return !done();}

		ContextType context;
		operator Context<promise_type>() const	{return context;}
		operator Context<>() const				{return context;}
	};

	/// @brief Coroutine return type.
	/// @tparam TData Return value type.
	/// @tparam S Whether the coroutine should start suspended.
	template<class TData, bool S>
	struct Promise: Typed<TData> {
		using Typed = ::CTL::Typed<TData>;
		
		using typename Typed::DataType;

		struct promise_type;
		using ContextType = Context<promise_type>;

		constexpr static bool START_SUSPENDED = S;

		struct promise_type {
			DataType value;

			~promise_type() {}
			
			Promise get_return_object() {
				return ContextType::from_promise(*this);
			}

			Suspend<S> initial_suspend()			{return {};	}
			AlwaysSuspend final_suspend() noexcept	{return {};	}
			
			void unhandled_exception() {
				throw Exception(*Exception::current());
			}

			template<Type::Convertible<DataType> TFrom>
			AlwaysSuspend yield_value(TFrom&& v) {
				value = v;
				return {};
			}

			template<Type::Convertible<DataType> TFrom>
			DataType return_value(TFrom&& v) {
				value = v;
				return value;
			}
		};
		
		ContextType context;

		Promise(ContextType context): context(context) {}
		Promise(Promise const&) = delete;

		~Promise() {context.destroy();}

		DataType value() const {return context.promise().value;}

		void process() const {context();}

		DataType next() const {process(); return value();}

		DataType await() const {
			while (!done())
				process();
			return value();
		}

		bool done() const {
			return context.done();
		}

		operator bool() const		{return !done();	}
		operator DataType() const	{return value();	}

		operator Context<promise_type>() const	{return context;}
		operator Context<>() const				{return context;}

		using Context = Co::Context<promise_type>;
	};
}

CTL_NAMESPACE_END

#endif