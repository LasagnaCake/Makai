#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_BREVE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_BREVE_H

#include "core.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct Breve: AAssembler {
		enum class Precedence {
			AV2_TANP_POSTFIX,
			AV2_TANP_POSTFIX_CAST,
			AV2_TANP_FN_CALL,
			AV2_TANP_SUBSCRIPT,
			AV2_TANP_MEMBER_ACCESS,
			AV2_TANP_PREFIX,
			AV2_TANP_UNARY_SIGN,
			AV2_TANP_NOT,
			AV2_TANP_SIZEOF,
			AV2_TANP_LENGTH,
			AV2_TANP_MUL_DIV_REM,
			AV2_TANP_ADD_SUB,
			AV2_TANP_BIT_SHIFT,
			AV2_TANP_BIT_THREEWAY,
			AV2_TANP_BIT_COMPARE,
			AV2_TANP_BIT_EQUALITY,
			AV2_TANP_BIT_BAND,
			AV2_TANP_BIT_BOR,
			AV2_TANP_BIT_BXOR,
			AV2_TANP_BIT_LAND,
			AV2_TANP_BIT_LOR,
			AV2_TANP_BIT_LXOR,
			AV2_TANP_ASSIGN,
			AV2_TANP_RIGHT_DECAY,
		};

		struct Context: BaseContext {};

		Breve(Context& context): AAssembler(context), context(context) {}
		void assemble() override;

		Context& context;
	};
}

#endif
