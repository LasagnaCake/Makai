#include "resolver.hpp"

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;
using Type = Makai::Lexer::CStyle::TokenStream::Token::Type;
using enum Type;

Node::Instance DirectResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving direct expression...");
	auto isIdentifier = parser.context.type() == LTS_TT_IDENTIFIER;
	Node::Instance result = Node::Instance::create();
	result->base = token;
	if (isIdentifier) {
		isIdentifier = false;
		auto const id = token.text;
		if (id == "true")		result->value = true;
		else if (id == "false")	result->value = false;
		else if (id == "null")	result->value = null;
		else {
			isIdentifier = true;
			result->value = id.toString();
		}
	} else result->value = token.value;
	result->content = isIdentifier ? Node::Content::AV2_TANC_NAME : Node::Content::AV2_TANC_VALUE;
	DEBUGLN("Direct:DONE!");
	return result;
}

Node::Instance PrefixResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving prefix expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->lhs = parser.nextExpression(precedence);
	result->content = Node::Content::AV2_TANC_PREFIX_OP;
	DEBUGLN("Prefix:DONE!");
	return result;
}

Node::Instance InfixResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving infix expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->lhs = lhs;
	result->rhs = parser.nextExpression(precedence);
	result->content = Node::Content::AV2_TANC_INFIX_OP;
	DEBUGLN("Infix:DONE!");
	return result;
}

Node::Instance PostfixResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving postfix expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->lhs = lhs;
	result->content = Node::Content::AV2_TANC_POSTFIX_OP;
	DEBUGLN("Postfix:DONE!");
	return result;
}

Node::Instance InlineMinimaResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving inline assembly expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	parser.context.expectNext(LTS_TT_OPEN_CURLY).next();
	while (!parser.context.has(LTS_TT_CLOSE_CURLY)) {
		result->interject.pushBack(parser.context.token());
		parser.context.next();
	}
	DEBUGLN("Assembly:DONE!");
	return result;
}

Node::Instance InlineIfElseResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving inline if-else expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->children.pushBack(lhs);
	result->lhs = parser.nextExpression(precedence);
	parser.context.expectNext(LTS_TT_IDENTIFIER, "'else'");
	if (parser.context.value().getString() != "else")
		parser.context.error("Expected 'else' here!");
	result->rhs = parser.nextExpression(precedence);
	result->content = Node::Content::AV2_TANC_INLINE_IF_ELSE;
	DEBUGLN("IfElse:DONE!");
	return result;
}

Node::Instance SubExpressionResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving sub-expression...");
	auto const expr = parser.nextExpression(precedence);
	parser.context.expectNext(LTS_TT_CLOSE_PAREN);
	DEBUGLN("SubExpression:DONE!");
	return expr;
}

Node::Instance FunctionCallResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	DEBUGLN("Resolving function call expression...");
	result->base = token;
	result->lhs = lhs;
	result->content = Node::Content::AV2_TANC_FN_CALL;
	if (parser.context.peek().type == LTS_TT_EXCLAMATION) {
		parser.context.next();
		return result;
	}
	while (true) {
		if (parser.context.peek().type == (LTS_TT_CLOSE_PAREN)) {
			parser.context.next();
			break;
		}
		result->children.pushBack(parser.nextExpression(precedence));
		DEBUGLN("Argument: ", result->children.back()->base.text);
		if (parser.context.peek().type == (LTS_TT_CLOSE_PAREN)) {
			DEBUGLN("No more arguments!");
			parser.context.next();
			break;
		}
		parser.context.expectNext(LTS_TT_COMMA);
		if (parser.context.peek().type == LTS_TT_CLOSE_PAREN)
			parser.context.error("Expected expression after the comma!");
	}
	parser.context.expect(LTS_TT_CLOSE_PAREN);
	DEBUGLN("FunctionCall:DONE!");
	return result;
}

