#ifndef CTL_EX_ASYNC_CO_ROUTINE_H
#define CTL_EX_ASYNC_CO_ROUTINE_H

#include "../../../ctl/exnamespace.hpp"
#include "../../../ctl/ctl.hpp"
#include "../../event/playable.hpp"
#include "promise.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Cooperative routine facilities.
namespace Co {
	/// @brief Asynchronous task interface.
	struct IRoutine {
		/// @brief Task state.
		enum class State {
			RS_READY,
			RS_RUNNING,
			RS_FINISHED
		};

		/// @brief Promise type.
		using PromiseType = CTL::Co::Promise<usize, true>;

		/// @brief task to process. Must be implemented.
		/// @return Promise to task result.
		/// @note
		///		Ideally, should not use `CTL::Co::yield` and `CTL::Co::Yielder`.
		///		Instead, simply `co_yield` the delay.
		virtual PromiseType run() = 0;

		/// @brief Empty constructor.
		IRoutine() {}

		/// @brief Processes the assiged routine.
		void process() {
			if (taskState == State::RS_READY) {
				prommy = run();
				taskState = State::RS_RUNNING;
			}
			if (taskState != State::RS_FINISHED && !paused) {
				if (!counter) {
					if (!prommy)					counter = prommy.next();
					else if (repeat && loops != 0)	loops--;
					else							taskState = State::RS_FINISHED;
				}
				if (counter == 0)
					process();
				else --counter;
			}
		}

		/// @brief Returns the current routine state.
		/// @return Current state.
		State state() const {
			return taskState;
		}

		/// @brief Whether the current routine is paused.
		bool	paused	= false;
		/// @brief Whether to repeatedly fire the event.
		bool	repeat	= false;
		/// @brief The amount of times to repeat for. If less than 0, loops indefinitely.
		llong	loops	= -1;

	private:
		/// @brief Underlying coroutine promise type.
		PromiseType	prommy;
		/// @brief The task's current state.
		State		taskState	= State::RS_READY;
		/// @brief The task's internal counter.
		usize		counter		= 0;
	};
}

CTL_EX_NAMESPACE_END

#endif