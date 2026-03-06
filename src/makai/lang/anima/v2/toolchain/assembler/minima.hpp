#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_MINIMA_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_MINIMA_H

#include "core.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct Minima: AAssembler {
		struct Context: BaseContext {
			struct Minima {
				Program program;

				struct Method: ID::Identifiable<Method const, uint64> {
					uint64			retType;
					List<uint64>	argTypes;
					bool			out		= false;
					bool			local	= false;
					String			entry;
				};

				struct Type: ID::Identifiable<Type const, uint64> {
					uint64						flags		= 0;
					Nullable<Core::BasicType>	basic		= Core::BasicType::AV2_BT_VOID;
					Nullable<uint64>			base		= null;
					uint64						alignment	= 1;
					List<uint64>				fields;
					List<uint64>				operators;
				};

				bool							canImport	= false;
				Nullable<String>				parentModule;
				Nullable<String>				module;
				Dictionary<Instance<Type>>		types;
				Dictionary<Instance<Method>>	methods;
			} minima;
		};

		using AAssembler::AAssembler;
		void assemble() override;
	};
}

#endif
