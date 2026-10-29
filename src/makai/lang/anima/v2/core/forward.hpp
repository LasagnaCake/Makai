#ifndef MAKAILIB_ANIMA_V2_CORE_FORWARD_H
#define MAKAILIB_ANIMA_V2_CORE_FORWARD_H

#include "../../../../compat/ctl.hpp"

namespace Makai::Anima::V2::Core {
	struct Symbol {
		Nullable<uint64>	source = null;
		uint64				id;

		constexpr Data::Value serialize() const {
			auto def = Data::Value::object();
			if (source) def["src"] = *source;
			def["id"] = id;
			return def;
		}

		constexpr static Symbol deserialize(Data::Value const& v) {
			Symbol sym;
			if (v.contains("src"))
				sym.source = v["src"].getUnsigned();
			sym.id = v["id"].getUnsigned();
			return sym;
		}
	};
}

#endif
