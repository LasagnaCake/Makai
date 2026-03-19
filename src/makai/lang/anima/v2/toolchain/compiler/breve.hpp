#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_H

#include "core.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	struct Breve: ACompiler {
		struct Context: BaseContext {
			Intermediate inter;
			List<Namespace::Instance> stack;

			Namespace::Instance getNamespace(String const& name);
		};

		void invoke() override;
		Intermediate value() const override;

		Breve(Parser& parser, Context& context): ACompiler(parser, context), context(context) {}

	private:
		Context& context;
	};
}

#endif
