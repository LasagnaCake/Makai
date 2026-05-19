#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_PARSER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_PARSER_H

#include "../../assembler/assembler.hpp"
#include "../../../core/core.hpp"
#include "node.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Breve {
	struct AResolver;

	struct Parser {
		enum class Precedence: uint16 {
			AV2_TAPP_NONE,
			AV2_TAPP_DECL,
			AV2_TAPP_BLOCK,
			AV2_TAPP_RHS_DECAY,
			AV2_TAPP_NULL_DECAY,
			AV2_TAPP_ASSIGN,
			AV2_TAPP_INLINE_CONDITIONAL,
			AV2_TAPP_LOR,
			AV2_TAPP_LXOR,
			AV2_TAPP_LAND,
			AV2_TAPP_TYPE_CHECK,
			AV2_TAPP_BOR,
			AV2_TAPP_BXOR,
			AV2_TAPP_BAND,
			AV2_TAPP_EQ_INEQ,
			AV2_TAPP_COMPARE,
			AV2_TAPP_ORDER,
			AV2_TAPP_RANGE_EXPR,
			AV2_TAPP_BIT_SHIFT,
			AV2_TAPP_ADD_SUB,
			AV2_TAPP_MUL_DIV_REM,
			AV2_TAPP_POW_ROOT,
			AV2_TAPP_ATAN2,
			AV2_TAPP_CROSS_FCROSS,
			AV2_TAPP_PREFIX,
			AV2_TAPP_CAST,
			AV2_TAPP_PATH,
			AV2_TAPP_FN_CALL,
			AV2_TAPP_POSTFIX,
		};

		BaseContext& context;

		virtual ~Parser();

		virtual Node::Instance nextExpression(Precedence prec = Precedence::AV2_TAPP_NONE);

		Node::Instance parse();

		using NamedOperatorBank	= Map<UTF8String, Instance<AResolver>>;
		using TokenOperatorBank	= Map<BaseContext::Axiom::Type, Instance<AResolver>>;

		using OperatorBank = NamedOperatorBank;

		TokenOperatorBank directs;
		OperatorBank prefixes;
		OperatorBank infixes;

		static Precedence precedenceOf(BaseContext::Axiom const& tok);

		static void add(BaseContext::Axiom::Type const op, OperatorBank& bank, Instance<AResolver> const& resolver);
		static void add(UTF8String const op, OperatorBank& bank, Instance<AResolver> const& resolver);

		void direct(BaseContext::Axiom::Type const op);

		void prefix(BaseContext::Axiom::Type const op);
		void prefix(UTF8String const&);

		void infix(BaseContext::Axiom::Type const op, bool const rightToLeft);
		void infix(UTF8String const& op, bool const rightToLeft);

		void postfix(BaseContext::Axiom::Type const op);
		void postfix(UTF8String const& op);

		template <class... Types> void direct(Types const&... args) requires (sizeof...(Types) > 1)		{(..., direct(args));	}
		template <class... Types> void prefix(Types const&... args) requires (sizeof...(Types) > 1)		{(..., prefix(args));	}
		template <class... Types> void postfix(Types const&... args) requires (sizeof...(Types) > 1)	{(..., postfix(args));	}

		Parser::Precedence currentPrecedence();

		Parser(BaseContext& context);
	};
}
#endif
