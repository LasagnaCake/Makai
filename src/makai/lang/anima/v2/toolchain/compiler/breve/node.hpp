#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_NODE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_NODE_H

#include "../../assembler/assembler.hpp"
#include "../../../core/core.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Breve {
	using BaseContext = Assembler::BaseContext;

	struct Node: ID::Identifiable<Node const, ID::VLUID> {
		using Instance = Instance<Node>;

		using ID::Identifiable<Node const, ID::VLUID>::id;

		enum class Content {
			AV2_TANC_EMPTY,
			AV2_TANC_VALUE,
			AV2_TANC_NAME,
			AV2_TANC_DECLARATION,
			AV2_TANC_DEFINITION,
			AV2_TANC_ASSIGNMENT,
			AV2_TANC_FN_PROTOTYPE,
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
			AV2_TANC_ATTRIBUTE,
			AV2_TANC_TYPE_EXTENSION,
			AV2_TANC_ALIAS,
			AV2_TANC_UNSCOPING,
			AV2_TANC_INLINE_MINIMA,
			AV2_TANC_EMPTY_DECAY,
			AV2_TANC_NEW,
			AV2_TANC_DROP,
			AV2_TANC_CUSTOM_PREFIX_OP,
			AV2_TANC_CUSTOM_INFIX_OP,
			AV2_TANC_CUSTOM_POSTFIX_OP,
			AV2_TANC_RANGE,
			AV2_TANC_ITERATION,
			AV2_TANC_EXPANSION,
			AV2_TANC_NULLABLE_DECL,
		};

		Content							content = Content::AV2_TANC_EMPTY;
		Data::Value						value;
		Instance						leftSide;
		Instance						middle;
		Instance						rightSide;
		List<Instance>					children;
		Nullable<Core::DataLocation>	source;
		BaseContext::Axiom				base;
		List<BaseContext::Axiom>		interject;

		constexpr bool isPathOrName() const {
			return content == Content::AV2_TANC_PATH || content == Content::AV2_TANC_NAME;
		}

		constexpr bool isDeclarationOrBlock() const {
			return content == Content::AV2_TANC_DECLARATION || content == Content::AV2_TANC_BLOCK;
		}

		constexpr bool isVariableDeclaration() const {
			return (
				content == Content::AV2_TANC_DECLARATION
			&&	(
					base.type == As<decltype(base.type)>(':')
				||	base.type == As<decltype(base.type)>::LTS_TT_DECLARE
				)
			);
		}

		constexpr bool isBlock() const {
			return content == Content::AV2_TANC_BLOCK;
		}

		constexpr String name() const {
			auto const base = id();
			return toString("_ID_u", base[3], "u", base[2], "u", base[1], "u", base[0], "_");
		}

		Data::Value serialize() const;

		static String asString(Content const content);
	};
}

#endif
