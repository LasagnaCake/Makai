#include "resolver.hpp"

using namespace Makai::Anima::V2::Toolchain::Assembler;
using Type = Makai::Lexer::CStyle::TokenStream::Token::Type;
using enum Type;

Node::Instance ExpressionResolver::resolve(BaseContext& context) {
	Node::Instance result = Node::Instance::create();
	while (!context.empty()) {
		context.next();
		switch (context.type()) {
			case LTS_TT_IDENTIFIER: {

			};
		}
	}
}


Node::Instance BinaryResolver::resolve(BaseContext& context) {
}
