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
			else if (id == "cross" || id == "fcross")
				return AV2_TAPP_CROSS_FCROSS;
			else if (id == "atan") return AV2_TAPP_ATAN2;
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
		case LTS_TT_RAISE: return AV2_TAPP_BXOR;
		case LTS_TT_BIT_AND: return AV2_TAPP_BAND;
		case LTS_TT_BIT_OR: return AV2_TAPP_BOR;
		case LTS_TT_LOGIC_AND: return AV2_TAPP_LAND;
		case LTS_TT_LOGIC_OR: return AV2_TAPP_LOR;
		case LTS_TT_INCREMENT:
		case LTS_TT_DECREMENT:
		case LTS_TT_DOT: return AV2_TAPP_POSTFIX;
		case LTS_TT_COMMA: return AV2_TAPP_RHS_DECAY;
		case LTS_TT_QUESTION: return AV2_TAPP_NULL_DECAY;
	}
	return Parser::Precedence::AV2_TAPP_NONE;
}

Parser::Parser(BaseContext& context): context(context) {
	direct(
		LTS_TT_IDENTIFIER,
		LTS_TT_INTEGER,
		LTS_TT_DOUBLE_QUOTE_STRING,
		LTS_TT_SINGLE_QUOTE_STRING,
		LTS_TT_REAL,
		LTS_TT_CHARACTER
	);
	prefix(
		"sizeof",
		"countof",
		"typeof",
		"sin",
		"cos",
		"tan",
		"asin",
		"acos",
		"atan",
		"log2",
		"log10",
		"ln",
		"not",
		"return",
		LTS_TT_PLUS,
		LTS_TT_MINUS,
		LTS_TT_LOGIC_NOT,
		LTS_TT_BIT_NOT
	);
	infix(LTS_TT_PLUS, false);
	infix(LTS_TT_MINUS, false);
	infix(LTS_TT_DIVIDE, false);
	infix(LTS_TT_STAR, false);
	infix(LTS_TT_MODULO, false);
	infix(LTS_TT_DOT, false);
	infix(LTS_TT_COMMA, false);
	infix(LTS_TT_AMP, false);
	infix(LTS_TT_PIPE, false);
	infix(LTS_TT_RAISE, false);
	infix(LTS_TT_LOGIC_AND, false);
	infix(LTS_TT_LOGIC_OR, false);
	infix(LTS_TT_LESS_THAN, false);
	infix(LTS_TT_GREATER_THAN, false);
	infix(LTS_TT_COMPARE_LESS_EQUALS, false);
	infix(LTS_TT_COMPARE_GREATER_EQUALS, false);
	infix(LTS_TT_COMPARE_EQUALS, false);
	infix(LTS_TT_COMPARE_NOT_EQUALS, false);
	infix("xor", false);
	infix("atan", false);
	infix("cross", false);
	infix("fcross", false);
	infix("is", false);
	infix("as", false);
	infix(LTS_TT_ADD_ASSIGN, true);
	infix(LTS_TT_SUB_ASSIGN, true);
	infix(LTS_TT_MUL_ASSIGN, true);
	infix(LTS_TT_DIV_ASSIGN, true);
	infix(LTS_TT_MOD_ASSIGN, true);
	infix(LTS_TT_BIT_AND_ASSIGN, true);
	infix(LTS_TT_BIT_OR_ASSIGN, true);
	infix(LTS_TT_BIT_XOR_ASSIGN, true);
	infix(LTS_TT_BIT_SHIFT_LEFT_ASSIGN, true);
	infix(LTS_TT_BIT_SHIFT_RIGHT_ASSIGN, true);
	postfix(
		LTS_TT_INCREMENT,
		LTS_TT_DECREMENT
	);
	add({{.type = LTS_TT_OPEN_PAREN}}, prefixes, new SubExpressionResolver());
	add({{.type = LTS_TT_OPEN_CURLY}}, prefixes, new BlockResolver());
	add({{.type = LTS_TT_OPEN_BRACKET}}, prefixes, new ArrayResolver());
	add({{.type = LTS_TT_OPEN_BRACKET}}, infixes, new ArrayResolver());
	add({{.type = LTS_TT_COLON}}, infixes, new DeclarationResolver());
	add({{.type = LTS_TT_OPEN_PAREN}}, infixes, new FunctionCallResolver());
	add({{.type = LTS_TT_EXCLAMATION}}, infixes, new FunctionCallResolver());
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

Node::Instance Parser::parse() {
	return nextExpression();
}

Parser::Precedence Parser::currentPrecedence() {
	auto const tok = context.peek();
	if (!infixes.contains(tok))
		return Parser::Precedence::AV2_TAPP_NONE;
	return infixes[tok]->precedence;
}

void Parser::direct(BaseContext::Axiom::Type const op) {
	BaseContext::Axiom ax;
	ax.strict = false;
	ax.type = op;
	add(ax, prefixes, new DirectResolver(Precedence::AV2_TAPP_NONE, false));
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
	add(ax, prefixes, new PrefixResolver(Precedence::AV2_TAPP_NONE, false));
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
	result->children.pushBack(parser.nextExpression(precedence));
	return result;
}

Node::Instance InfixResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->children.appendBack({lhs, parser.nextExpression(Parser::Precedence{enumcast(precedence) - rightwards})});
	return result;
}

