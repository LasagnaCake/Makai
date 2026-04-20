#include "compiler.hpp"
#include "composer.hpp"
#include "intermediate.hpp"
#include "../../assembler/minima.hpp"
#include "node.hpp"
#include "transformer.hpp"

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain;
using namespace Compiler;

// TODO: This hellspawn

Makai::Data::Value Breve::compile(
	Makai::UTF8String const& fname,
	Makai::UTF8String const& file,
	CompilationLevel const level,
	bool const strip,
	Makai::UTF8String const& append
) {
	Assembler::BaseContext ctx;
	Breve::Parser parser(ctx);
	Makai::Lexer::CStyle::TokenStream stream;
	stream.open(file);
	Makai::List<Assembler::BaseContext::Axiom> ax;
	while (stream.next())
		ax.pushBack({stream.current(), true, fname});
	if (!stream.ok())
		throw Makai::Error::InvalidValue(
			"Parsing failure!",
			stream.error().value().what
		);
	ctx.put(ax).pad();
	switch (level) {
		using enum Breve::CompilationLevel;
		case CompilationLevel::AV2_TCB_CCL_PARSE_TREE: {
			auto const i = parser.parse();
			return i->serialize();
		}
		case CompilationLevel::AV2_TCB_CCL_INTERMEDIATE: {
			Transformer::ATransformer::Context ctx;
			Transformer::TheEntireProgram tf;
			tf.transform(ctx, parser.parse());
			return ctx.serialize();
		}
		case CompilationLevel::AV2_TCB_CCL_MINIMA: {
			Transformer::ATransformer::Context ctx;
			Transformer::TheEntireProgram tf;
			Composer comp(ctx);
			tf.transform(ctx, parser.parse());
			return comp.toMinima().toString();
		}
		case CompilationLevel::AV2_TCB_CCL_FULL: {
			Transformer::ATransformer::Context ctx;
			Transformer::TheEntireProgram tf;
			Composer comp(ctx);
			tf.transform(ctx, parser.parse());
			return Assembler::Minima::assemble(fname, append + comp.toMinima(), strip);
		}
	}
}
