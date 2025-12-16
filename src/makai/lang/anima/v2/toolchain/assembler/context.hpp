#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_CONTEXT_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_CONTEXT_H

#include "../../../../../lexer/lexer.hpp"
#include "../../runtime/program.hpp"
#include "../../instruction.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct Context {
		using TokenStream	= Lexer::CStyle::TokenStream;
		using Program		= Runtime::Program;

		struct Jumps {
			Dictionary<uint64>			labels;
			Dictionary<List<uint64>>	unmapped;

			constexpr void map(Program& program) {
				for (auto const& [label, jumps]: unmapped)
					for (auto& jump: jumps)
						program.code[jump] = Cast::bit<Instruction>(labels[label]);
				unmapped.clear();
			}
		};

		constexpr void mapJumps() {
			jumps.map(program);
		}

		constexpr void addJumpTarget(String const& label) {
			if (jumps.labels.contains(label)) {
				program.code.pushBack(Makai::Cast::bit<Instruction>(jumps.labels[label]));
			} else {
				jumps.unmapped[label].pushBack(program.code.size());
				program.code.pushBack({});
			}
		}

		Jumps		jumps;
		TokenStream	stream;
		Program		program;
		String		fileName;
	};
}

#endif