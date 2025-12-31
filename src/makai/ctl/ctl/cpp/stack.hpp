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
			result += "at " + toString(trace.frames[i].info);
			if (trace.frames[i].line != -1) {
				result += ": line " + toString(trace.frames[i].line);
				result += "(in " + toString(trace.frames[i].file) + ")";
			} else result += ": address[" +toString(trace.frames[i].address) + "]";
			result += "\n";
		}
		return result;
	}
}

CTL_NAMESPACE_END

#endif