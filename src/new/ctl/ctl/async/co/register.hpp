#ifndef CTL_ASYNC_CO_REGISTER_H
#define CTL_ASYNC_CO_REGISTER_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"
#include "../../container/function.hpp"
#include "../../container/list.hpp"

// Implementation based off of Greeny: https://github.com/nifigase/greeny

CTL_NAMESPACE_BEGIN

/// @brief Asynchronous green thread facilities.
namespace Co {
	/// @brief CPU registers.
	/// @warning Only `Context` and `Routine` should use this!
	struct Registers {
		/// @brief Pointers to stack data.
		pointer sp, bp;

	protected:
		/// @brief Puts data in the stack.
		/// @param what Data to put in the stack.
		/// @return Pointer to stack.
		inline pointer put(pointer const what) {
			void* stack = static_cast<char*>(sp) - sizeof(void*);
			MX::memcpy(stack, &what, sizeof(void*));
			return stack;
		}

	public:
		/// @brief Constructs the stack pointer register.
		/// @param stack Pointer to stack.
		explicit Registers(pointer const stack): sp(stack), bp(sp) {}

		#ifdef X86
		#if (CPU_ARCH == 64)
		/// @brief Stores the current stack pointers.
		[[gnu::always_inline]]
		inline void pull() {
			__asm__ __volatile__(
				"movq %%rsp, %0\n"
				"movq %%rbp, %1\n" : "=m"(sp), "=m"(bp)
			);
		}
		/// @brief Sets the current stack pointers.
		[[gnu::always_inline]]
		inline pointer push(pointer preserve) {
			void* stack = put(preserve);
				__asm__ __volatile__(
				"movq %1, %%rsp\n"
				"movq %2, %%rbp\n"
				"popq %0\n" : "+r"(preserve) : "m"(stack), "m"(bp) : "memory"
			);
			return preserve;
		}
		#elif (CPU_ARCH == 32)
		/// @brief Stores the current stack pointers.
		[[gnu::always_inline]]
		inline void pull() {
			__asm__ __volatile__(
				"mov %%esp, %0\n"
				"mov %%ebp, %1\n" : "=m"(sp), "=m"(bp)
			);
		}
		/// @brief Sets the current stack pointers.
		[[gnu::always_inline]]
		inline pointer push(pointer preserve) {
			void* stack = put(preserve);
				__asm__ __volatile__(
				"mov %1, %%esp\n"
				"mov %2, %%ebp\n"
				"pop %0\n" : "+r"(preserve) : "m"(stack), "m"(bp) : "memory"
			);
			return preserve;
		}
		#endif
		#else
		#error "Routines are currently only supported on X86 architectures!"
		#endif
	};
}

CTL_NAMESPACE_END

#endif