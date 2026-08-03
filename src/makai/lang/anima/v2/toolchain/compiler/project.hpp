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

		enum class Type {
			AV2_TCPT_LIBRARY,
			AV2_TCPT_WEB_PROGRAM,
			AV2_TCPT_BIN_PROGRAM,
			AV2_TCPT_EXECUTABLE,
		};

		enum class Language {
			AV2_TCPL_BREVE,
			AV2_TCPL_MINIMA,
		};

		Version				version		= {1};
		Version				art			= Core::Info::ART_VER;
		Version				concerto	= Core::Info::CONCERTO_VER;

		StringList			sources;
		Dictionary<Library>	libraries;
		File				main;

		Type				type;
		Language			language;

		Data::Value serialize() const;
		static Project deserialize(Data::Value const& v);
	};
}

#endif
