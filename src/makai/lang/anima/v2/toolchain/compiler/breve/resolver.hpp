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
		virtual ~DirectResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct PathResolver: AResolver {
		PathResolver(): AResolver(Parser::Precedence::AV2_TAPP_PATH, false) {}
		virtual ~PathResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct PrefixResolver: AResolver {
		PrefixResolver(): AResolver(Parser::Precedence::AV2_TAPP_PREFIX, true) {}
		virtual ~PrefixResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct InlineMinimaResolver: AResolver {
		InlineMinimaResolver(): AResolver(Parser::Precedence::AV2_TAPP_PREFIX, true) {}
		virtual ~InlineMinimaResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct InfixResolver: AResolver {
		using AResolver::AResolver;
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct PostfixResolver: AResolver {
		PostfixResolver(): AResolver(Parser::Precedence::AV2_TAPP_POSTFIX, false) {}
		virtual ~PostfixResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct FunctionCallResolver: AResolver {
		FunctionCallResolver(): AResolver(Parser::Precedence::AV2_TAPP_FN_CALL, true) {}
		virtual ~FunctionCallResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct BlockResolver: AResolver {
		BlockResolver(): AResolver(Parser::Precedence::AV2_TAPP_BLOCK, false) {}
		virtual ~BlockResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct SubExpressionResolver: AResolver {
		SubExpressionResolver(): AResolver() {}
		virtual ~SubExpressionResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct BranchResolver: AResolver {
		BranchResolver(): AResolver() {}
		virtual ~BranchResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct LoopResolver: AResolver {
		LoopResolver(): AResolver() {}
		virtual ~LoopResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct InlineIfElseResolver: AResolver {
		InlineIfElseResolver(): AResolver(Parser::Precedence::AV2_TAPP_INLINE_CONDITIONAL, false) {}
		virtual ~InlineIfElseResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct AttributeResolver: AResolver {
		AttributeResolver(): AResolver() {}
		virtual ~AttributeResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct AssignmentResolver: AResolver {
		AssignmentResolver(): AResolver(Parser::Precedence::AV2_TAPP_ASSIGN, true) {}
		virtual ~AssignmentResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct FunctionPrototypeResolver: AResolver {
		FunctionPrototypeResolver(): AResolver() {}
		virtual ~FunctionPrototypeResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct FunctionDeclResolver: AResolver {
		FunctionDeclResolver(): AResolver(Parser::Precedence::AV2_TAPP_DECL, false) {}
		virtual ~FunctionDeclResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct VariableDeclResolver: AResolver {
		VariableDeclResolver(): AResolver(Parser::Precedence::AV2_TAPP_DECL, false) {}
		virtual ~VariableDeclResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct ExtensionResolver: AResolver {
		ExtensionResolver(): AResolver() {}
		virtual ~ExtensionResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct ArrayResolver: AResolver {
		ArrayResolver(): AResolver() {}
		virtual ~ArrayResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct ImportResolver: AResolver {
		ImportResolver(): AResolver() {}
		virtual ~ImportResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct TemplateDeclResolver: AResolver {
		TemplateDeclResolver(): AResolver() {}
		virtual ~TemplateDeclResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct StructureDeclResolver: AResolver {
		StructureDeclResolver(): AResolver() {}
		virtual ~StructureDeclResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct PropertyDeclResolver: AResolver {
		PropertyDeclResolver(): AResolver() {}
		virtual ~PropertyDeclResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct TraitDeclResolver: AResolver {
		TraitDeclResolver(): AResolver() {}
		virtual ~TraitDeclResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct ModuleDeclResolver: AResolver {
		ModuleDeclResolver(): AResolver() {}
		virtual ~ModuleDeclResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct InlineStructureResolver: AResolver {
		InlineStructureResolver(): AResolver() {}
		virtual ~InlineStructureResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct DynamicOperatorDeclResolver: AResolver {
		DynamicOperatorDeclResolver(): AResolver() {}
		virtual ~DynamicOperatorDeclResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct MainBlockResolver: AResolver {
		MainBlockResolver(): AResolver() {}
		virtual ~MainBlockResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct UsingResolver: AResolver {
		UsingResolver(): AResolver() {}
		virtual ~UsingResolver() {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};

	struct DynamicOperatorResolver: AResolver {
		enum class Class {
			AV2_TA_DORC_PREFIX,
			AV2_TA_DORC_INFIX,
			AV2_TA_DORC_POSTFIX,
		};
		Class opClass;
		DynamicOperatorResolver(Class const opClass, Parser::Precedence const precedence, bool const rightToLeft): AResolver(precedence, rightToLeft), opClass(opClass) {}
		Node::Instance resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) override;
	};
}

#endif
