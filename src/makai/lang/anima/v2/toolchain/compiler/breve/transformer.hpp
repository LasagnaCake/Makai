#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_TRANSFORMER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_TRANSFORMER_H

#include "../../assembler/assembler.hpp"
#include "../../../core/core.hpp"
#include "node.hpp"
#include "intermediate.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Breve::Transformer {
	struct ITransformer {
		struct Context: Intermediate {};

		virtual ~ITransformer();

		virtual void transform(Context& context, Node::Instance const& node) = 0;

		static Namespace::Instance declare(Context& context, Node::Instance const& node);
		static Namespace::Instance resolve(Context& context, UTF8StringList const& node);
		static UTF8StringList pathOf(Node::Instance const& node);
	};

	struct StructureDecl: ITransformer {
		void transform(Context& context, Node::Instance const& node) override;
	};

	struct FunctionDecl: ITransformer {
		void transform(Context& context, Node::Instance const& node) override;
	};

	struct FunctionImpl: ITransformer {
		void transform(Context& context, Node::Instance const& node) override;
	};
}

#endif
