#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_PARSETREE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_PARSETREE_H

#include "../../../../../lexer/lexer.hpp"
#include "../../instruction.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	struct ParseTree {
		struct Node {
			enum class Type {
				AV2_TC_PNT_INVALID	= -1,
				AV2_TC_PNT_LOCAL_DECL,
				AV2_TC_PNT_GLOBAL_DECL,
				AV2_TC_PNT_FN_DECL,
				AV2_TC_PNT_TYPE_DECL,
				AV2_TC_PNT_EXPRESSION,
				AV2_TC_PNT_FN_CALL,
				AV2_TC_PNT_DIRECT_INST,
				AV2_TC_PNT_OPERATOR
			};
			Type		type;
			Data::Value	value;

			LinkedList<Node> children;

			BinaryData<> compile() const;
		};

		constexpr Node& create() {
			return root.children.pushBack({}).back();
		}
		
		constexpr Node& create(Node& parent) {
			return parent.children.pushBack({}).back();
		}

		BinaryData<> compile() const;

		Node root;
	};
}

#endif