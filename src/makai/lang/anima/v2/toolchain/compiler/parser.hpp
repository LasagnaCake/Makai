#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_PARSER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_PARSER_H

#include "../assembler/assembler.hpp"
#include "../../core/core.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	using BaseContext = Assembler::BaseContext;

	struct Node: ID::Identifiable<Node const, ID::VLUID> {
		using Instance = Instance<Node>;

		enum class Content {
			AV2_TANC_EMPTY,
			AV2_TANC_VALUE,
			AV2_TANC_NAME,
			AV2_TANC_DECLARATION,
			AV2_TANC_ASSIGNMENT,
			AV2_TANC_FN_CALL,
			AV2_TANC_PREFIX_OP,
			AV2_TANC_INFIX_OP,
			AV2_TANC_POSTFIX_OP,
			AV2_TANC_INLINE_IF_ELSE,
			AV2_TANC_BLOCK,
			AV2_TANC_PATH,
			AV2_TANC_ARRAY,
			AV2_TANC_SUBSCRIPT,
			AV2_TANC_BRANCH,
			AV2_TANC_LOOP,
			AV2_TANC_IMPORT,
			AV2_TANC_TEMPLATE,
			AV2_TANC_INLINE_MINIMA
		};

		Content							content = Content::AV2_TANC_EMPTY;
		Data::Value						value;
		List<Instance>					children;
		Nullable<Core::DataLocation>	source;
		BaseContext::Axiom				base;
		List<BaseContext::Axiom>		interject;

		constexpr String name() const {
			auto const base = id();
			return toString("_u", base[0], "u", base[1], "u", base[2], "u", base[3]);
		}
	};

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
			AV2_TAPP_BIT_SHIFT,
			AV2_TAPP_ADD_SUB,
			AV2_TAPP_MUL_DIV_REM,
			AV2_TAPP_POW_ROOT,
			AV2_TAPP_ATAN2,
			AV2_TAPP_CROSS_FCROSS,
			AV2_TAPP_PREFIX,
			AV2_TAPP_CAST,
			AV2_TAPP_POSTFIX,
			AV2_TAPP_PATH,
		};

		BaseContext& context;

		virtual ~Parser();

		virtual Node::Instance nextExpression(Precedence prec = Precedence::AV2_TAPP_NONE);

		Node::Instance parse();

		using OperatorBank = Map<BaseContext::Axiom, Instance<AResolver>>;

		OperatorBank prefixes;
		OperatorBank infixes;

		static void add(BaseContext::Axiom const op, OperatorBank& bank, Instance<AResolver> const& resolver);
		static void add(BaseContext::Axiom::Type const op, OperatorBank& bank, Instance<AResolver> const& resolver);
		static void add(String const op, OperatorBank& bank, Instance<AResolver> const& resolver);

		void direct(BaseContext::Axiom::Type const op);

		void prefix(BaseContext::Axiom::Type const op);
		void prefix(String const&);

		void infix(BaseContext::Axiom::Type const op, bool const rightToLeft);
		void infix(String const& op, bool const rightToLeft);

		void postfix(BaseContext::Axiom::Type const op);
		void postfix(String const& op);

		template <class... Types> void direct(Types const&... args) requires (sizeof...(Types) > 1)		{(..., direct(args));	}
		template <class... Types> void prefix(Types const&... args) requires (sizeof...(Types) > 1)		{(..., prefix(args));	}
		template <class... Types> void postfix(Types const&... args) requires (sizeof...(Types) > 1)	{(..., postfix(args));	}

		Parser::Precedence currentPrecedence();

		Parser(BaseContext& context);
	};
}
#endif
