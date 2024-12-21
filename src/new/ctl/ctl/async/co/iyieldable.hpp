#ifndef CTL_ASYNC_CO_IYIELDABLE_H
#define CTL_ASYNC_CO_IYIELDABLE_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"
#include "context.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Cooperative routine facilities.
namespace Co {
	/// @brief Yieldable object interface.
	struct IYieldable {
		IYieldable(Context& context): context(context), routine(context.spawn([&](Context&){run();})) {}

		virtual ~IYieldable() {}

		virtual void run() = 0;

	protected:
		void yield(usize count) {
			while(count-- > 0)
				context.yield();
		}

		bool waitFor(IYieldable const& other) {
			return context.waitFor(other.routine);
		}

		usize id() const {
			return routine;
		}

	private:
		Context& context;

		usize const routine;
	};
}

CTL_NAMESPACE_END

#endif