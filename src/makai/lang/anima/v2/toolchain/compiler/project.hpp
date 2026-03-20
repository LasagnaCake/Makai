#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_PROJECT_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_PROJECT_H

#include "../assembler/assembler.hpp"
#include "../../core/core.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	struct Project;

	struct Project {
		using Version = Core::Module::Version;

		struct Library {
			String	source;
			Version	version;
		};

		struct File {
			String source;
			String path;
		};

		StringList			sources;
		Dictionary<Library>	libraries;
		File				main;

		Data::Value serialize() const;
		static Project deserialize(Data::Value const& v);
	};
}

#endif