Node::Instance PostfixResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->children.pushBack(lhs);
	result->postfix = true;
	return result;
}

Node::Instance InlineIfElseResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->base.token = "@::if-else";
	result->children.appendBack({
		parser.nextExpression(precedence),
		lhs
	});
	parser.context.expectNext(LTS_TT_IDENTIFIER, "'else'");
	if (parser.context.value().getString() != "else")
		parser.context.error("Expected 'else' here!");
	result->children.pushBack(parser.nextExpression(precedence));
	return result;
}

Node::Instance SubExpressionResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	auto const expr = parser.nextExpression();
	parser.context.expectNext(LTS_TT_CLOSE_PAREN);
	return expr;
}

Node::Instance FunctionCallResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->base.token = "@::call";
	result->children.pushBack(lhs);
	if (parser.context.type() == LTS_TT_EXCLAMATION) return result;
	while (true) {
		if (parser.context.next().has(LTS_TT_CLOSE_PAREN)) break;
		result->children.pushBack(parser.nextExpression());
		parser.context.expectNext(LTS_TT_COMMA);
		if (parser.context.next().has(LTS_TT_CLOSE_PAREN))
			parser.context.error("Expected expression after the comma!");
	}
	parser.context.expectNext(LTS_TT_CLOSE_PAREN);
	return result;
}

Node::Instance BlockResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->base.token = "@::block";
	while (true) {
		if (parser.context.next().has(LTS_TT_CLOSE_CURLY)) break;
		result->children.pushBack(parser.nextExpression());
	}
	parser.context.expectNext(LTS_TT_CLOSE_CURLY);
	return result;
}

Node::Instance ArrayResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->base.token = lhs ? "@::index" : "@::array";
	if (lhs)
		result->children.pushBack(lhs);
	while (true) {
		if (parser.context.next().has(LTS_TT_CLOSE_BRACKET)) break;
		result->children.pushBack(parser.nextExpression());
		parser.context.expectNext(LTS_TT_COMMA);
		if (parser.context.next().has(LTS_TT_CLOSE_BRACKET))
			parser.context.error("Expected expression after the comma!");
	}
	parser.context.expectNext(LTS_TT_CLOSE_BRACKET);
	return result;
}


Node::Instance SubfieldResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->base.token = "@::field:";
	do {
		parser.context.expectNext(LTS_TT_IDENTIFIER);
		auto const id = parser.context.value().getString();
		result->base.token += "/" + id;
		parser.context.next();
	} while (parser.context.has(LTS_TT_DOT));
	parser.context.pad();
	return result;
}

Node::Instance DeclarationResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->base.token = "@::decl";
	return result;
}
