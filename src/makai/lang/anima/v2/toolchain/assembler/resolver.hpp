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

	struct IResolver;

	struct Parser {
		enum class Precedence {
			AV2_TAPP_NONE,
			AV2_TAPP_RHS_DECAY,
			AV2_TAPP_ASSIGN,
			AV2_TAPP_CONDITIONAL,
			AV2_TAPP_LOR,
			AV2_TAPP_LXOR,
			AV2_TAPP_LAND,
			AV2_TAPP_BOR,
			AV2_TAPP_BXOR,
			AV2_TAPP_BAND,
			AV2_TAPP_EQ_INEQ,
			AV2_TAPP_COMPARE,
			AV2_TAPP_ORDER,
			AV2_TAPP_BIT_SHIFT,
			AV2_TAPP_ADD_SUB,
			AV2_TAPP_MUL_DIV_REM,
			AV2_TAPP_PREFIX,
			AV2_TAPP_POSTFIX,
			AV2_TAPP_FN_CALL
		};

		BaseContext& context;

		virtual ~Parser();

		virtual Node::Instance nextExpression(Precedence prec = Precedence::AV2_TAPP_NONE);

		using OperatorBank = Map<BaseContext::Axiom, Instance<IResolver>>;

		OperatorBank prefixes;
		OperatorBank infixes;

		static void add(BaseContext::Axiom::Type const op, OperatorBank& bank, Instance<IResolver> const& resolver);
		static void add(String const& op, OperatorBank& bank, Instance<IResolver> const& resolver);

		void prefix(BaseContext::Axiom::Type const op);
		void prefix(String const& op);

		void infix(BaseContext::Axiom::Type const op);
		void infix(String const& op);

		template <class... Types> void prefix(Types const&... args) requires (sizeof...(Types) > 1)		{(..., prefix(args));}
		template <class... Types> void infix(Types const&... args) requires (sizeof...(Types) > 1)		{(..., prefix(args));}

		usize currentPrecedence();

		Parser(BaseContext& context);
	};

	struct IResolver {
		virtual ~IResolver();

		IResolver();

		virtual Parser::Precedence precedence(Parser&) {return Parser::Precedence::AV2_TAPP_NONE;}
		virtual Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) = 0;
	};

	struct NameResolver: IResolver {
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct PrefixResolver: IResolver {
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct InfixResolver: IResolver {
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
		Parser::Precedence precedence(Parser& parser) override;
	};

	struct PostfixResolver: IResolver {
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct FunctionCallResolver: IResolver {
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct BlockResolver: IResolver {
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct DeclarationResolver: IResolver {
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct BinaryResolver: IResolver {
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct InlineIfElseResolver: IResolver {
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct ExpressionResolver: IResolver {
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};
}

#endif
