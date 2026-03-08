#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_RESOLVER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_ASSEMBLER_RESOLVER_H

#include "context.hpp"
#include "core.hpp"
#include "../../core/instruction.hpp"

namespace Makai::Anima::V2::Toolchain::Assembler {
	struct Decl;

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
			AV2_TANC_LOOP
		};

		Content					content = Content::AV2_TANC_EMPTY;
		Data::Value				value;
		List<Instance>			children;
		Makai::Instance<Decl>	type;
		Core::DataLocation		source;
		BaseContext::Axiom		base;

		constexpr String name() const {
			auto const base = id();
			return toString("_u", base[0], "u", base[1], "u", base[2], "u", base[3]);
		}
	};

	struct Decl {
	};

	struct AResolver;

	struct Parser {
		enum class Precedence: uint16 {
			AV2_TAPP_NONE,
			AV2_TAPP_DECL,
			AV2_TAPP_BLOCK,
			AV2_TAPP_BRANCHES_AND_LOOPS,
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

		template <class... Types> void direct(Types const&... args) requires (sizeof...(Types) > 1)		{(..., nofix(args));	}
		template <class... Types> void prefix(Types const&... args) requires (sizeof...(Types) > 1)		{(..., prefix(args));	}
		template <class... Types> void postfix(Types const&... args) requires (sizeof...(Types) > 1)	{(..., postfix(args));	}

		Parser::Precedence currentPrecedence();

		Parser(BaseContext& context);
	};

	struct AResolver {
		virtual ~AResolver();

		AResolver(Parser::Precedence const precedence = Parser::Precedence::AV2_TAPP_NONE, bool const rightToLeft = false);

		virtual Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) = 0;

		Parser::Precedence const	precedence;
		bool const					rightToLeft;
	};

	struct DirectResolver: AResolver {
		DirectResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct PrefixResolver: AResolver {
		PrefixResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct InfixResolver: AResolver {
		using AResolver::AResolver;
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct PostfixResolver: AResolver {
		using AResolver::AResolver;
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct PathResolver: AResolver {
		PathResolver(): AResolver(Parser::Precedence::AV2_TAPP_PATH, false) {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct FunctionCallResolver: AResolver {
		FunctionCallResolver(): AResolver(Parser::Precedence::AV2_TAPP_POSTFIX, false) {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct BlockResolver: AResolver {
		BlockResolver(): AResolver(Parser::Precedence::AV2_TAPP_BLOCK, false) {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct SubExpressionResolver: AResolver {
		SubExpressionResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct BranchResolver: AResolver {
		BranchResolver(): AResolver(Parser::Precedence::AV2_TAPP_BRANCHES_AND_LOOPS, false) {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct LoopResolver: AResolver {
		LoopResolver(): AResolver(Parser::Precedence::AV2_TAPP_BRANCHES_AND_LOOPS, false) {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct InlineIfElseResolver: AResolver {
		InlineIfElseResolver(): AResolver(Parser::Precedence::AV2_TAPP_INLINE_CONDITIONAL, false) {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct DeclarationResolver: AResolver {
		DeclarationResolver(): AResolver(Parser::Precedence::AV2_TAPP_DECL, false) {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct AssignmentResolver: AResolver {
		AssignmentResolver(): AResolver(Parser::Precedence::AV2_TAPP_ASSIGN, true) {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct FunctionPrototypeResolver: AResolver {
		FunctionPrototypeResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct ExtensionResolver: AResolver {
		ExtensionResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct TraitResolver: AResolver {
		TraitResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct ArrayResolver: AResolver {
		ArrayResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct ImportResolver: AResolver {
		ImportResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct TemplateResolver: AResolver {
		TemplateResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};
}

#endif