Node::Instance BlockResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving block expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	while (true) {
		if (parser.context.has(LTS_TT_CLOSE_CURLY))
			break;
		if (parser.context.peek().type == (LTS_TT_CLOSE_CURLY)) {
			parser.context.next();
			break;
		}
		result->children.pushBack(parser.nextExpression(precedence));
	}
	parser.context.expect(LTS_TT_CLOSE_CURLY);
	result->content = Node::Content::AV2_TANC_BLOCK;
	DEBUGLN("Block:DONE!");
	return result;
}

Node::Instance ArrayResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving array expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	if (lhs)
		result->lhs = lhs;
	while (true) {
		if (parser.context.peek().type == (LTS_TT_CLOSE_BRACKET)) {
			parser.context.next();
			break;
		}
		result->children.pushBack(parser.nextExpression(precedence));
		if (parser.context.peek().type == (LTS_TT_CLOSE_BRACKET)) {
			parser.context.next();
			break;
		}
		parser.context.expect(LTS_TT_COMMA);
		if (parser.context.peek().type == LTS_TT_CLOSE_BRACKET)
			parser.context.error("Expected expression after the comma!");
	}
	parser.context.expect(LTS_TT_CLOSE_BRACKET);
	result->content = lhs ? Node::Content::AV2_TANC_SUBSCRIPT : Node::Content::AV2_TANC_ARRAY;
	DEBUGLN("Array:DONE!");
	return result;
}

Node::Instance BranchResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving branch expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_BRANCH;
	result->value = token.text.toString();
	// TODO: This
	DEBUGLN("Branch:DONE!");
	return result;
}

Node::Instance LoopResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_LOOP;
	result->value = token.text.toString();
	// TODO: This
	return result;
}

Node::Instance ImportResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	DEBUGLN("Resolving import expression...");
	result->base = token;
	result->content = Node::Content::AV2_TANC_IMPORT;
	result->value = token.text.toString();
	result->lhs = lhs;
	DEBUGLN("Follows: ", parser.context.token().text);
	DEBUGLN("Follows: ", parser.context.peek().text);
	if (parser.context.type() == LTS_TT_DOT) {
		DEBUGLN("Here!");
		parser.context.next();
	}
	result->rhs = parser.nextExpression();
	return result;
}

Node::Instance AssignmentResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_ASSIGNMENT;
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

Node::Instance AttributeResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_ATTRIBUTE;
	result->lhs = parser.nextExpression();
	if (!result->lhs)
		parser.context.error("Unexpected end-of-file!");
	if (!(
		result->lhs->isPathOrName()
	||	result->lhs->content == Node::Content::AV2_TANC_ARRAY
	)) parser.context.error("Expected single-attribute or attribute list here!");
	result->rhs = parser.nextExpression();
	return result;
}

Node::Instance FunctionPrototypeResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving function prototype expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_DECLARATION;
		parser.context.expectNext(LTS_TT_OPEN_PAREN);
	while (true) {
		if (parser.context.peek().type == (LTS_TT_CLOSE_PAREN)) {
			parser.context.next();
			break;
		}
		DEBUGLN(">>>>>>>>>>>> Argument");
		result->children.pushBack(parser.nextExpression(precedence));
		DEBUGLN("<<<<<<<<<<<< Follows: ", parser.context.peek().text);
		if (parser.context.peek().type == (LTS_TT_CLOSE_PAREN)) {
			parser.context.next();
			break;
		}
		parser.context.expect(LTS_TT_COMMA);
		if (parser.context.peek().type == LTS_TT_CLOSE_PAREN)
			parser.context.error("Expected expression after the comma!");
	}
	parser.context.expect(LTS_TT_CLOSE_PAREN);
	DEBUGLN("FunctionPrototype:Arguments:DONE!");
	if (parser.context.peek().type == LTS_TT_LITTLE_ARROW) {
		DEBUGLN("Resolving result type...");
		parser.context.next();
		result->lhs = parser.nextExpression();
		DEBUGLN("FunctionPrototype:Result:DONE!");
	}
	DEBUGLN("FunctionPrototype:DONE!");
	return result;
}

Node::Instance VariableDeclResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving variable declaration expression...");
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	result->lhs = lhs;
	result->rhs = parser.nextExpression(precedence);
	DEBUGLN("VariableDecl:DONE!");
	return result;
}

