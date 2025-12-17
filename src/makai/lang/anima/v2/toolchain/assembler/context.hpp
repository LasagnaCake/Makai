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

			constexpr StringList map(Program& program) {
				StringList  stillUnmapped;
				for (auto const& [label, jumps]: unmapped)
					if (labels.contains(label))
						for (auto& jump: jumps)
							program.code[jump] = Cast::bit<Instruction>(labels[label]);
					else stillUnmapped.pushBack(label);
				unmapped.clear();
				return stillUnmapped;
			}
		};

		constexpr StringList mapJumps() {
			return jumps.map(program);
		}

		constexpr void addJumpTarget(String const& label) {
			if (jumps.labels.contains(label)) {
				program.code.pushBack(Makai::Cast::bit<Instruction>(jumps.labels[label]));
			} else {
				jumps.unmapped[label].pushBack(program.code.size());
				program.code.pushBack({});
			}
		}

		[[nodiscard]]
		constexpr usize addEmptyInstruction() {
			program.code.pushBack({});
			return program.code.size() - 1;
		}

		template <class T>
		constexpr usize addInstruction(T const& inst)
		requires (sizeof(T) == sizeof(Instruction)) {
			program.code.pushBack(Cast::bit<Instruction, T>(inst));
			return program.code.size() - 1;
		}

		template <class T>
		constexpr static void addInstructionType(Instruction& inst, T const& type)
		requires (sizeof(T) == sizeof(uint32)) {
			inst.type = Cast::bit<uint32, T>(type);
		}

		constexpr Instruction& instruction(usize const i) {
			return program.code[i];
		}

		Jumps		jumps;
		TokenStream	stream;
		Program		program;
		String		fileName;
	};
}

#endif