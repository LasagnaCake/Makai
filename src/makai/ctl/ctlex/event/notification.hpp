#ifndef CTL_EX_EVENT_NOTIFICATION_H
#define CTL_EX_EVENT_NOTIFICATION_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/container/functor.hpp"
#include "../../ctl/container/pointer/reference.hpp"
#include "../../ctl/container/strings/string.hpp"
#include "../../ctl/container/map/map.hpp"
#include "../../ctl/container/dictionary.hpp"
#include "../../ctl/container/arguments.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Notifiable object abstract class.
/// @tparam I Server ID.
template<usize I = 0>
class ANotifiable {
public:
	/// @brief Notification message interface.
	struct IMessage {
		virtual ~IMessage() {}
	};

	/// @brief Server ID.
	constexpr static usize ID = I;

	/// @brief Message handle type.
	typedef Reference<IMessage const>	MessageHandleType;
	/// @brief Signal list type.
	typedef ref<ANotifiable>			ReceiverType;
	/// @brief Signal list type.
	typedef List<ReceiverType>			ReceiverList;
	/// @brief Signal database type.
	typedef Dictionary<ReceiverList>	SignalDatabase;

	/// @brief Default constructor.
	ANotifiable() {}

	/// @brief Subscribes the object to a signal.
	/// @param signal Signal name.
	ANotifiable(String const& signal)		{subscribeTo(signal);	}
	/// @brief Subscribes the object to a series of signals.
	/// @param signals Signals to attach to.
	ANotifiable(StringList const& signals)	{subscribeTo(signals);	}

	/// @brief Destructor.
	virtual ~ANotifiable() {unsubscribeFromAll();}

	/// @brief Gets alled when this object receives a message. MUST be implemented.
	/// @param event Signal that fired the message.
	/// @param message Message associated with the signal.
	virtual void onMessage(String const& signal, MessageHandleType const& message) = 0;

	/// @brief Receives a message from a signal.
	/// @param event Signal that fired the message.
	/// @param message Message associated with the signal.
	void receive(String const& signal, MessageHandleType const& message) {
		onMessage(signal, message);
	}

	/// @brief Subscribes this object to a signal.
	/// @param signal Signal name.
	/// @param action Action for the signal.
	/// @return Reference to self.
	ANotifiable& subscribeTo(String const& signal) {
		db[signal].pushBack(this);
		added[signal] = true;
		return *this;
	}

	/// @brief Subscribes this object to a series of signals.
	/// @param signals Signals to attach to.
	/// @param action Action for the signal.
	/// @return Reference to self.
	ANotifiable& subscribeTo(StringList const& signals) {
		for (auto& signal: signals)
			subscribeTo(signal, this);
		return *this;
	}

	/// @brief Unsubscribes this object from a signal.
	/// @param signal Signal to unsubscribe.
	/// @return Reference to self.
	ANotifiable& unsubscribeFrom(String const& signal) {
		if (db.contains(signal) && added.contains(signal))
			db[signal].eraseLike(this);
		added.erase(signal);
		return *this;
	}

	/// @brief Unsubscribes this object from a series of signals.
	/// @param signals Signals to unsubscribe.
	/// @return Reference to self.
	ANotifiable& unsubscribeFrom(StringList const& signals) {
		for (String const& s: signals)
			unsubscribeFrom(s);
		return *this;
	}

	/// @brief Unsubscribes this object from a series of signals.
	/// @tparam ...Args Argument types. 
	/// @param ...signals Signals to unsubscribe.
	/// @return Reference to self.
	template <typename... Args>
	ANotifiable& unsubscribeFrom(Args const&... signals)
	requires (... && Type::Convertible<Args, String>) {
		(..., unsubscribe(signals));
		return *this;
	}

	/// @brief Unsubscribes from all signals this object registere to.
	/// @return Reference to self.
	ANotifiable& unsubscribeFromAll() {
		for (auto [name, isAdded]: added.items())
			if (isAdded) unsubscribeFrom(name);
		added.clear();
		return *this;
	}

