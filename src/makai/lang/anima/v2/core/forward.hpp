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
		UTF8String			name;

		constexpr Data::Value serialize() const {
			auto def = Data::Value::object();
			if (source) def["src"] = *source;
			def["id"] = id;
			def["name"] = name.toString();
			return def;
		}

		constexpr static Symbol deserialize(Data::Value const& v) {
			Symbol sym;
			if (v.contains("src"))
				sym.source = v["src"].getUnsigned();
			sym.id = v["id"].getUnsigned();
			sym.name = v["name"].getString();
			return sym;
		}
	};

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
		Instance<Object> value;

		constexpr Instance<Object> const& operator->() const {
			return value;
		}
	};

	struct TypeID: Ordered {
		uint64 id;

		constexpr OrderType operator<=>(TypeID const& other) const {
			return id <=> other.id;
		}

		constexpr bool operator==(TypeID const& other) const {
			return id == other.id;
		}
	};
}

#endif
