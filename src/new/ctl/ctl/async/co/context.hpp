#ifndef CTL_ASYNC_CO_CONTEXT_H
#define CTL_ASYNC_CO_CONTEXT_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"
#include "../../templates.hpp"
#include "../../order.hpp"
#include "../../typetraits/traits.hpp"

// Based off of: https://github.com/gcc-mirror/gcc/blob/7d83a32aacd6005c0c038c74562e35d70f6a77a8/libstdc%2B%2B-v3/include/std/coroutine#L264

CTL_NAMESPACE_BEGIN

/// @brief Cooperative routine facilities.
namespace Co {
	template<class T = void>
	struct Context;

	template<>
	struct Context<nulltype> {
		constexpr operator Context<void>() const noexcept {
			return Context<void>::from(frame);
		}

		constexpr void operator()() const		{}
		constexpr bool done() const noexcept	{}
		constexpr void resume() const			{}
		constexpr void destroy() const			{}

		constexpr nulltype promise() const noexcept {return nulltype();}

	private:
		friend Context noContext() noexcept;

		struct KingdomOfNothingness {
			constexpr static void nothingness() {};
			void (*r) = nothingness;
			void (*d) = nothingness;
			nulltype p;
		};

		static KingdomOfNothingness kon;

		explicit Context() noexcept = default;

		pointer frame = &kon;
	};

	template<Type::NonNull TPromise>
	struct Context<TPromise>: Ordered {
		using PromiseType = TPromise;

		constexpr Context() noexcept {}

		constexpr Context(nulltype) noexcept {}

		constexpr Context& operator=(nulltype) noexcept {
			frame = nullptr;
			return *this;
		}

		constexpr static Context from(pointer const address) noexcept
		requires (Type::Void<PromiseType>) {
			Frame self;
			self.frame = address;
			return self;
		}

		constexpr static Context from(Promise& promise) noexcept
		requires (Type::NonVoid<PromiseType>) {
			Frame self;
			self.frame = &promise;
			return self;
		}

		constexpr pointer address() noexcept {
			return frame;
		}

		constexpr operator bool() const noexcept {
			return frame;
		}

		constexpr void operator()() const {return resume();}

		constexpr operator Context<void>() const noexcept {
			return Context<void>::from(frame);
		}

		constexpr bool done() const noexcept	{return __builtin_coro_done(frame);	}
		constexpr void resume() const			{__builtin_coro_resume(frame);		}
		constexpr void destroy() const			{__builtin_coro_destroy(frame);		}

		constexpr bool operator==(Context const& other) {
			return other.frame == frame;
		}

		constexpr Ordered::OrderType operator<=>(Context const& other) {
			return frame <=> other.frame;
		}

		template<Type::NonVoid T>
		constexpr T& promise() requires Type::Equal<T, PromiseType> {
			return *static_cast<T*>(
				__builtin_coro_promise(frame, __alignof(T), false)
			);
		}

	protected:
		pointer frame = nullptr;
	};
}

CTL_NAMESPACE_END

#endif