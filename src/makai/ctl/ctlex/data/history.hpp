#ifndef CTL_EX_DATA_HISTORY_H
#define CTL_EX_DATA_HISTORY_H

#include "../../ctl/container/lists/list.hpp"
#include "../../ctl/exnamespace.hpp"

CTL_EX_NAMESPACE_BEGIN

namespace Data {
	template <class T>
	struct History {
		using Stack = List<T>;

		constexpr History& undo() {
			
		}
	private:
		usize current = 0;
		Stack stack;
	};
}

CTL_EX_NAMESPACE_END

#endif