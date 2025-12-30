#ifndef CTL_CPP_STACK_H
#define CTL_CPP_STACK_H

#include "stacktrace.hpp"
#include "../container/strings/string.hpp"
#include "../algorithm/strconv.hpp"

CTL_NAMESPACE_BEGIN

namespace CPP::Stack {
	template <usize S>
	inline String format(Trace<S> const& trace) {
		if (!(trace.frames && trace.count)) return "";
		String result;
		for (usize i = 0; i < trace.count; ++i) {
			result += "at " + trace.frames[i].info;
			if (trace.frames[i].line != -1) {
				result += ": line " + toString(trace.frames[i].line);
				result += "(in " + trace.frames[i].file + ")";
			} else result += ": address " + String::fromNumber(trace.frames[i].address, 16);
			result += "\n";
		}
		return result;
	}
}

CTL_NAMESPACE_END

#endif