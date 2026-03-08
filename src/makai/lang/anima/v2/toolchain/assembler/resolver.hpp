#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_RESOLVER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_RESOLVER_H

#include "context.hpp"
#include "core.hpp"
#include "../../core/instruction.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct Node: ID::Identifiable<Node const, uint64> {
		using Instance = Instance<Node>;

		enum class Type {
			AV2_TANT_CONSTANT,
			AV2_TANT_BIN_OP,
			AV2_TANT_UN_OP,
			AV2_TANT_INLINE_IF_ELSE,
			AV2_TANT_FN_CALL,
			AV2_TANT_VAR_DECL,
			AV2_TANT_FN_DECL,
			AV2_TANT_PATH_RESOLVE,
		};

		Type				type;
		List<Instance>		children;
		Core::DataLocation	source;
		BaseContext::Axiom	base;
		bool				postfix = false;
	};

	struct AResolver;

	struct Parser {
		enum class Precedence {
			AV2_TAPP_NONE,
			AV2_TAPP_RHS_DECAY = 10,
			AV2_TAPP_ASSIGN = 20,
			AV2_TAPP_NULL_DECAY = 30,
			AV2_TAPP_CONDITIONAL = 40,
			AV2_TAPP_LOR = 50,
			AV2_TAPP_LXOR = 60,
			AV2_TAPP_LAND = 70,
			AV2_TAPP_BOR = 80,
			AV2_TAPP_BXOR = 90,
			AV2_TAPP_BAND = 100,
			AV2_TAPP_EQ_INEQ = 120,
			AV2_TAPP_COMPARE = 130,
			AV2_TAPP_ORDER = 140,
			AV2_TAPP_BIT_SHIFT = 150,
			AV2_TAPP_ADD_SUB = 160,
			AV2_TAPP_MUL_DIV_REM = 170,
			AV2_TAPP_PREFIX = 180,
			AV2_TAPP_POSTFIX = 190,
		};

		BaseContext& context;

		virtual ~Parser();

		virtual Node::Instance nextExpression(Precedence prec = Precedence::AV2_TAPP_NONE);

		Node::Instance parse();

		using OperatorBank = Map<BaseContext::Axiom, Instance<AResolver>>;

		OperatorBank prefixes;
		OperatorBank infixes;

		static void add(BaseContext::Axiom const op, OperatorBank& bank, Instance<AResolver> const& resolver);

		void direct(BaseContext::Axiom::Type const op);

		void prefix(BaseContext::Axiom::Type const op);
		void prefix(String const&);

		void infix(BaseContext::Axiom::Type const op, bool const rightwards);
		void infix(String const& op, bool const rightwards);

		void postfix(BaseContext::Axiom::Type const op);
		void postfix(String const& op);

		template <class... Types> void direct(Types const&... args) requires (sizeof...(Types) > 1)		{(..., nofix(args));	}
		template <class... Types> void prefix(Types const&... args) requires (sizeof...(Types) > 1)		{(..., prefix(args));	}
		template <class... Types> void postfix(Types const&... args) requires (sizeof...(Types) > 1)	{(..., postfix(args));	}

		Parser::Precedence currentPrecedence();

		Parser(BaseContext& context);
	};

	struct AResolver {
		virtual ~AResolver();

		Parser::Precedence const	precedence;
		bool const					rightwards;

		AResolver(Parser::Precedence const precedence, bool const rightwards);

		virtual Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) = 0;
	};

	struct DirectResolver: AResolver {
		using AResolver::AResolver;
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct PrefixResolver: AResolver {
		using AResolver::AResolver;
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct InfixResolver: AResolver {
		using AResolver::AResolver;
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct PostfixResolver: AResolver {
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct FunctionCallResolver: AResolver {
		using AResolver::AResolver;
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct BlockResolver: AResolver {
		using AResolver::AResolver;
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct DeclarationResolver: AResolver {
		using AResolver::AResolver;
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct InlineIfElseResolver: AResolver {
		using AResolver::AResolver;
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct ExpressionResolver: AResolver {
		using AResolver::AResolver;
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};
}

#endif
