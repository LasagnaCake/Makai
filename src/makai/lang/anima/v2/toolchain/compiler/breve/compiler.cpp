#include "compiler.hpp"
#include "composer.hpp"
#include "intermediate.hpp"
#include "../../assembler/minima.hpp"
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
	auto const min	= Composer(context).toMinima();
	Assembler::Minima::Context minctx;
	Makai::Lexer::CStyle::TokenStream stream;
	stream.open(min);
	Makai::List<Assembler::BaseContext::Axiom> ax;
	while (stream.next())
		ax.pushBack({stream.current(), true, ""});
	minctx.put(ax).pad();
	Assembler::Minima(minctx).invoke();
	result = minctx.program;
}
