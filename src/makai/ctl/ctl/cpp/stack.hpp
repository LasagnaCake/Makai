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
			result += "at Function:[" + String(trace.frames[i].info) + "]";
			if (trace.frames[i].line != -1) {
				result += " in Line:[" + toString(trace.frames[i].line) + "]";
				result += " (in File:[" + String(trace.frames[i].file) + "])";
			} else result += " -> at Address:[" +String(trace.frames[i].address) + "]";
			result += "\n";
		}
		return result;
	}
}

CTL_NAMESPACE_END

#endif