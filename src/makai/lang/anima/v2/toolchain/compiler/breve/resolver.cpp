#include "resolver.hpp"

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;
using Type = Makai::Lexer::CStyle::TokenStream::Token::Type;
using enum Type;

Node::Instance EmptyResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_EMPTY;
	return result;
}

Node::Instance DirectResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
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

Node::Instance PrefixResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving prefix expression [", token.text, "]...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->leftSide = parser.nextExpression(precedence);
	result->content = Node::Content::AV2_TANC_PREFIX_OP;
	DEBUGLN("Prefix:DONE!");
	return result;
}

Node::Instance InfixResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving infix expression [", token.text, "]...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->leftSide = leftSide;
	result->rightSide = parser.nextExpression(precedence);
	result->content = Node::Content::AV2_TANC_INFIX_OP;
	DEBUGLN("Infix:DONE!");
	return result;
}

Node::Instance PostfixResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving postfix expression [", token.text, "]...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->leftSide = leftSide;
	result->content = Node::Content::AV2_TANC_POSTFIX_OP;
	DEBUGLN("Postfix:DONE!");
	return result;
}

Node::Instance InlineMinimaResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
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

Node::Instance InlineIfElseResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving inline if-else expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->middle= leftSide;
	result->leftSide = parser.nextExpression(precedence);
	parser.context.expectNext(LTS_TT_IDENTIFIER, "'else'");
	if (parser.context.value().getString() != "else")
		parser.context.error("Expected 'else' here!");
	result->rightSide = parser.nextExpression(precedence);
	result->content = Node::Content::AV2_TANC_INLINE_IF_ELSE;
	DEBUGLN("IfElse:DONE!");
	return result;
}

Node::Instance SubExpressionResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving sub-expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_BLOCK;
	while (true) {
		if (parser.context.peek().type == (LTS_TT_CLOSE_PAREN)) {
			parser.context.next();
			break;
		}
		result->children.pushBack(parser.nextExpression(precedence));
		if (parser.context.peek().type == (LTS_TT_CLOSE_PAREN)) {
			parser.context.next();
			break;
		}
		parser.context.expectNext(LTS_TT_COMMA);
		if (parser.context.peek().type == LTS_TT_CLOSE_PAREN)
			parser.context.error("Expected expression after the comma!");
	}
	parser.context.expect(LTS_TT_CLOSE_PAREN);
	DEBUGLN("SubExpression:DONE!");
	return result;
}

Node::Instance FunctionCallResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	DEBUGLN("Resolving function call expression...");
	result->base = token;
	result->leftSide = leftSide;
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
		result->children.pushBack(parser.nextExpression());
		DEBUGLN(":::::: Argument: ", result->children.back()->base.text);
		DEBUGLN(":::::: Followup: ", parser.context.peek().text);
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

Node::Instance BlockResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving block expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	while (true) {
		if (parser.context.peek().type == (LTS_TT_CLOSE_CURLY)) {
			parser.context.next();
			break;
		}
		DEBUGLN("!!!!!!!!!!!!!!!!!!!!!! Resolving block statement...");
		result->children.pushBack(parser.nextExpression());
		DEBUGLN("!!!!!!!!!!!!!!!!!!!!!! Block:Statement:DONE!");
		if (parser.context.peek().type == (LTS_TT_CLOSE_CURLY)) {
			parser.context.next();
			break;
		}
	}
	parser.context.expect(LTS_TT_CLOSE_CURLY);
	result->content = Node::Content::AV2_TANC_BLOCK;
	DEBUGLN("Block:DONE!");
	return result;
}

Node::Instance ArrayResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving array expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	if (leftSide)
		result->leftSide = leftSide;
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
	result->content = leftSide ? Node::Content::AV2_TANC_SUBSCRIPT : Node::Content::AV2_TANC_ARRAY;
	DEBUGLN("Array:DONE!");
	return result;
}

Node::Instance BranchResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving branch expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_BRANCH;
	result->value = token.text.toString();
	// TODO: This
	DEBUGLN("Branch:DONE!");
	return result;
}

Node::Instance LoopResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_LOOP;
	result->value = token.text.toString();
	// TODO: This
	return result;
}

Node::Instance ImportResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	DEBUGLN("Resolving import expression...");
	result->base = token;
	result->content = Node::Content::AV2_TANC_IMPORT;
	result->value = token.text.toString();
	result->leftSide = leftSide;
	DEBUGLN("Follows: ", parser.context.token().text);
	DEBUGLN("Follows: ", parser.context.peek().text);
	if (parser.context.type() == LTS_TT_DOT) {
		DEBUGLN("Here!");
		parser.context.next();
	}
	result->rightSide = parser.nextExpression();
	return result;
}

