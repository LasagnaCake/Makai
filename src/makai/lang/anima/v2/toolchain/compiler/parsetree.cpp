#include "parsetree.hpp"

using namespace Makai::Anima::V2::Toolchain::Compiler;

Makai::BinaryData<> ParseTree::Node::compile() const {
	return {};
}

Makai::BinaryData<> ParseTree::compile() const {
	return root.compile();
}