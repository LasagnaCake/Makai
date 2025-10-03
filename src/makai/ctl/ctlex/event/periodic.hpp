#ifndef CTL_EX_EVENT_PERIODIC_H
#define CTL_EX_EVENT_PERIODIC_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/container/lists/list.hpp"
#include "../../ctl/container/functor.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Interface for an event that fires periodically.
/// @tparam _ Class associated with the event.
/// @tparam ...Args Event argument types.
template<class _ = void, class... Args>
class APeriodic: Argumented<Args...> {
public:
	/// @brief Event list.
	using EventList		= List<ref<APeriodic>>;

	/// @brief Constructs the periodic.
	/// @param manual Whether the event is fired manually. By default, it is `false`.
	APeriodic(bool const manual = false): manual(manual) {
		if (!manual)
			events.pushBack(this);
	}

	/// @brief Yields all available non-manual periodic events.
	/// @param ...args Values to pass.
	static void process(Args... args) {
		if (events.size()) {
			// Done this way because the list may be modified during another object's update cycle
			for(usize i = 0; i < events.size(); ++i)
				if (events[i]) autoUpdate(*events[i], args...);
			cleanup();
		}
	}

	/// @brief Cleans the periodic events list.
	static void cleanup() {
		events.eraseLike(nullptr);
	}

	/// @brief Sets the periodic event to be manually executed.
	void setManual() {
		if (manual) return;
		if (!events.empty())
			events.replace(this, nullptr);
		manual = true;
	}

	/// @brief Sets the periodic event to be automatically executed.
	void setAutomatic() {
		if (!manual) return;
		events.pushBack(this);
		manual = false;
	}

	/// @brief Destructor.
	virtual ~APeriodic() {
		if (!manual && !events.empty())
			events.replace(this, nullptr);
	}

	/// @brief Returns whether the periodic event is manually executed.
	/// @return Whether the periodic event is manually executed.
	bool isManual() {return manual;}

	/// @brief Whether the periodic event is updating.
	bool updating = true;

	/// @brief Yields an update cycle for the periodic.
	/// @param ...args Passed values.
	void update(Args... args) {
		if (updating) onUpdate(args...);
	}

protected:
	/// @brief Called when the event needs to be fired. Must be implemented.
	/// @param ...args Passed values.
	virtual void onUpdate(Args... args) = 0;

private:
	static void autoUpdate(APeriodic& periodic, Args... args) {periodic.update(args...);}

	/// @brief Whether the periodic event is manually executed.
	bool manual = false;

	/// @brief All automatic events.
	inline static EventList events;
};

CTL_EX_NAMESPACE_END

#endif // CTL_EX_EVENT_PERIODIC_H
