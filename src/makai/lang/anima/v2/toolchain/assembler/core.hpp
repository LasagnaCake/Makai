#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_CORE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_CORE_H

#include "context.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct AAssembler: IInvokable<void> {
		using TokenStream	= Lexer::CStyle::TokenStream;
		using TokenList		= Lexer::CStyle::TokenStream::TokenList;
		using Program		= Runtime::Program;

		virtual  ~AAssembler() {}

		AAssembler(BaseContext& ctx): context(ctx) {}

		BaseContext& context;
	};
}

#endif
