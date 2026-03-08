#include "resolver.hpp"

using namespace Makai;

using namespace Anima::V2::Toolchain::Compiler;
using Type = Lexer::CStyle::TokenStream::Token::Type;
using enum Type;

Node::Instance DirectResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	auto isIdentifier = parser.context.type() == LTS_TT_IDENTIFIER;
	Node::Instance result = Node::Instance::create();
	result->base = token;
	if (isIdentifier) {
		isIdentifier = false;
		auto const id = token.token;
		if (id == "true")		result->value = true;
		else if (id == "false")	result->value = false;
		else if (id == "null")	result->value = null;
		else {
			isIdentifier = true;
			result->value = id;
		}
	} else result->value = token.value;
	result->content = isIdentifier ? Node::Content::AV2_TANC_PATH : Node::Content::AV2_TANC_VALUE;
	return result;
}

Node::Instance PrefixResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->children.pushBack(parser.nextExpression(precedence));
	result->content = Node::Content::AV2_TANC_PREFIX_OP;
	return result;
}

Node::Instance InfixResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->children.appendBack({lhs, parser.nextExpression(precedence)});
	result->content = Node::Content::AV2_TANC_INFIX_OP;
	return result;
}

Node::Instance PostfixResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->children.pushBack(lhs);
	result->content = Node::Content::AV2_TANC_POSTFIX_OP;
	return result;
}

Node::Instance InlineIfElseResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->children.appendBack({
		parser.nextExpression(precedence),
		lhs
	});
	parser.context.expectNext(LTS_TT_IDENTIFIER, "'else'");
	if (parser.context.value().getString() != "else")
		parser.context.error("Expected 'else' here!");
	result->children.pushBack(parser.nextExpression(precedence));
	result->content = Node::Content::AV2_TANC_INLINE_IF_ELSE;
	return result;
}

Node::Instance SubExpressionResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	auto const expr = parser.nextExpression(precedence);
	parser.context.expectNext(LTS_TT_CLOSE_PAREN);
	return expr;
}

Node::Instance FunctionCallResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->children.pushBack(lhs);
	result->content = Node::Content::AV2_TANC_FN_CALL;
	if (parser.context.type() == LTS_TT_EXCLAMATION) return result;
	while (true) {
		if (parser.context.has(LTS_TT_CLOSE_PAREN)) break;
		result->children.pushBack(parser.nextExpression(precedence));
		parser.context.expect(LTS_TT_COMMA);
		if (parser.context.peek().type == LTS_TT_CLOSE_PAREN)
			parser.context.error("Expected expression after the comma!");
	}
	parser.context.expect(LTS_TT_CLOSE_PAREN);
	return result;
}

Node::Instance BlockResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	while (true) {
		if (parser.context.next().has(LTS_TT_CLOSE_CURLY)) break;
		result->children.pushBack(parser.nextExpression(precedence));
	}
	parser.context.expectNext(LTS_TT_CLOSE_CURLY);
	result->content = Node::Content::AV2_TANC_BLOCK;
	return result;
}

Node::Instance ArrayResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	if (lhs)
		result->children.pushBack(lhs);
	while (true) {
		if (parser.context.has(LTS_TT_CLOSE_BRACKET)) break;
		result->children.pushBack(parser.nextExpression(precedence));
		parser.context.expect(LTS_TT_COMMA);
		if (parser.context.peek().type == LTS_TT_CLOSE_BRACKET)
			parser.context.error("Expected expression after the comma!");
	}
	parser.context.expect(LTS_TT_CLOSE_BRACKET);
	result->content = lhs ? Node::Content::AV2_TANC_SUBSCRIPT : Node::Content::AV2_TANC_ARRAY;
	return result;
}

Node::Instance DeclarationResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_DECLARATION;
	return result;
}

Node::Instance BranchResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_BRANCH;
	result->value = token.token;
	return result;
}

Node::Instance LoopResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_LOOP;
	result->value = token.token;
	return result;
}

Node::Instance ImportResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_IMPORT;
	result->value = token.token;
	return result;
}

Node::Instance AssignmentResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_ASSIGNMENT;
	if (!(
		lhs->content == Node::Content::AV2_TANC_ASSIGNMENT
	||	lhs->content == Node::Content::AV2_TANC_PATH
	)) parser.context.error("Expected assignment chain or declaration path here!");
	result->children.appendBack({
		lhs,
		parser.nextExpression(precedence)
	});
	return result;
}
