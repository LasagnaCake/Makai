#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_RESOLVER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_RESOLVER_H

#include "../../assembler/assembler.hpp"
#include "../../../core/core.hpp"
#include "parser.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Breve {
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

	struct PathResolver: AResolver {
		PathResolver(): AResolver(Parser::Precedence::AV2_TAPP_PATH, false) {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct PrefixResolver: AResolver {
		PrefixResolver(): AResolver(Parser::Precedence::AV2_TAPP_PREFIX, true) {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct InlineMinimaResolver: AResolver {
		InlineMinimaResolver(): AResolver(Parser::Precedence::AV2_TAPP_PREFIX, true) {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct InfixResolver: AResolver {
		using AResolver::AResolver;
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct PostfixResolver: AResolver {
		PostfixResolver(): AResolver(Parser::Precedence::AV2_TAPP_POSTFIX, false) {}
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
		BranchResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct LoopResolver: AResolver {
		LoopResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct InlineIfElseResolver: AResolver {
		InlineIfElseResolver(): AResolver(Parser::Precedence::AV2_TAPP_INLINE_CONDITIONAL, false) {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct SpecialVarDeclResolver: AResolver {
		SpecialVarDeclResolver(): AResolver(Parser::Precedence::AV2_TAPP_DECL, false) {}
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

	struct FunctionDeclResolver: AResolver {
		FunctionDeclResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct VariableDeclResolver: AResolver {
		VariableDeclResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct ExtensionResolver: AResolver {
		ExtensionResolver(): AResolver() {}
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

	struct TemplateDeclResolver: AResolver {
		TemplateDeclResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct StructureDeclResolver: AResolver {
		StructureDeclResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct AttributeDeclResolver: AResolver {
		AttributeDeclResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct PropertyDeclResolver: AResolver {
		PropertyDeclResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct TraitDeclResolver: AResolver {
		TraitDeclResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct ModuleDeclResolver: AResolver {
		ModuleDeclResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct InlineStructureResolver: AResolver {
		InlineStructureResolver(): AResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};
}

#endif
