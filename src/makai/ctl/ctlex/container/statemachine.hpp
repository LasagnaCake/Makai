#ifndef CTL_EX_CONTAINER_STATEMACHINE_H
#define CTL_EX_CONTAINER_STATEMACHINE_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/typetraits/traits.hpp"
#include "../../ctl/templates.hpp"
#include "../../ctl/container/map.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Simple state machine.
/// @tparam TState State type.
template <class TState>
struct StateMachine {
	/// @brief State type.
	using StateType 	= TState;
	/// @brief State map type.
	using StateMap		= Map<StateType, usize>;
	/// @brief State graph type.
	using StateGraph	= Map<StateType, StateMap>;

	/// @brief Advances the state machine forward to its next state. If state does not exists, returns the current state.
	/// @param priority Priority of the path to take. By default, it is the highest possible priority (`Limit::MAX<usize>`).
	/// @return Next state.
	/// @note Will always take the path with the priority that is closest (BUT not greater than) the given priority.
	constexpr StateType advance(usize const priority = Limit::MAX<usize>) {
		if (forward.contains(current)) {
			for (auto const& [state, id]: forward[current]) {
				if (id > priority) break;
				else current = state;
			}
		}
		return current;
	}

	/// @brief Retreats the state machine forward to its previous state. If state does not exists, returns the current state.
	/// @param priority Priority of the path to take. By default, it is the highest possible priority (`Limit::MAX<usize>`).
	/// @return Previous state.
	/// @note Will always take the path with the priority that is closest (BUT not greater than) the given priority.
	constexpr StateType retreat(usize const priority = Limit::MAX<usize>) {
		if (reverse.contains(current)) {
			for (auto const& [state, id]: reverse[current]) {
				if (id > priority) break;
				else current = state;
			}
		}
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
		for (auto const& [current, next]: states)
			for (auto const& [state, priority]: next)
				reverse[state][current] = priority;
		return *this;
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
	/// @brief Forward state graph.
	StateGraph	forward;
	/// @brief Reverse state graph.
	StateGraph	reverse;
};

CTL_EX_NAMESPACE_END

#endif