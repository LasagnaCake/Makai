#ifndef CTL_EX_CONTAINER_STATEMACHINE_H
#define CTL_EX_CONTAINER_STATEMACHINE_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/typetraits/traits.hpp"
#include "../../ctl/templates.hpp"
#include "../../ctl/container/map/map.hpp"
#include "../../ctl/container/nullable.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Simple state machine with priority pathing.
/// @tparam TState State type.
template <class TState>
struct StateMachine {
	/// @brief State type.
	using StateType 	= TState;
	/// @brief Priority type.
	using PriorityType	= Nullable<usize>;
	/// @brief State map type.
	using StateMap		= Map<StateType, PriorityType>;
	/// @brief State graph type.
	using StateGraph	= Map<StateType, StateMap>;

	/// @brief Priority selection behaviour.
	/// @note What happens on failure depends on the function that uses it.
	enum class Behaviour {
		/// @brief
		///		Path with a priority closest to (BUT not less than) the requested priority.
		///		Fails if no paths with priority greater than or equal to requested priority exists.
		SMB_CLOSEST_MATCH,
		/// @brief
		///		First path found which has the EXACT requested priority.
		///		Fails if no paths with the requested priority exists.
		SMB_FIRST_MATCH,
		/// @brief
		///		Last path found which has the EXACT requested priority.
		///		Fails if no paths with the requested priority exists.
		SMB_LAST_MATCH,
		/// @brief
		///		First path found with a priority equal to or higher than the requested priority.
		///		Fails if no paths with priority greater than or equal to requested priority exists.
		SMB_FIRST_PRECEDENCE,
		/// @brief
		///		Last path found with a priority equal to or higher than the requested priority.
		///		Fails if no paths with priority greater than or equal to requested priority exists.
		SMB_LAST_PRECEDENCE
	};

	/// @brief Default behaviour for path selection.
	constexpr static Behaviour DEFAULT_BEHAVIOUR = Behaviour::SMB_FIRST_MATCH;

	/// @brief Returns whether the state machine is empty.
	/// @return Whether state machine is empty.
	constexpr bool empty() const {
		return forward.empty() || reverse.empty();
	}

	/// @brief
	///		Advances the state machine forward to its next state.
	///		If state does not contain any path,
	///		or a path could not be found with the given behaviour (failure),
	///		returns the current state.
	/// @param priority
	///		Priority of the path to take.
	///		By default, it is the lowest possible priority (`0`).
	/// @param behaviour Behaviour to use for choosing the next path. By default, it is `DEFAULT_BEHAVIOUR`.
	/// @return Next state.
	constexpr StateType advance(usize const priority = 0, Behaviour const behaviour = DEFAULT_BEHAVIOUR) {
		if (forward.contains(current))
			current = getStateByBehaviour(forward[current], priority, current, behaviour);
		return current;
	}

	/// @brief
	///		Retreats the state machine forward to its previous state.
	///		If state does not contain any path,
	///		or a path could not be found with the given behaviour (failure),
	///		returns the current state.
	/// @param priority
	///		Priority of the path to take.
	///		By default, it is the lowest possible priority (`0`).
	/// @param priority
	/// @param behaviour Behaviour to use for choosing the next path. By default, it is `DEFAULT_BEHAVIOUR`.
	/// @return Previous state.
	constexpr StateType retreat(usize const priority = 0, Behaviour const behaviour = DEFAULT_BEHAVIOUR) {
		if (reverse.contains(current))
			current = getStateByBehaviour(reverse[current], priority, current, behaviour);
		return current;
	}

	/// @brief Returns the state map for a given state.
	/// @return State map for state.
	constexpr StateMap getStateMap(StateType const& state) const {
		return forward[state];
	}

	/// @brief Sets the state graph.
	/// @param states State graph to use.
	/// @return Reference to self.
	constexpr StateMachine& setStates(StateGraph const& states) {
		forward = states;
		reverse.clear();
		for (auto const& [current, next]: states)
			for (auto const& [state, priority]: next)
				reverse[state][current] = priority;
		return *this;
	}

