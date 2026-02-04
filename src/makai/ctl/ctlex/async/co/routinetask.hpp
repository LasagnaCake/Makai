#ifndef CTL_EX_ASYNC_CO_ROUTINETASK_H
#define CTL_EX_ASYNC_CO_ROUTINETASK_H

#include "../../../ctl/exnamespace.hpp"
#include "../../../ctl/ctl.hpp"
#include "../../event/playable.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Cooperative routine facilities.
namespace Co {
	/// @brief Specialized coroutine task interface.
	struct ARoutineTask: IPlayable {
		/// @brief Routine state.
		enum class State {
			RS_READY,
			RS_RUNNING,
			RS_FINISHED
		};

		/// @brief Promise type.
		using PromiseType = CTL::Co::Promise<usize, true>;

		/// @brief Destructor.
		virtual ~ARoutineTask() {}

		/// @brief Default constructor.
		ARoutineTask() {}

		/// @brief Copy constructor (deleted).
		ARoutineTask(ARoutineTask const&)	= delete;
		/// @brief Move constructor (deleted).
		ARoutineTask(ARoutineTask&&)		= delete;

		/// @brief Processes the assiged task.
		void process() {
			if (taskState == State::RS_READY)
				start();
			if (taskState == State::RS_RUNNING && !paused) {
				if (!counter) {
					if (prommy)
						counter = prommy.next();
					else if (repeat && loops != 0)	{
						prommy = task();
						if (loops < 0) --loops;
					} else stop();
				}
				if (counter == 0)
					process();
				else --counter;
			}
		}

		/// @brief Starts the routine.
		/// @return Reference to self.
		ARoutineTask& start() override final {
			prommy = task();
			taskState = State::RS_RUNNING;
			isFinished = false;
			return *this;
		}

		/// @brief Unpauses the routine.
		/// @return Reference to self.
		ARoutineTask& play() override final {
			paused = false;
			return *this;
		}

		/// @brief Pauses the routine.
		/// @return Reference to self.
		ARoutineTask& pause() override final {
			paused = true;
			return *this;
		}

		/// @brief Stops the routine.
		/// @return Reference to self.
		ARoutineTask& stop() override final {
			taskState = State::RS_FINISHED;
			isFinished = true;
			return *this;
		}

		/// @brief Stops the routine, while waiting for the underlying task to end processing.
		/// @return Reference to self.
		ARoutineTask& finalize() {
			prommy.await();
			return stop();
		}

		/// @brief Returns the current routine state.
		/// @return Current state.
		State state() const {return taskState;}

		/// @brief Whether to repeatedly fire the event.
		bool	repeat	= false;
		/// @brief The amount of times to repeat for. If less than 0, loops indefinitely.
		ssize	loops	= -1;

	protected:
		/// @brief Task to process. Must be implemented.
		/// @return Promise to task result.
		/// @note
		///		Ideally, should not use `CTL::Co::yield` and `CTL::Co::Yielder`.
		///		Instead, simply `co_yield` the delay.
		virtual PromiseType task() = 0;

		/// @brief Do-nothing task.
		/// @return Promise to task result.
		PromiseType doNothing() {
			co_return 1;
		}

	private:
		/// @brief Underlying coroutine task.
		PromiseType	prommy;
		/// @brief The routine's current state.
		State		taskState	= State::RS_READY;
		/// @brief The routine's internal counter.
		usize		counter		= 0;
	};
}

CTL_EX_NAMESPACE_END

#endif
