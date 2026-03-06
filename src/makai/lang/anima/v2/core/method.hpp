#ifndef MAKAILIB_ANIMA_V2_CORE_METHOD_H
#define MAKAILIB_ANIMA_V2_CORE_METHOD_H

#include "type.hpp"

namespace Makai::Anima::V2::Core {
	struct Method {
		String						name;
		Instance<Definition>		retType;
		List<Instance<Definition>>	argTypes;
		bool						out = false;

		struct Database {
			using Type = Instance<Definition>;
			using StorageType = List<Type>;

			StorageType byAlias(String const& alias) {
				StorageType fns;
				for (auto& type: types) {
					if (type->name == alias)
						fns.pushBack(type);
				}
				return fns;
			};

			Type byID(uint64 const id) {
				if (id < types.size())
					return types[id];
				return nullptr;
			};

			StorageType types;
		};
	};
}

#endif
