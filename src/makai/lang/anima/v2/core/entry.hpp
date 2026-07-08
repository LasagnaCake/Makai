#ifndef MAKAILIB_ANIMA_V2_CORE_ENTRY_H
#define MAKAILIB_ANIMA_V2_CORE_ENTRY_H

#include "forward.hpp"

namespace Makai::Anima::V2::Core {
	struct Entry {
		uint64	id		= 0;
		String	name;
		uint64	hash	= 0;
	};

	template <class T>
	requires (sizeof(T) == sizeof(uint64))
	struct Flagged {
		T		flags	= T();

		constexpr uint64 flagNumber() const {
			return bitcast<uint64>(flags);
		}
	};
}

#endif
