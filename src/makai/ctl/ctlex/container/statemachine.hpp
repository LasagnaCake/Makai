#ifndef CTL_EX_CONTAINER_STATEMACHINE_H
#define CTL_EX_CONTAINER_STATEMACHINE_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/typetraits/traits.hpp"
#include "../../ctl/templates.hpp"
#include "../../ctl/container/map.hpp"
#include "../../ctl/container/nullable.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Simple state machine.
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

	/// @brief
	///		Advances the state machine forward to its next state.
	///		If state does not contain any path,
	///		or all possible paths are of greater priority than requested,
	///		returns the current state.
	/// @param priority Priority of the path to take. By default, it is the highest possible priority (`Limit::MAX<usize>`).
	/// @return Next state.
	/// @note Will always take the path with the priority that is closest (BUT not greater than) the given priority.
	constexpr StateType advance(usize const priority = Limit::MAX<usize>) {
		if (forward.contains(current)) {
			for (auto const& [state, id]: forward[current]) {
				if (!id || id.value() > priority) continue;
				current = state;
			}
		}
		return current;
	}

	/// @brief
	///		Retreats the state machine forward to its previous state.
	///		If state does not contain any path,
	///		or all possible paths are of greater priority than requested,
	///		returns the current state.
	/// @param priority Priority of the path to take. By default, it is the highest possible priority (`Limit::MAX<usize>`).
	/// @return Previous state.
	/// @note Will always take the path with the priority that is closest (BUT not greater than) the given priority.
	constexpr StateType retreat(usize const priority = Limit::MAX<usize>) {
		if (reverse.contains(current)) {
			for (auto const& [state, id]: reverse[current]) {
				if (!id || id.value() > priority) continue;
				current = state;
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
	/// @param priority Priority of path.
	/// @return Reference to self.
	constexpr StateMachine& setPath(StateType const& from, StateType const& to, PriorityType const priority = 0ull) {
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
	/// @brief Forward state graph.
	StateGraph	forward;
	/// @brief Reverse state graph.
	StateGraph	reverse;
};

CTL_EX_NAMESPACE_END

#endif