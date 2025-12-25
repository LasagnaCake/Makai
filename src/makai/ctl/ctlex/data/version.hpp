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
					.eraseIf([] (auto& e) {return e.isNullOrSpaces();})
				;
				if (verv.size() > 0) ver.major		= toUInt64(verv[0]);
				if (verv.size() > 1) ver.minor		= toUInt64(verv[1]);
				if (verv.size() > 2) ver.patch		= toUInt64(verv[2]);
				if (verv.size() > 3) ver.hotfix		= toUInt64(verv[3]);
			}
			return ver;
		}

		constexpr Data::Value serialize() const {
			Data::Value ver;
			if (hotfix)
				ver = toString(major, ".", minor, ".", patch, ".", hotfix);
			else ver = toString(major, ".", minor, ".", patch);
			return ver;
		}
	};
}

CTL_EX_NAMESPACE_END

#endif