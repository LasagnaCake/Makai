#ifndef CTL_EX_EVENT_NOTIFICATION_H
#define CTL_EX_EVENT_NOTIFICATION_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/container/functor.hpp"
#include "../../ctl/container/string.hpp"
#include "../../ctl/container/map.hpp"
#include "../../ctl/container/dictionary.hpp"
#include "../../ctl/container/arguments.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Notification server.
/// @tparam I Server ID.
template<usize I>
class NotificationServer {
public:
	/// @brief Interface for the notification's message.
	struct IMessage {
		virtual ~IMessage() {}
	};

	/// @brief Server ID.
	constexpr static usize ID = I;

	/// @brief Message handle type
	typedef IMessage const*								MessageHandleType;
	/// @brief Signal type.
	typedef Signal<MessageHandleType const>				SignalType;
	/// @brief Signal list type.
	typedef List<SignalType>							SignalList;
	/// @brief Signal arguments type.
	typedef Arguments<SignalType>						SignalArguments;
	/// @brief Signal wrapper type.
	typedef Functor<typename SignalType::FunctionType>	SignalWrapper;
	/// @brief Signal database type.
	typedef Dictionary<List<SignalWrapper>>				SignalDatabase;

	/// @brief Empty constructor.
	NotificationServer() {}

	/// @brief Subscribes an action to a signal.
	/// @param signal Signal name.
	/// @param action Action to perform for the signal.
	NotificationServer(String const& signal, SignalType const& action)					{subscribe(signal, action);		}
	/// @brief Subscribes a series of actions to a signal.
	/// @param signal Signal name.
	/// @param action Actions to perform for the signal.
	NotificationServer(String const& signal, SignalList const& actions)					{subscribe(signal, actions);	}
	/// @brief Subscribes a series of actions to a signal.
	/// @param signal Signal name.
	/// @param action Actions to perform for the signal.
	NotificationServer(String const& signal, SignalArguments const& actions)			{subscribe(signal, actions);	}

	/// @brief Registers a series of signals.
	/// @param signals Pairs of signals and their series of actions.
	NotificationServer(Dictionary<SignalList> const& signals)							{subscribe(signals);			}
	/// @brief Registers a series of signals.
	/// @param signals Pairs of signals and their series of actions.
	NotificationServer(Arguments<KeyValuePair<String, SignalList>> const& signals)		{subscribe(signals);			}
	/// @brief Registers a series of signals.
	/// @param signals Pairs of signals and their series of actions.
	NotificationServer(Arguments<KeyValuePair<String, SignalArguments>> const& signals)	{subscribe(signals);			}

	/// @brief Destructor.
	~NotificationServer() {
		unsubscribeFromAll();
	}

	/// @brief Subscribes an action to a signal.
	/// @param signal Signal name.
	/// @param action Action for the signal.
	/// @return Reference to self.
	NotificationServer& subscribe(String const& signal, SignalType const& action) {
		added[signal].pushBack(action);
		db[signal].pushBack(action);
		return *this;
	}

	/// @brief Subscribes a series of actions to a signal.
	/// @param signal Signal name.
	/// @param action Actions to perform for the signal.
	/// @return Reference to self.
	NotificationServer& subscribe(String const& signal, SignalList const& actions) {
		for (SignalType const& a: actions) {
			added[signal].pushBack(a);
			db[signal].pushBack(a);
		}
		return *this;
	}

	/// @brief Subscribes a series of actions to a signal.
	/// @param signal Signal name.
	/// @param action Actions to perform for the signal.
	/// @return Reference to self.
	NotificationServer& subscribe(String const& signal, SignalArguments const& actions) {
		for (SignalType const& a: actions) {
			added[signal].pushBack(a);
			db[signal].pushBack(a);
		}
		return *this;
	}

	/// @brief Subscribes to a series of signals.
	/// @param signals Pairs of signals and their series of actions.
	/// @return Reference to self.
	NotificationServer& subscribe(Dictionary<SignalList> const& signals) {
		for (auto& [name, actions]: signals)
			subscribe(name, actions);
		return *this;
	}

	/// @brief Subscribes to a series of signals.
	/// @param signals Pairs of signals and their series of actions.
	/// @return Reference to self.
	NotificationServer& subscribe(Arguments<KeyValuePair<String, SignalList>> const& signals) {
		for (auto& [name, actions]: signals)
			subscribe(name, actions);
		return *this;
	}

	/// @brief Subscribes to a series of signals.
	/// @param signals Pairs of signals and their series of actions.
	/// @return Reference to self.
	NotificationServer& subscribe(Arguments<KeyValuePair<String, SignalArguments>> const& signals) {
		for (auto& [name, actions]: signals)
			subscribe(name, actions);
		return *this;
	}

