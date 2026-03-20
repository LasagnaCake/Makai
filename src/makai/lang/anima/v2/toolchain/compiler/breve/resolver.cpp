#include "resolver.hpp"

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;
using Type = Makai::Lexer::CStyle::TokenStream::Token::Type;
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
	result->content = isIdentifier ? Node::Content::AV2_TANC_NAME : Node::Content::AV2_TANC_VALUE;
	return result;
}

Node::Instance PrefixResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->lhs = parser.nextExpression(precedence);
	result->content = Node::Content::AV2_TANC_PREFIX_OP;
	return result;
}

Node::Instance InfixResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->lhs = lhs;
	result->rhs = parser.nextExpression(precedence);
	result->content = Node::Content::AV2_TANC_INFIX_OP;
	return result;
}

Node::Instance PostfixResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->lhs = lhs;
	result->content = Node::Content::AV2_TANC_POSTFIX_OP;
	return result;
}

Node::Instance InlineMinimaResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	parser.context.expectNext(LTS_TT_OPEN_CURLY).next();
	while (!parser.context.has(LTS_TT_CLOSE_CURLY)) {
		result->interject.pushBack(parser.context.token());
		parser.context.next();
	}
	return result;
}

Node::Instance InlineIfElseResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->children.pushBack(lhs);
	result->lhs = parser.nextExpression(precedence);
	parser.context.expectNext(LTS_TT_IDENTIFIER, "'else'");
	if (parser.context.value().getString() != "else")
		parser.context.error("Expected 'else' here!");
	result->rhs = parser.nextExpression(precedence);
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
	result->lhs = lhs;
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
		result->lhs = lhs;
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

Node::Instance BranchResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_BRANCH;
	result->value = token.token;
	// TODO: This
	return result;
}

Node::Instance LoopResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_LOOP;
	result->value = token.token;
	// TODO: This
	return result;
}

Node::Instance ImportResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_IMPORT;
	result->value = token.token;
	// TODO: This
	return result;
}

Node::Instance AssignmentResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_ASSIGNMENT;
	if (!(
		lhs->content == Node::Content::AV2_TANC_ASSIGNMENT
	||	lhs->content == Node::Content::AV2_TANC_PATH
	||	lhs->content == Node::Content::AV2_TANC_DECLARATION
	||	lhs->content == Node::Content::AV2_TANC_SUBSCRIPT
	)) parser.context.error("Expected assignment chain or declaration path here!");
	result->lhs = lhs;
	result->rhs = parser.nextExpression(precedence);
	return result;
}

Node::Instance ExtensionResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_TYPE_EXTENSION;
	result->lhs = parser.nextExpression();
	if (!lhs->isPathOrName())
		parser.context.error("Invalid expression for extension!");
	if (parser.context.peek().type == LTS_TT_IDENTIFIER) {
		auto const id = parser.context.peek().value.getString();
		if (id == "with")
			parser.context.next();
	}
	result->rhs = parser.nextExpression();
	if (!result->rhs->isDeclarationOrBlock())
		parser.context.error("Expected declaration or code block here!");
	return result;
}

Node::Instance SpecialVarDeclResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_DECLARATION;
	// TODO: This
	return result;
}

Node::Instance FunctionPrototypeResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	// TODO: This
	return result;
}

Node::Instance VariableDeclResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	// TODO: This
	return result;
}

Node::Instance PathResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_PATH;
	result->lhs = lhs;
	result->rhs = parser.nextExpression(precedence);
	return result;
}
Node::Instance DynamicOperatorResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	return result;
	switch (opClass) {
		case Class::AV2_TA_DORC_PREFIX: {
			result->lhs = parser.nextExpression(precedence);
			result->content = Node::Content::AV2_TANC_PREFIX_OP;
		}
		case Class::AV2_TA_DORC_INFIX: {
			result->lhs = lhs;
			result->rhs = parser.nextExpression(precedence);
			result->content = Node::Content::AV2_TANC_INFIX_OP;
		}
		case Class::AV2_TA_DORC_POSTFIX: {
			result->lhs = lhs;
			result->content = Node::Content::AV2_TANC_POSTFIX_OP;
		}
	}
}

Node::Instance DynamicOperatorDeclResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	auto const opkey = parser.context.expectNext(LTS_TT_IDENTIFIER, "operator name").token();
	if (
		token.token == "prefix"
	||	token.token == "postfix"
	) {
		Instance<DynamicOperatorResolver> op = new DynamicOperatorResolver(
			token.token == "prefix"
		?	DynamicOperatorResolver::Class::AV2_TA_DORC_PREFIX
		:	DynamicOperatorResolver::Class::AV2_TA_DORC_POSTFIX,
			token.token == "prefix"
		?	decltype(precedence)::AV2_TAPP_PREFIX
		:	decltype(precedence)::AV2_TAPP_POSTFIX,
			false
		);
		if (token.token == "prefix") {
			if (parser.prefixes.contains(opkey))
				parser.context.error("Redeclaration of operator ["+ opkey.token +"]!");
			parser.add(opkey, parser.prefixes, op.as<AResolver>());
		}
		else {
			if (parser.infixes.contains(opkey))
				parser.context.error("Redeclaration of operator ["+ opkey.token +"]!");
			parser.add(opkey, parser.infixes, op.as<AResolver>());
		}
	} else {
		int precOffset = 0;
		switch (parser.context.next().type()) {
			case LTS_TT_GREATER_THAN:	precOffset = 1;		break;
			case LTS_TT_EQUALS:			precOffset = 0;		break;
			case LTS_TT_LESS_THAN:		precOffset = -1;	break;
			default: parser.context.error("Invalid precedence specifier!");
		}
		auto const precedence = enumcast(parser.precedenceOf(parser.context.next().token())) + precOffset;
		bool rightToLeft = false;
		if (parser.context.peek().type == LTS_TT_OPEN_BRACKET) {
			auto const t = parser.context.next().next().type();
			switch (t) {
				case LTS_TT_BIT_SHIFT_LEFT:		rightToLeft = true;		break;
				case LTS_TT_BIT_SHIFT_RIGHT:	rightToLeft = false;	break;
				default: parser.context.error("Invalid direction specifier!");
			}
			parser.context.expectNext(LTS_TT_CLOSE_BRACKET);
		}
		if (parser.infixes.contains(opkey))
			parser.context.error("Redeclaration of operator ["+ opkey.token +"]!");
		parser.add(
			opkey,
			parser.infixes,
			new DynamicOperatorResolver(
				DynamicOperatorResolver::Class::AV2_TA_DORC_INFIX,
				Makai::Cast::as<Parser::Precedence>(precedence),
				rightToLeft
			)
		);
	}
}
