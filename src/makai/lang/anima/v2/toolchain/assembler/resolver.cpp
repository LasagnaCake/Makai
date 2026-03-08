#include "resolver.hpp"
#include "context.hpp"

using namespace Makai::Anima::V2::Toolchain::Assembler;
using Type = Makai::Lexer::CStyle::TokenStream::Token::Type;
using enum Type;

static Parser::Precedence precedenceOf(BaseContext::Axiom const& tok) {
	switch (tok.type) {
		using enum Parser::Precedence;
		case LTS_TT_IDENTIFIER: {
			auto const id = tok.value.getString();
			if (id == "else") return AV2_TAPP_NULL_DECAY;
			else if (id == "if") return AV2_TAPP_CONDITIONAL;
			else if (id == "and") return AV2_TAPP_LAND;
			else if (id == "or") return AV2_TAPP_LOR;
			else if (id == "xor") return AV2_TAPP_LXOR;
		}
		default: break;
		case LTS_TT_ASSIGN:
		case LTS_TT_ADD_ASSIGN:
		case LTS_TT_MUL_ASSIGN:
		case LTS_TT_SUB_ASSIGN:
		case LTS_TT_DIV_ASSIGN:
		case LTS_TT_MOD_ASSIGN:
		case LTS_TT_BIT_AND_ASSIGN:
		case LTS_TT_BIT_OR_ASSIGN:
		case LTS_TT_BIT_XOR_ASSIGN:
		case LTS_TT_BIT_SHIFT_LEFT_ASSIGN:
		case LTS_TT_BIT_SHIFT_RIGHT_ASSIGN: return AV2_TAPP_ASSIGN;
		case LTS_TT_MINUS:
		case LTS_TT_PLUS: return AV2_TAPP_ADD_SUB;
		case LTS_TT_STAR:
		case LTS_TT_FWD_SLASH:
		case LTS_TT_PERCENT: return AV2_TAPP_MUL_DIV_REM;
		case LTS_TT_BIT_SHIFT_RIGHT:
		case LTS_TT_BIT_SHIFT_LEFT: return AV2_TAPP_BIT_SHIFT;
		case LTS_TT_TILDE: return AV2_TAPP_ORDER;
		case LTS_TT_LESS_THAN:
		case LTS_TT_GREATER_THAN:
		case LTS_TT_COMPARE_GREATER_EQUALS:
		case LTS_TT_COMPARE_LESS_EQUALS: return AV2_TAPP_COMPARE;
		case LTS_TT_COMPARE_EQUALS:
		case LTS_TT_COMPARE_NOT_EQUALS: return AV2_TAPP_EQ_INEQ;
		case LTS_TT_BXOR: return AV2_TAPP_BXOR;
		case LTS_TT_BIT_AND: return AV2_TAPP_BAND;
		case LTS_TT_BIT_OR: return AV2_TAPP_BOR;
		case LTS_TT_LOGIC_AND: return AV2_TAPP_LAND;
		case LTS_TT_LOGIC_OR: return AV2_TAPP_LOR;
	}
	return Parser::Precedence::AV2_TAPP_NONE;
}

Parser::Parser(BaseContext& context): context(context) {
	add({{.type = LTS_TT_IDENTIFIER, .value = ""}}, prefixes, new NameResolver(Precedence::AV2_TAPP_NONE, false));
	prefix(
		"sizeof",
		"countof",
		LTS_TT_PLUS,
		LTS_TT_MINUS,
		LTS_TT_LOGIC_NOT,
		LTS_TT_BIT_NOT
	);
}

Node::Instance Parser::nextExpression(Parser::Precedence precedence) {
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

Parser::Precedence Parser::currentPrecedence() {
	auto const tok = context.peek();
	if (!infixes.contains(tok))
		return Parser::Precedence::AV2_TAPP_NONE;
	return infixes[tok]->precedence;
}

void Parser::prefix(BaseContext::Axiom::Type const op) {
	BaseContext::Axiom ax;
	ax.strict = false;
	ax.type = op;
	add(ax, prefixes, new PrefixResolver(Precedence::AV2_TAPP_NONE, false));
}

void Parser::infix(BaseContext::Axiom::Type const op, bool const rightwards) {
	BaseContext::Axiom ax;
	ax.strict = false;
	ax.type = op;
	add(ax, infixes, new InfixResolver(precedenceOf(ax), rightwards));
}

void Parser::prefix(Makai::String const& op) {
	BaseContext::Axiom ax;
	ax.type = LTS_TT_IDENTIFIER;
	ax.strict = true;
	ax.token = op;
	add(op, prefixes, new PrefixResolver(Precedence::AV2_TAPP_NONE, false));
}

void Parser::infix(Makai::String const& op, bool const rightwards) {
	BaseContext::Axiom ax;
	ax.type = LTS_TT_IDENTIFIER;
	ax.strict = true;
	ax.token = op;
	add(ax, infixes, new InfixResolver(precedenceOf(ax), rightwards));
}

void Parser::add(BaseContext::Axiom const op, OperatorBank& bank, Instance<AResolver> const& resolver) {
	bank[op] = resolver;
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
	result->children.appendBack({lhs, parser.nextExpression()});
	return result;
}

Parser::Precedence InfixResolver::precedenceOf(BaseContext::Axiom const& tok) {
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
		parser.nextExpression(precedenceOf(token))
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
