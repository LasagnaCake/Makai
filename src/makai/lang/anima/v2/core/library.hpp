#ifndef MAKAILIB_ANIMA_V2_CORE_LIBRARY_H
#define MAKAILIB_ANIMA_V2_CORE_LIBRARY_H

#include "module.hpp"


namespace Makai::Anima::V2::Core {
	struct Library {
		using Version = Module::Version;
		String							name;
		Dictionary<String>				modules;
		Dictionary<Module::Declaration>	types;
		Dictionary<Module::Method>		methods;
	};
}

#endif
