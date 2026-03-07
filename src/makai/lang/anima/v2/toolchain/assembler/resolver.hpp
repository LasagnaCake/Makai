#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_RESOLVER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_RESOLVER_H

#include "context.hpp"
#include "core.hpp"
#include "../../core/instruction.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct ITypeResolver {
		struct Result {
			uint64 typeID;
		};

		virtual Result resolve(BaseContext& context) = 0;
	};

	struct IValueResolver {
		struct Result {
			uint64				typeID;
			String				action;
			Core::DataLocation	source;
			String				value;
		};

		virtual Result resolve(BaseContext& context) = 0;
	};

	struct PathResolver: IValueResolver {
		Result resolve(BaseContext& context) override;
	};

	struct ExpressionResolver: IValueResolver {
		Result resolve(BaseContext& context) override;
	};

	struct BinaryResolver: IValueResolver {
		Result resolve(BaseContext& context) override;
	};
}

#endif
