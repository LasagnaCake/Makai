#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_CORE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_CORE_H

#include "../../runtime/runtime.hpp"
#include "../assembler/assembler.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	struct Project {
		enum class Type {
			AV2_TC_PT_EXECUTABLE,
			AV2_TC_PT_PROGRAM,
			AV2_TC_PT_MODULE,
		};

		struct File {
			enum class Type {
				AV2_TC_PFT_MINIMA,
				AV2_TC_PFT_BREVE,
			};
			Type	type	= Type::AV2_TC_PFT_BREVE;
			String	path;
			String	source;
		};

		Type		type		= Type::AV2_TC_PT_EXECUTABLE;
		File		main;
		StringList	sources;
		StringList	modules;

		static Project deserialize(Data::Value const& value) {
			if (!value.contains("version")) return deserializeV2(value);
			auto const ver = value["version"].get<String>("2.0.0").split('.').front();
			switch (toUInt64(ver)) {
				case 2: return deserializeV2(value);
				default: throw Error::InvalidValue("Unsupported project version!");
			}
		}

		static Project deserializeV2(Data::Value const& value);
	};
	
	template<Type::Derived<Assembler::AAssembler> TAsm = Assembler::Breve>
	inline void build(Assembler::Context& context, Makai::String const& source) {
		context.stream.open(source);
		TAsm assembler(context);
		assembler.assemble();
	}

	Runtime::Program buildProject(Project const& proj);
}

#endif