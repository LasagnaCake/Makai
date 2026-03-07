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
			AV2_TANT_OPERATION,
			AV2_TANT_FN_CALL,
			AV2_TANT_VAR_DECL,
			AV2_TANT_FN_DECL,
			AV2_TANT_PATH_RESOLVE,
		};

		Type				type;
		List<Instance>		children;
		Core::DataLocation	source;
		BaseContext::Axiom	base;
	};

	struct IResolver {
		virtual Node::Instance resolve(BaseContext& context) = 0;
	};

	struct ITypeResolver: IResolver {
	};

	struct IValueResolver: IResolver {
	};

	struct FunctionResolver: IValueResolver {
		Node::Instance resolve(BaseContext& context) override;
	};

	struct PathResolver: IValueResolver {
		Node::Instance resolve(BaseContext& context) override;
	};

	struct ExpressionResolver: IValueResolver {
		Node::Instance resolve(BaseContext& context) override;
	};

	struct AddSubResolver: IValueResolver {
		Node::Instance resolve(BaseContext& context) override;
	};

	struct MulDivRemResolver: IValueResolver {
		Node::Instance resolve(BaseContext& context) override;
	};

	struct FunctionCallResolver: IValueResolver {
		Node::Instance resolve(BaseContext& context) override;
	};

	struct MemberGetResolver: IValueResolver {
		Node::Instance resolve(BaseContext& context) override;
	};

	struct SubscriptResolver: IValueResolver {
		Node::Instance resolve(BaseContext& context) override;
	};

	struct BinaryResolver: IValueResolver {
		Node::Instance resolve(BaseContext& context) override;
	};

	struct UnaryPrefixResolver: IValueResolver {
		Node::Instance resolve(BaseContext& context) override;
	};

	struct UnaryPostfixResolver: IValueResolver {
		Node::Instance resolve(BaseContext& context) override;
	};

	struct EquationResolver: IValueResolver {
		Node::Instance resolve(BaseContext& context) override;
	};
}

#endif
