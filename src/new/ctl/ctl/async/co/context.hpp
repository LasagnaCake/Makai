#ifndef CTL_ASYNC_CO_CONTEXT_H
#define CTL_ASYNC_CO_CONTEXT_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"
#include "../../container/function.hpp"
#include "../../container/list.hpp"
#include "routine.hpp"

#include <iostream>

// Implementation based off of Greeny: https://github.com/nifigase/greeny

CTL_NAMESPACE_BEGIN

/// @brief Cooperative routine facilities.
namespace Co {
	/// @brief Routine context.
	struct Context {
	private:
		/// @brief Routines associated with this context.
		List<Routine*>	routines;
		/// @brief Current routine being processed.
		usize			current		= 0;
		/// @brief Count of active coroutines.
		usize			active		= 0;

		Routine& get(usize const id) {
			if (!id || !(id < routines.size()))
				throw OutOfBoundsException("Routine does not exist!");
			return *routines[id-1];
		}

		/*usize next(Routine::Status const status) {
			for (usize i = current + 1; i != current; ++i) {
				if (i < routines.size()) {
					i = 0;
					continue;
				}
				Routine& r = get(i);
				if (r.status == status)
					return i;
			}
			return 0;
		}*/

		template<usize N>
		usize next(Decay::AsType<Routine::Status[N]> const& status) {
			if (routines.empty()) return 0;
			for (usize i = current + 1; i != current; ++i) {
				if (i >= routines.size()) {
					i = 0;
					continue;
				}
				Routine& r = get(i);
				for (usize j = 0; j < N; ++j)
					if (r.status == status[j])
						return i;
			}
			return 0;
		}

		void run() {
			Routine& r = get(current);
			if (r.status == Routine::Status::RS_NEW) {
				r.status = Routine::Status::RS_READY;
				++active;
				r.call();
				--active;
				r.status = Routine::Status::RS_FINISHED;
				yield(false);
			}
		}

		/// @brief Yields the current routine.
		/// @param post Whether to post a message.
		void yield(bool const post) {
			if (routines.empty()) return;
			Routine& r = get(current);
			if (post)
				r.status = Routine::Status::RS_POSTED;
			usize nextID = next({
				Routine::Status::RS_NEW,
				Routine::Status::RS_READY
			});
			if (!nextID) return;
			current = nextID;
			r.registers.pull();
			Context* nextContext = static_cast<Context*>(r.registers.push(this));
			nextContext->run();
		}

	public:
		/// @brief Default routine stack size.
		constexpr static usize STACK_SIZE = 1024*8;

		/// @brief Empty constructor.
		Context() {
			routines.pushBack(new Routine(1));
			routines.front()->status = Routine::Status::RS_READY;
			current = 1;
		}

		/// @brief Copy constructor (deleted).
		Context(Context const&) = delete;

		/// @brief Destructor.
		~Context() {
			for (Routine* r: routines)
				delete r;
		}

		/// @brief Releases execution.
		void yield() {
			yield(true);
		}

		/// @brief Spawns a routine.
		/// @tparam F Callable type.
		/// @param func Function to run.
		/// @param stack Stack size. By default, it is `STACK_SIZE`.
		/// @return ID of spawned routine.
		usize spawn(Signal<> const& func, usize const stack = STACK_SIZE) {
			const usize id = routines.size() + 1;
			routines.pushBack(new Routine(func, stack, id));
			return id;
		}

		/// @brief Waits for the given routine to yield.
		/// @param id Routine to wait for.
		/// @return Whether the routine finished execution.
		/// @throw OutOfBoundsException if routine does not exist.
		bool waitFor(usize const id) {
			Routine& r = get(id);
			while (true) {
				switch(r.status) {
					case Routine::Status::RS_NEW:
					case Routine::Status::RS_READY:
					yield(false);
					continue;
					case Routine::Status::RS_POSTED:
					r.status = Routine::Status::RS_READY;
					return true;
					case Routine::Status::RS_FINISHED:
					return false;
				}
			}
		}

		/// @brief Releases execution, and waits for any other routine to yield.
		/// @return Routine that yielded, or zero if all routines are finished.
		usize waitForNext() {
			while (true) {
				usize id = next({Routine::Status::RS_POSTED});
				if (id) {
					get(id).status = Routine::Status::RS_READY;
					return id;
				}
				id = next({
					Routine::Status::RS_NEW,
					Routine::Status::RS_READY
				});
				if (id) {
					yield(false);
					continue;
				}
				break;
			}
			return 0;
		}

		/// @brief Waits for all routines to finish execution.
		/// @warning Should ONLY be called in the context's main thread.
		void join() {
			yield(false);
			while (active)
				yield(false);
		}
	};
}

CTL_NAMESPACE_END

#endif