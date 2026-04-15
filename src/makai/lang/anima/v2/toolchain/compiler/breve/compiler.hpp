#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_COMPILER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_COMPILER_H

#include "../core.hpp"
#include "parser.hpp"
#include "intermediate.hpp"
#include "node.hpp"
#include "transformer.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Breve {
	struct Compiler: ACompiler, IValue<Core::Module> {
		struct Context: BaseContext, Transformer::ATransformer::Context {
		};

		void invoke() override;

		Core::Module value() const override;

		Compiler(Parser& parser, Context& context): ACompiler(context), context(context), parser(parser) {}

	private:
		Context& context;
		Parser& parser;

		Core::Module result;
	};
}

#endif
