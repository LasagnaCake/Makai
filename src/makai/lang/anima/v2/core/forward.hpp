#ifndef MAKAILIB_ANIMA_V2_CORE_FORWARD_H
#define MAKAILIB_ANIMA_V2_CORE_FORWARD_H

#include "../../../../compat/ctl.hpp"

namespace Makai::Anima::V2::Core {
	struct Method;
	struct Definition;
	struct Value;
	struct Object;

	struct Symbol {
		Nullable<uint64>	source = null;
		uint64				id;
		String				name;
	};

	struct MethodTable {
		Map<uint64, uint64>	table;

		constexpr uint64 resolve(uint64 const sym) const {
			if (table.contains(sym))
				return table[sym];
			return sym;
		}
	};
}

#endif
