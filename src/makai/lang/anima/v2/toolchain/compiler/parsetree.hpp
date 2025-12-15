#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_PARSETREE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_PARSETREE_H

#include "../../../../../lexer/lexer.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	struct ParseTree {
		struct Node {
			enum class Type {
				AV2_TC_PNT_INVALID	= -1,
				AV2_TC_PNT_VAR_DECL,
				AV2_TC_PNT_FN_DECL,
				AV2_TC_PNT_INLINE_EXPR,
				AV2_TC_PNT_BLOCK_EXPR,
				AV2_TC_PNT_ARITHMETIC,
				AV2_TC_PNT_FN_CALL,
				AV2_TC_PNT_OPERATOR
			};
			Type type;
			Data::Value	value;

			Instance<List<Node>> nodes = new List<Node>();

			struct Accessor {
				Reference<List<Node>>	nodes;
				usize const				index;

				constexpr operator Node&() const {return node();}
				
				constexpr Node& node() const {
					return (*nodes)[index];
				}
			};
		};

		constexpr Node::Accessor create() {
			root.nodes->pushBack({});
			return {root.nodes.reference(), root.nodes->size() - 1};
		}
		
		constexpr Node::Accessor create(Node& parent) {
			parent.nodes->pushBack({});
			return {parent.nodes.reference(), parent.nodes->size() - 1};
		}

		Node root;
	};
}

#endif