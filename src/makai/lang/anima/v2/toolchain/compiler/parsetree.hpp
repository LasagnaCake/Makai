#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_PARSETREE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_PARSETREE_H

#include "../../../../../lexer/lexer.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler {
	struct ParseTree {
		struct Node {
			using ID = ID::VLUID;
			Data::Value	value;
		};

		NodeGraph::DU<Node::ID>	graph;
		Map<Node::ID, Node>		nodes;

		constexpr KeyValuePair<Node::ID, Node&> create() {
			nodes[++id] = {};
			return {id, nodes[id]};
		}

	private:
		Node::ID id = Node::ID::create(0);
	};
}

#endif