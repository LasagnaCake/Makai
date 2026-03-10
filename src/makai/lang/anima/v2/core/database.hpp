#ifndef MAKAILIB_ANIMA_V2_CORE_DATABASE_H
#define MAKAILIB_ANIMA_V2_CORE_DATABASE_H

#include "type.hpp"

namespace Makai::Anima::V2::Core {
	template <class T>
	struct Database {
		using ElementType = Instance<T>;
		using StorageType = List<T>;

		StorageType byName(String const& name) const {
			StorageType out;
			for (auto& v: values) {
				if (v->name == name)
					out.pushBack(v);
			}
			return out;
		};

		ElementType byID(uint64 const id) const {
			if (id < values.size())
				return values[id];
			return nullptr;
		};

		StorageType values;
	};
}

#endif
