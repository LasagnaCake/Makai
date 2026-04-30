#ifndef MAKAILIB_ANIMA_V2_CORE_DATABASE_H
#define MAKAILIB_ANIMA_V2_CORE_DATABASE_H

#include "entry.hpp"

namespace Makai::Anima::V2::Core {
	template <Type::Derived<Entry> T>
	struct Database {
		using ElementType		= Instance<T>;
		using StorageType		= List<ElementType>;
		using QueryType			= Handle<T>;
		using QueryResultType	= List<QueryType>;

		StorageType byName(String const& name) const {
			StorageType out;
			for (auto& v: values) {
				if (v->name == name)
					out.pushBack(v);
			}
			return out;
		}

		StorageType byNameHash(uint64 const& hash) const {
			StorageType out;
			for (auto& v: values) {
				if (v->hash == hash)
					out.pushBack(v);
			}
			return out;
		}

		ElementType byID(uint64 const id) const {
			if (id < values.size())
				return values[id];
			return nullptr;
		}

		QueryResultType queryByName(String const& name) const {
			QueryResultType out;
			for (auto& v: values) {
				if (v->name == name)
					out.pushBack(v.asWeak());
			}
			return out;
		}

		QueryResultType queryByNameHash(uint64 const& hash) const {
			QueryResultType out;
			for (auto& v: values) {
				if (v->hash == hash)
					out.pushBack(v.asWeak());
			}
			return out;
		}

		QueryResultType queryByID(uint64 const id) const {
			if (id < values.size())
				return values[id].asWeak();
			return nullptr;
		}

		void addElement(ElementType const& elem) {
			values.pushBack(elem);
		}

		StorageType values;
	};
}

#endif
