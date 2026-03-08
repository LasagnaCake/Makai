#include "resolver.hpp"
#include "context.hpp"

using namespace Makai::Anima::V2::Toolchain::Assembler;
using Type = Makai::Lexer::CStyle::TokenStream::Token::Type;
using enum Type;

Parser::Parser(BaseContext& context): context(context) {
	add(LTS_TT_IDENTIFIER, prefixes, new NameResolver());
	prefix(
		"sizeof",
		"countof",
		LTS_TT_PLUS,
		LTS_TT_MINUS,
		LTS_TT_LOGIC_NOT,
		LTS_TT_BIT_NOT
	);
}

Node::Instance Parser::nextExpression(usize precedence) {
	auto tok = context.next().token();
	if (!prefixes.contains(tok))
		context.error("Invalid expression!");
	Node::Instance lhs = prefixes[tok]->resolve(*this, null, tok);
	if (!infixes.contains(tok))
		return lhs;
	while (precedence < currentPrecedence()) {
		tok = context.next().token();
		lhs = infixes[tok]->resolve(*this, lhs, tok);
	}
	return lhs;
}

usize Parser::currentPrecedence() {
	auto const tok = context.peek();
	if (!infixes.contains(tok))
		return 0;
	return infixes[tok]->precedence(*this);
}

void Parser::prefix(BaseContext::Axiom::Type const op) {
	add(op, prefixes, new PrefixResolver());
}

void Parser::infix(BaseContext::Axiom::Type const op) {
	add(op, infixes, new InfixResolver());
}

void Parser::prefix(Makai::String const& op) {
	add(op, prefixes, new PrefixResolver());
}

void Parser::infix(Makai::String const& op) {
	add(op, infixes, new InfixResolver());
}

void Parser::add(BaseContext::Axiom::Type const op, OperatorBank& bank, Instance<IResolver> const& resolver) {
	BaseContext::Axiom ax;
	ax.strict = false;
	ax.type = op;
	bank[ax] = resolver;
}

void Parser::add(Makai::String const& op, OperatorBank& bank, Instance<IResolver> const& resolver) {
	BaseContext::Axiom ax;
	ax.type = LTS_TT_IDENTIFIER;
	ax.strict = true;
	ax.token = op;
	bank[ax] = resolver;
}

Node::Instance PrefixResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->children.pushBack(parser.nextExpression());
	return result;
}

Node::Instance InfixResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->children.pushBack(parser.nextExpression());
	return result;
}

usize InfixResolver::precedence(Parser& parser) {
	auto const tok = parser.context.peek();
}

Node::Instance PostfixResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->children.pushBack(lhs);
	result->postfix = true;
	return result;
}

Node::Instance BinaryResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->children.appendBack({
		lhs,
		parser.nextExpression()
	});
	return result;
}


Node::Instance InlineIfElseResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->children.appendBack({
		parser.nextExpression(),
		lhs
	});
	parser.context.expectNext(LTS_TT_IDENTIFIER, "'else'");
	if (parser.context.value().getString() != "else")
		parser.context.error("Expected 'else' here!");
	result->children.pushBack(parser.nextExpression());
	return result;
}