Node::Instance ModuleDeclResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	auto const name = parser.nextExpression();
	if (!name->isPathOrName())
		parser.context.error("Expected path or name here!");
	auto const def = parser.nextExpression();
	if (def->content != Node::Content::AV2_TANC_BLOCK)
		parser.context.error("Expected block expression here!");
	result->lhs = name;
	result->rhs = def;
	return result;
}

Node::Instance TemplateDeclResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	// TODO: This
	return result;
}

Node::Instance StructureDeclResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	auto const name = parser.nextExpression();
	if (!name->isPathOrName())
		parser.context.error("Expected path or name here!");
	auto const def = parser.nextExpression();
	if (def->content != Node::Content::AV2_TANC_BLOCK)
		parser.context.error("Expected block expression here!");
	result->lhs = name;
	result->rhs = def;
	return result;
}

Node::Instance TraitDeclResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	auto const name = parser.nextExpression();
	if (!name->isPathOrName())
		parser.context.error("Expected path or name here!");
	auto const def = parser.nextExpression();
	if (def->content != Node::Content::AV2_TANC_BLOCK)
		parser.context.error("Expected block expression here!");
	result->lhs = name;
	result->rhs = def;
	return result;
}

Node::Instance FunctionDeclResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	FunctionPrototypeResolver resolver;
	result->lhs = lhs;
	result->rhs = resolver.resolve(parser, null, {});
	if (parser.context.peek().type == LTS_TT_BIG_ARROW)
		parser.context.next();
	else if (parser.context.peek().type == LTS_TT_OPEN_CURLY)
		result->rhs = parser.nextExpression();
	else if (parser.context.peek().type != LTS_TT_SEMICOLON)
		parser.context.error("Expected '=>', ';' or '{' here!");
	return result;
}

Node::Instance PropertyDeclResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	// TODO: This
	return result;
}

Node::Instance PathResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_PATH;
	result->lhs = lhs;
	result->rhs = parser.nextExpression(precedence);
	result->base = token;
	if (
		result->rhs->content == Node::Content::AV2_TANC_PATH
	||	result->rhs->content == Node::Content::AV2_TANC_NAME
	) return result;
	parser.context.error("Invalid path expression!");
}

Node::Instance UsingResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	// TODO: This
	return result;
}

Node::Instance DynamicOperatorResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
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
	return result;
}

Node::Instance MainBlockResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->lhs = parser.nextExpression();
	result->content = Node::Content::AV2_TANC_BLOCK;
	if (!result->lhs || result->lhs->content != Node::Content::AV2_TANC_BLOCK)
		parser.context.error("Expected block expression here!");
	return result;
}

Node::Instance DynamicOperatorDeclResolver::resolve(Parser& parser, Node::Instance const& lhs, BaseContext::Axiom const& token) {
	auto const opkey = parser.context.getNext(LTS_TT_IDENTIFIER, "operator name").getString();
	if (
		token.text == "prefix"
	||	token.text == "postfix"
	) {
		Instance<DynamicOperatorResolver> op = new DynamicOperatorResolver(
			token.text == "prefix"
		?	DynamicOperatorResolver::Class::AV2_TA_DORC_PREFIX
		:	DynamicOperatorResolver::Class::AV2_TA_DORC_POSTFIX,
			token.text == "prefix"
		?	decltype(precedence)::AV2_TAPP_PREFIX
		:	decltype(precedence)::AV2_TAPP_POSTFIX,
			false
		);
		if (token.text == "prefix") {
			if (parser.prefixes.contains(opkey))
				parser.context.error("Redeclaration of operator ["+ opkey +"]!");
			parser.add(opkey, parser.prefixes, op.as<AResolver>());
		}
		else {
			if (parser.infixes.contains(opkey))
				parser.context.error("Redeclaration of operator ["+ opkey +"]!");
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
			parser.context.error("Redeclaration of operator ["+ opkey +"]!");
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
	return null;
}

AResolver::~AResolver() {}