	/// @brief Adds a set of states the state graph.
	/// @param states State graph to add.
	/// @return Reference to self.
	constexpr StateMachine& addStates(StateGraph const& states) {
		for (auto const& [current, next]: states)
			for (auto const& [state, priority]: next) {
				reverse[current][state] = priority;
				reverse[state][current] = priority;
			}
		return *this;
	}

	/// @brief Clears all states in the graph.
	/// @return Reference to self.
	constexpr StateMachine& clearStates() {
		forward.clear();
		reverse.clear();
		return *this;
	}

	/// @brief Adds a state to the graph.
	/// @param state State to add.
	/// @param paths Paths of state to add.
	/// @return Reference to self.
	constexpr StateMachine& addState(StateType const& state, StateMap const& paths) {
		forward[state] = paths;
		for (auto const& [path, priority]: paths)
			reverse[path][state] = priority;
		return *this;
	}

	/// @return Creates/Modifies a path between two states.
	/// @param from State to part from.
	/// @param to State to end up in.
	/// @param priority Priority of path. By default, it is the lowest possible priority (`0`).
	/// @return Reference to self.
	constexpr StateMachine& setPath(StateType const& from, StateType const& to, PriorityType const priority = Cast::as<usize>(0)) {
		forward[from][to] = priority;
		reverse[to][from] = priority;
		return *this;
	}

	/// @return Removes a path between two states.
	/// @param from State to part from.
	/// @param to State to end up in.
	/// @return Reference to self.
	constexpr StateMachine& removePath(StateType const& from, StateType const& to) {
		return setPath(from, to, nullptr);
	}

	/// @brief Returns the current state graph.
	/// @param revers Whether to return the reverse of the current graph. By default, it is false.
	/// @return Current state graph.
	constexpr StateGraph getStates(bool const reversed = false) const {
		return reversed ? reverse : forward;
	}

	/// @brief Current state.
	StateType	current;

private:
	constexpr static StateType getStateByBehaviour(
		StateMap const& map,
		usize const& priority,
		StateType const& startState,
		Behaviour const behaviour
	) {
		switch (behaviour) {
			case Behaviour::SMB_CLOSEST_MATCH:		return closestMatch(map, priority, startState);		break;
			case Behaviour::SMB_FIRST_MATCH:		return firstMatch(map, priority, startState);		break;
			case Behaviour::SMB_LAST_MATCH:			return lastMatch(map, priority, startState);		break;
			case Behaviour::SMB_FIRST_PRECEDENCE:	return firstPrecedence(map, priority, startState);	break;
			case Behaviour::SMB_LAST_PRECEDENCE:	return lastPrecedence(map, priority, startState);	break;
		}
		return startState;
	}

	constexpr static StateType closestMatch(StateMap const& map, usize const& priority, StateType const& startState) {
		StateType currentState = startState;
		usize currentPriority	= Limit::MAX<usize>;
		for (auto const& [state, id]: map) {
			if (id && id.value() == currentPriority) {
				currentState = state;
			}
		}
		return currentState;
	}

	constexpr static StateType firstMatch(StateMap const& map, usize const& priority, StateType const& startState) {
		StateType currentState = startState;
		for (auto const& [state, id]: map)
			if (id && id.value() == priority) {
				currentState = state;
				break;
			}
		return currentState;
	}

	constexpr static StateType lastMatch(StateMap const& map, usize const& priority, StateType const& startState) {
		StateType currentState = startState;
		for (auto const& [state, id]: map)
			if (id && id.value() == priority)
				currentState = state;
		return currentState;
	}

	constexpr static StateType firstPrecedence(StateMap const& map, usize const& priority, StateType const& startState) {
		StateType currentState = startState;
		for (auto const& [state, id]: map) {
			if (id && id.value() >= priority) {
				currentState = state;
				break;
			}
		}
		return currentState;
	}

	constexpr static StateType lastPrecedence(StateMap const& map, usize const& priority, StateType const& startState) {
		StateType currentState = startState;
		for (auto const& [state, id]: map)
			if (id && id.value() >= priority)
				currentState = state;
		return currentState;
	}

	/// @brief Forward state graph.
	StateGraph	forward;
	/// @brief Reverse state graph.
	StateGraph	reverse;
};

CTL_EX_NAMESPACE_END

#endif