	/// @brief Unsubscribes all actions this object registered for a signal.
	/// @param signal Signal to unsubscribe.
	/// @return Reference to self.
	NotificationServer& unsubscribe(String const& signal) {
		if (db.contains(signal)) {
			auto& al = added[signal];
			db[signal].eraseIf([&] (auto const& elem) {return al.find(elem) != -1;});
		}
		added[signal].clear();
		return *this;
	}

	/// @brief Unsubscribes all actions this object registered for a series of signals.
	/// @param signals Signals to unsubscribe.
	/// @return Reference to self.
	NotificationServer& unsubscribe(StringList const& signals) {
		for (String const& s: signals)
			unsubscribe(s);
		return *this;
	}

	/// @brief Unsubscribes all actions this object registered for a series of signals.
	/// @param signals Signals to unsubscribe.
	/// @return Reference to self.
	NotificationServer& unsubscribe(StringArguments const& signals) {
		for (String const& s: signals)
			unsubscribe(s);
		return *this;
	}

	/// @brief Unsubscribes all actions this object registered for a series of signals.
	/// @tparam ...Args Argument types. 
	/// @param ...signals Signals to unsubscribe.
	/// @return Reference to self.
	template <typename... Args>
	NotificationServer& unsubscribe(Args const&... signals)
	requires (... && Type::Equal<Args, String>) {
		(..., unsubscribe(signals));
		return *this;
	}

	/// @brief Unsubscribes all actions this object registered.
	/// @return Reference to self.
	NotificationServer& unsubscribeFromAll() {
		for (auto& [name, lst]: db) {
			if (!added.contains(name))
				continue;
			auto& al = added[name];
			lst.eraseIf([&] (auto const& elem) {return al.find(elem) != -1;});
		}
		added.clear();
		return *this;
	}

	/// @brief Broadcasts a message to a signal.
	/// @param signals Signal to broadcast.
	/// @param msg Message to pass.
	static void broadcast(String const& signal, MessageHandleType const& msg = nullptr) {
		if (db.contains(signal))
			for (SignalWrapper& s: db[signal])
				s(msg);
	}

	/// @brief Broadcasts a message to a series of signals.
	/// @param signals Signals to broadcast.
	/// @param msg Message to pass.
	static void broadcast(StringList const& signals, MessageHandleType const& msg = nullptr) {
		for (String const& s: signals)
			broadcast(s, msg);
	}

	/// @brief Broadcasts a message to a series of signals.
	/// @param signals Signals to broadcast.
	/// @param msg Message to pass
	static void broadcast(StringArguments const& signals, MessageHandleType const& msg = nullptr) {
		for (String const& s: signals)
			broadcast(s, msg);
	}

	/// @brief Broadcasts a series of notifications.
	/// @param notifs Pairs of notifications and their messages.
	static void broadcast(List<KeyValuePair<String, MessageHandleType>> const& notifs) {
		for (auto& [sig, msg]: notifs)
			broadcast(sig, msg);
	}

	/// @brief Broadcasts a message to a signal.
	/// @param signals Signal to broadcast.
	/// @param msg Message to pass.
	/// @return Reference to self.
	NotificationServer& operator()(String const& signal, MessageHandleType const& msg = nullptr) const {
		broadcast(signal, msg);
		return *this;
	}

	/// @brief Broadcasts a message to a series of signals.
	/// @param signals Signals to broadcast.
	/// @param msg Message to pass
	/// @return Reference to self.
	NotificationServer& operator()(StringList const& signals, MessageHandleType const& msg = nullptr) const {
		broadcast(signals, msg);
		return *this;
	}

	/// @brief Broadcasts a message to a series of signals.
	/// @param signals Signals to broadcast.
	/// @param msg Message to pass
	/// @return Reference to self.
	NotificationServer& operator()(StringArguments const& signals, MessageHandleType const& msg = nullptr) const {
		broadcast(signals, msg);
		return *this;
	}

	/// @brief Broadcasts a series of notifications.
	/// @param notifs Pairs of notifications and their message.
	/// @return Reference to self.
	NotificationServer& operator()(List<KeyValuePair<String, MessageHandleType>> const& notifs) const {
		broadcast(notifs);
		return *this;
	}

private:
	/// @brief Notifications registered by this server.
	SignalDatabase added;

	/// @brief All registered notifications.
	inline static SignalDatabase db;
};

using Notifier = NotificationServer<0>;

/// @brief Specialized notification handler.
/// @tparam ...Args Message argument types.
/// @note Only allows one action to be registered per signal. If you need multiple actions, use `NotificationServer`.
template<typename... Args>
struct Notification {
	typedef Signal<Args...>	SignalType;

	/// @brief Empty constructor.
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
