#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_PARSETREE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_PARSETREE_H

#include "../../../../../lexer/lexer.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	struct ParseTree {
		struct Node {
			using ID = ID::VLUID;
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

	private:
		Node::ID id = Node::ID::create(0);
	};
}

#endif