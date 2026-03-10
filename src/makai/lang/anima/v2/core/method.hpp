#ifndef MAKAILIB_ANIMA_V2_CORE_METHOD_H
#define MAKAILIB_ANIMA_V2_CORE_METHOD_H

#include "type.hpp"

namespace Makai::Anima::V2::Core {
	struct Method: Entry {
		Instance<Definition>		retType;
		List<Instance<Definition>>	argTypes;
		bool						out = false;

		struct Database {
			using Type = Instance<Method>;
			using StorageType = List<Type>;

			StorageType byName(String const& name) {
				StorageType fns;
				for (auto& type: types) {
					if (type->name == name)
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