	/// @brief Broadcasts a message to a signal.
	/// @param signals Signal to broadcast.
	/// @param msg Message to pass.
	static void broadcast(String const& signal, MessageHandleType const& msg = nullptr) {
		if (db.contains(signal))
			for (ReceiverType s: db[signal])
				s->receive(signal, msg);
	}

	/// @brief Broadcasts a message to a series of signals.
	/// @param signals Signals to broadcast.
	/// @param msg Message to pass.
	static void broadcast(StringList const& signals, MessageHandleType const& msg = nullptr) {
		for (String const& s: signals)
			broadcast(s, msg);
	}

	/// @brief Broadcasts a series of notifications.
	/// @param notifs Pairs of notifications and their messages.
	static void broadcast(List<KeyValuePair<String, MessageHandleType>> const& notifs) {
		for (auto const& [sig, msg]: notifs)
			broadcast(sig, msg);
	}

private:
	/// @brief Signals this object is registered to.
	Map<String, bool> added;

	/// @brief All registered notifications.
	inline static SignalDatabase db;
};

/// @brief Specialized notification handler.
/// @tparam ...Args Message argument types.
/// @note Only allows one action to be registered per signal. If you need multiple actions, use `NotificationServer`.
template<typename... Args>
struct Notification {
	typedef Signal<Args...>	SignalType;

	/// @brief Default constructor.
	Notification() {}
	
	/// @brief Registers which signal this notification should fire.
	/// @param name Signal name.
	Notification(String const& name): id(name)											{}
	/// @brief Subscribes an action to a signal.
	/// @param name Signal name.
	/// @param action Action to perform for the signal.
	Notification(String const& name, SignalType const& action): Notification(name)		{db[id] = (func = action);}
	/// @brief Copy constructor.
	/// @param other `Notification` to get the name from.
	/// @return Reference to self.
	Notification(Notification const& other): Notification(other.id)						{}
	/// @brief Move constructor.
	/// @param other `Notification` to move.
	/// @return Reference to self.
	Notification(Notification&& other): Notification(other.id, other.func)				{}

	/// @brief Registers which signal this notification should fire.
	/// @param name Signal name.
	/// @return Reference to self.
	Notification& operator=(String const& name)			{id = name; return *this;								}
	/// @brief Copy assignment operator.
	/// @param other `Notification` to get the name from.
	/// @return Reference to self.
	Notification& operator=(Notification const& other)	{id = other.id; return *this;							}
	/// @brief Move assignment operator.
	/// @param other `Notification` to move.
	/// @return Reference to self.
	Notification& operator=(Notification&& other)		{id = other.id; moveFunction(other.func); return *this;	}

	/// @brief Destructor.
	~Notification() {clear();}

	/// @brief Emits a signal.
	/// @param ...args Values to pass for the signal.
	/// @return Reference to self.
	Notification const& emit(Args... args) const		{db[id](args...); return *this;	}
	/// @brief Emits a signal.
	/// @param ...args Values to pass for the signal.
	/// @return Reference to self.
	Notification& emit(Args... args)					{db[id](args...); return *this;	}
	/// @brief Emits a signal.
	/// @param ...args Values to pass for the signal.
	/// @return Reference to self.
	Notification const& operator()(Args... args) const	{return emit(args...);			}
	/// @brief Emits a signal.
	/// @param ...args Values to pass for the signal.
	/// @return Reference to self.
	Notification& operator()(Args... args)				{return emit(args...);			}

private:
	void clear() {
		if (db[id] == func) db[id] = SignalType();
	}

	void moveFunction(SignalType& f) {
		if (f) {
			db[id] = (func = f);
			f = SignalType();
		}
	}

	Notification(String const& name, SignalType& action): Notification(name) {moveFunction(action);}

	/// @brief Registered signal, if owner.
	SignalType	func;
	/// @brief Signal name.
	String		id;

	/// @brief All registered signals.
	inline static Dictionary<SignalType> db;
};

CTL_EX_NAMESPACE_END

#endif // CTL_EX_EVENT_NOTIFICATION_H
