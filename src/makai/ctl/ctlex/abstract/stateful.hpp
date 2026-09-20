#ifndef CTL_EX_ABSTRACT_STATEFUL_H
#define CTL_EX_ABSTRACT_STATEFUL_H

#include "../../ctl/exnamespace.hpp"

CTL_EX_NAMESPACE_BEGIN;

template <Type::Enumerator T, class TResult = void>
struct AStateful {
	virtual ~AStateful() {}

	using State		= T;
	using Result	= TResult;

	constexpr Result process() {return onState(current);}

	constexpr State state() const {
		return current;
	}

	constexpr AStateful& next(State const next) const {
		current = next;
		return *this;
	}

protected:
	constexpr virtual Result onState(State const state) = 0;

private:
	State current;
};

CTL_EX_NAMESPACE_END

#endif
