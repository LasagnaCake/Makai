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
	template<class TData = void>
	struct Promise;

	template<>
	struct Promise<void> {
		struct promise_type {
			Promise get_return_object() {
				return {
					.context = Context<promise_type>::from(*this)
				};
			}
			NeverSuspend initial_suspend()			{return {};	}
			NeverSuspend final_suspend() noexcept	{return {};	}
			AlwaysSuspend yield_value()				{			}
			void return_void()						{			}

			void unhandled_exception() {
				if (Exception::current())
					throw Exception(*Exception::current());
			}
		};
		Context<promise_type> context;
		constexpr operator Context<promise_type>() const	{return context;}
		constexpr operator Context<>() const				{return context;}
	};

	template<class TData>
	struct Promise: Typed<TData> {
		using Typed = ::CTL::Typed<TData>;
		
		using typename Typed::DataType;
		
		~Promise() {context.destroy();}

		struct promise_type {
			Nullable<DataType> value = nullptr;

			~promise_type() {}
			
			Handler get_return_object() {
				return {
					.context = Context<promise_type>::from(*this)
				};
			}

			NeverSuspend initial_suspend()			{return {};	}
			NeverSuspend final_suspend() noexcept	{return {};	}
			void unhandled_exception()				{			}

			AlwaysSuspend yield_value(DataType const& value) {
				this->value = value;
				return {};
			}

			Nullable<DataType> return_value() {
				return value;
			}

			void return_void() {
			}
		};

		using ContextType = Context<promise_type>;
		
		ContextType context;

		constexpr Promise(ContextType context): context(context) {}

		constexpr operator Context<promise_type>() const	{return context;}
		constexpr operator Context<>() const				{return context;}

		using Context = Co::Context<promise_type>;
	};
}

CTL_NAMESPACE_END

#endif