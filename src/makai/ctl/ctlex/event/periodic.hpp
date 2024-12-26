#ifndef CTL_EX_EVENT_PERIODIC_H
#define CTL_EX_EVENT_PERIODIC_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/container/list.hpp"
#include "../../ctl/container/function.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Interface for an event that fires periodically.
/// @tparam _ Class associated with the event.
/// @tparam ...Args Event argument types.
template<class _ = void, class... Args>
class IPeriodic: Argumented<Args...> {
public:
	/// @brief Event wrapper.
	using EventWrapper	= Signal<Args...>;
	/// @brief Event list.
	using EventList		= List<EventWrapper const*>;

	/// @brief Constructs the periodic.
	/// @param manual Whether the event is fired manually. By default, it is `false`.
	IPeriodic(bool const manual = false): manual(manual) {
		if (!manual)
			events.pushBack(&update);
	}

	/// @brief Yields all available non-manual periodic events.
	/// @param ...args Values to pass.
	static void process(Args... args) {
		if (events.size())
			for(EventWrapper const* func : events)
				(*func)(args...);
	}

	/// @brief Sets the periodic event to be manually executed.
	void setManual() {
		if (manual) return;
		if (!events.empty())
			events.eraseIf([&](auto const& e){return e == &update;});
		manual = true;
	}

	/// @brief Sets the periodic event to be automatically executed.
	void setAutomatic() {
		if (!manual) return;
		events.pushBack(&update);
		manual = false;
	}

	/// @brief Destructor.
	virtual ~IPeriodic() {
		if (!manual && !events.empty())
			events.eraseIf([&](auto const& e){return e == &update;});
	}

	/// @brief Called when the event needs to be fired. Must be implemented.
	/// @param ...args Passed values.
	virtual void onUpdate(Args... args) = 0;

	/// @brief Returns whether the periodic event is manually executed.
	/// @return Whether the periodic event is manually executed.
	bool isManual() {return manual;}

private:
	/// @brief Event wrapper.
	const EventWrapper update = [this] (Args... args) {onUpdate(args...);};

	/// @brief Whether the periodic event is manually executed.
	bool manual = false;

	/// @brief All automatic events.
	inline static EventList events;
};

CTL_EX_NAMESPACE_END

#endif // CTL_EX_EVENT_PERIODIC_H
