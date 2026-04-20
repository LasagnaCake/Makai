#include "project.hpp"

using namespace Makai::Anima::V2::Toolchain::Compiler;
using Type = Makai::Lexer::CStyle::TokenStream::Token::Type;
using enum Type;

Makai::Data::Value Project::serialize() const {
	Makai::Data::Value out;
	if (main.path.size())
		out["main"]	= main.path;
	else out["main_src"] = main.source;
	out["src"]	= sources.toList<Makai::Data::Value>();
	return out;
}

Project Project::deserialize(Makai::Data::Value const& v) {
	Project out;
	return out;
}
