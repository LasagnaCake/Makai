#include "breve.hpp"
#include "intermediate.hpp"

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Compiler;

using Context = Breve::Context;

using enum Context::Tokenizer::Token::Type;

using enum Makai::Anima::V2::Toolchain::Compiler::Node::Content;

// TODO: This hellspawn

void Breve::invoke() {
	context.stack.pushBack(context.inter.root);
	Parser parser(context);
	auto const tree = parser.parse();
}
