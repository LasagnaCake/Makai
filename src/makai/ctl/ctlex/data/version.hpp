#ifndef CTL_EX_DATA_VERSION_H
#define CTL_EX_DATA_VERSION_H

#include "value.hpp"

CTL_EX_NAMESPACE_BEGIN

namespace Data {
	struct Version {
		uint64 major	= 0;
		uint64 minor	= 0;
		uint64 patch	= 0;
		uint64 hotfix	= 0;

		constexpr static Version deserialize(Data::Value const& value) {
			Version ver;
			if (value.isString()) {
				StringList verv = 
					value
					.get<String>()
					.eraseIf(isNullOrSpaceChar<char>)
					.split('.')
					.filter([] (auto& e) {return !e.isNullOrSpaces();})
				;
				if (verv.size() > 0) ver.major		= toUInt64(verv[0], 10);
				if (verv.size() > 1) ver.minor		= toUInt64(verv[1], 10);
				if (verv.size() > 2) ver.patch		= toUInt64(verv[2], 10);
				if (verv.size() > 3) ver.hotfix		= toUInt64(verv[3], 10);
			}
			return ver;
		}

		constexpr Data::Value serialize() const {
			// Ugly, but works :/
			if (hotfix)
				return toString(major, ".", minor, ".", patch, ".", hotfix);
			else if (patch)
				return toString(major, ".", minor, ".", patch);
			else if (minor)
				return toString(major, ".", minor);
			else
				return toString(major);
		}
	};
}

CTL_EX_NAMESPACE_END

#endif