Node::Instance AssignmentResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_ASSIGNMENT;
	result->leftSide = leftSide;
	result->rightSide = parser.nextExpression(precedence);
	return result;
}

Node::Instance ExtensionResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_TYPE_EXTENSION;
	result->leftSide = parser.nextExpression();
	if (!leftSide->isPathOrName())
		parser.context.error("Invalid expression for extension!");
	if (parser.context.peek().type == LTS_TT_IDENTIFIER) {
		auto const id = parser.context.peek().value.getString();
		if (id == "with")
			parser.context.next();
	}
	result->rightSide = parser.nextExpression();
	if (!result->rightSide->isDeclarationOrBlock())
		parser.context.error("Expected declaration or code block here!");
	return result;
}

Node::Instance AttributeResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_ATTRIBUTE;
	result->leftSide = parser.nextExpression();
	if (!result->leftSide)
		parser.context.error("Unexpected end-of-file!");
	if (!(
		result->leftSide->isPathOrName()
	||	result->leftSide->content == Node::Content::AV2_TANC_ARRAY
	||	result->leftSide->content == Node::Content::AV2_TANC_FN_CALL
	)) parser.context.error("Expected single-attribute or attribute list here!");
	result->rightSide = parser.nextExpression();
	return result;
}

Node::Instance FunctionPrototypeResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
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
		result->leftSide = parser.nextExpression();
		DEBUGLN("FunctionPrototype:Result:DONE!");
	}
	DEBUGLN("FunctionPrototype:DONE!");
	return result;
}

Node::Instance VariableDeclResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving variable declaration expression...");
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	result->leftSide = leftSide;
	if (token.type == LTS_TT_ASSIGN)
		result->children.pushBack(parser.nextExpression());
	else {
		auto const v = parser.nextExpression(precedence);
		if (v->content == Node::Content::AV2_TANC_ASSIGNMENT) {
			result->middle = v->leftSide;
			result->rightSide = v->rightSide;
		}
	}
	DEBUGLN("VariableDecl:DONE!");
	return result;
}

Node::Instance TemplateDeclResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	// TODO: This
	return result;
}

Node::Instance NamedBlockDeclResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	auto const name = parser.nextExpression();
	if (optionalName && name->isBlock()) {
		result->rightSide = name;
		return result;
	} else if (!name->isPathOrName())
		parser.context.error("Expected path or name here!");
	auto const def = parser.nextExpression();
	if (def->content != Node::Content::AV2_TANC_BLOCK)
		parser.context.error("Expected block expression here!");
	result->leftSide = name;
	result->rightSide = def;
	return result;
}

Node::Instance FunctionDeclResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	DEBUGLN("Resolving function declaration expression...");
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	FunctionPrototypeResolver resolver;
	result->leftSide = leftSide;
	result->middle = resolver.resolve(parser, null, {});
	if (parser.context.peek().type == LTS_TT_BIG_ARROW) {
		parser.context.next();
		result->rightSide = parser.nextExpression();
	}
	else if (parser.context.peek().type == LTS_TT_OPEN_CURLY)
		result->rightSide = parser.nextExpression();
	/*else if (parser.context.peek().type == LTS_TT_SEMICOLON)
		parser.context.next();
	else parser.context.error("Expected '=>', ';' or '{' here!");*/
	DEBUGLN("FunctionDecl:DONE!");
	return result;
}

Node::Instance PropertyDeclResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	// TODO: This
	return result;
}

Node::Instance PathResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_PATH;
	result->leftSide = leftSide;
	result->rightSide = parser.nextExpression(precedence);
	result->base = token;
	if (
		result->rightSide->content == Node::Content::AV2_TANC_PATH
	||	result->rightSide->content == Node::Content::AV2_TANC_NAME
	) return result;
	parser.context.error("Invalid path expression!");
}

Node::Instance UsingResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	// TODO: This
	return result;
}

Node::Instance DynamicOperatorResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	switch (opClass) {
		case Class::AV2_TA_DORC_PREFIX: {
			result->leftSide = parser.nextExpression(precedence);
			result->content = Node::Content::AV2_TANC_PREFIX_OP;
		}
		case Class::AV2_TA_DORC_INFIX: {
			result->leftSide = leftSide;
			result->rightSide = parser.nextExpression(precedence);
			result->content = Node::Content::AV2_TANC_INFIX_OP;
		}
		case Class::AV2_TA_DORC_POSTFIX: {
			result->leftSide = leftSide;
			result->content = Node::Content::AV2_TANC_POSTFIX_OP;
		}
	}
	return result;
}

Node::Instance MainBlockResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->leftSide = parser.nextExpression();
	result->content = Node::Content::AV2_TANC_BLOCK;
	if (!(result->leftSide && result->leftSide->isBlock()))
		parser.context.error("Expected block expression here!");
	return result;
}

Node::Instance DynamicOperatorDeclResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
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
