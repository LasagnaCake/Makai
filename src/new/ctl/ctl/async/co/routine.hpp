#ifndef CTL_ASYNC_CO_ROUTINE_H
#define CTL_ASYNC_CO_ROUTINE_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"
#include "../../container/functor.hpp"
#include "../../container/list.hpp"
#include "register.hpp"

// Implementation based off of Greeny: https://github.com/nifigase/greeny

CTL_NAMESPACE_BEGIN

/// @brief Cooperative routine facilities.
namespace Co {
	struct Context;

	/// @brief Green thread.
	struct Routine {
	private:
		enum class Status {
			RS_FINISHED,
			RS_NEW,
			RS_READY,
			RS_POSTED
		};

		/// @brief Function to call.
		Signal<>				call;
		/// @brief Function stack.
		MemorySlice<byte> const	stack;
		/// @brief Routine status.
		Status					status = Status::RS_NEW;
		/// @brief Routine id.
		usize					routineID;
		/// @brief Stack pointer register copy.
		Registers				registers;

		explicit Routine(Signal<> const& fun, usize const stackSize, usize const id):
		call(fun), stack(stackSize), routineID(id), registers(nullptr) {
			registers = Registers(pointer(stack.data() + stack.size()));
		}

		explicit Routine(usize const id): stack(0), routineID(id), registers(nullptr) {}

		Routine()				= delete;
		Routine(Routine const&)	= delete;

		usize id() const {
			return routineID;
		}

		friend class Context;
	public:

		/// @brief Destructor.
		~Routine() {}
	};
}

CTL_NAMESPACE_END

#endif