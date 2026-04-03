#include "compiler.hpp"
#include "intermediate.hpp"
#include "node.hpp"
#include "transformer.hpp"

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;

using Context = Compiler::Context;

using enum Context::Tokenizer::Token::Type;

using enum Makai::Anima::V2::Toolchain::Compiler::Breve::Node::Content;

// TODO: This hellspawn

void Compiler::invoke() {
	Parser parser(context);
	auto const tree	= parser.parse();
	auto const ir	= Transformer::TheEntireProgram().transform(context, tree);
}
