#ifndef MAKAILIB_ANIMA_V2_RUNTIME_MODEL_CORE_H
#define MAKAILIB_ANIMA_V2_RUNTIME_MODEL_CORE_H

#include "../../../../../compat/ctl.hpp"

namespace Makai::Anima::V2::Runtime {
	struct TypeModel;
	struct Value;
	struct Object;
	struct ALibrary;

	// While `AtomicCell` is not working, this will do
	using Storage = Instance<Object, Object>;

	struct MethodTable {
		Map<uint64, uint64>	table;

		constexpr uint64 resolve(uint64 const sym) const {
			if (table.contains(sym))
				return table[sym];
			return sym;
		}
	};

	struct Void {};

	struct Any {
		Storage value;

		constexpr Storage const& operator->() const {
			return value;
		}
	};

	struct Identifier: Ordered {
		uint64 id;

		constexpr OrderType operator<=>(Identifier const& other) const {
			return id <=> other.id;
		}

		constexpr bool operator==(Identifier const& other) const {
			return id == other.id;
		}
	};

	struct TypeID:	Identifier {};
	struct JumpID:	Identifier {};
	struct CallID:	Identifier {};
}

#endif
