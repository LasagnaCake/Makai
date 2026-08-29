#include "resolver.hpp"

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;
using Type = Makai::Lexer::CStyle::TokenStream::Token::Type;
using enum Type;

Node::Instance EmptyResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	MAKAILIB_DEBUGLN_FULL("Resolving empty expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_EMPTY;
	MAKAILIB_DEBUGLN_FULL("Empty:DONE!");
	return result;
}

Node::Instance EnumResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	MAKAILIB_DEBUGLN_FULL("Resolving enumeration expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_DECLARATION;
	auto const decl = parser.nextExpression();
	MAKAILIB_DEBUGLN_FULL("Enum has type? ", decl->base.type == LTS_TT_COLON);
	if (decl->base.type == LTS_TT_COLON) {
		result->leftSide = decl->leftSide;
		result->middle = decl->rightSide ? decl->rightSide : decl->middle;
	} else result->leftSide = decl;
	result->rightSide = parser.nextExpression();
	MAKAILIB_DEBUGLN_FULL("Enum:DONE!");
	return result;
}

Node::Instance DirectResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	MAKAILIB_DEBUGLN_FULL("Resolving direct expression...");
	MAKAILIB_DEBUGLN_FULL("######## Value: ", token.value ? token.value.toString() : token.text.toString());
	auto isIdentifier = parser.context.type() == LTS_TT_IDENTIFIER;
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->value = token.value;
	result->content = isIdentifier ? Node::Content::AV2_TANC_NAME : Node::Content::AV2_TANC_VALUE;
	MAKAILIB_DEBUGLN_FULL("Direct:DONE!");
	return result;
}

Node::Instance SpecialDirectResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	MAKAILIB_DEBUGLN_FULL("Resolving special direct expression...");
	MAKAILIB_DEBUGLN_FULL("######## Value: ", token.text);
	Node::Instance result = Node::Instance::create();
	result->base = token;
	auto const id = token.text;
	if (id == "true")		result->value = true;
	else if (id == "false")	result->value = false;
	else if (id == "null")	result->value = null;
	result->content = Node::Content::AV2_TANC_VALUE;
	MAKAILIB_DEBUGLN_FULL("Direct:DONE!");
	return result;
}

Node::Instance PrefixResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	MAKAILIB_DEBUGLN_FULL("Resolving prefix expression [", token.text, "]...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->leftSide = parser.nextExpression(precedence);
	result->content = Node::Content::AV2_TANC_PREFIX_OP;
	MAKAILIB_DEBUGLN_FULL("Prefix:DONE!");
	return result;
}

Node::Instance ExitResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_EXIT;
	return result;
}

Node::Instance InfixResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	MAKAILIB_DEBUGLN_FULL("Resolving infix expression [", token.text, "]...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->leftSide = leftSide;
	result->rightSide = parser.nextExpression(precedence);
	result->content = Node::Content::AV2_TANC_INFIX_OP;
	MAKAILIB_DEBUGLN_FULL("Infix:DONE!");
	return result;
}

Node::Instance PostfixResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	MAKAILIB_DEBUGLN_FULL("Resolving postfix expression [", token.text, "]...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->leftSide = leftSide;
	result->content = Node::Content::AV2_TANC_POSTFIX_OP;
	MAKAILIB_DEBUGLN_FULL("Postfix:DONE!");
	return result;
}

Node::Instance NullableDeclResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	MAKAILIB_DEBUGLN_FULL("Resolving nullable declaration expression [", token.text, "]...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->leftSide = leftSide;
	result->content = Node::Content::AV2_TANC_NULLABLE_DECL;
	MAKAILIB_DEBUGLN_FULL("NullableDeclResolver:DONE!");
	return result;
}

Node::Instance InlineMinimaResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	MAKAILIB_DEBUGLN_FULL("Resolving inline assembly expression...");
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_INLINE_MINIMA;
	result->base = token;
	parser.context.expectNext(LTS_TT_OPEN_CURLY).next();
	while (!parser.context.has(LTS_TT_CLOSE_CURLY)) {
		result->interject.pushBack(parser.context.token());
		parser.context.next();
	}
	MAKAILIB_DEBUGLN_FULL("Assembly:DONE!");
	return result;
}

Node::Instance InlineIfElseResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	MAKAILIB_DEBUGLN_FULL("Resolving inline if-else expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->leftSide = leftSide;
	result->middle = parser.nextExpression(precedence);
	parser.context.expectNext(LTS_TT_IDENTIFIER, "'else'");
	if (parser.context.value().getString() != "else")
		parser.context.error("Expected 'else' here!");
	result->rightSide = parser.nextExpression(precedence);
	result->content = Node::Content::AV2_TANC_INLINE_IF_ELSE;
	MAKAILIB_DEBUGLN_FULL("<inline-if-else>");
	MAKAILIB_DEBUGLN_FULL("Condition: ", result->middle->base.text);
	MAKAILIB_DEBUGLN_FULL("If-True: ", result->leftSide->base.text);
	MAKAILIB_DEBUGLN_FULL("If-False: ", result->rightSide->base.text);
	MAKAILIB_DEBUGLN_FULL("</inline-if-else>");
	MAKAILIB_DEBUGLN_FULL("IfElse:DONE!");
	return result;
}

Node::Instance SubExpressionResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	MAKAILIB_DEBUGLN_FULL("Resolving sub-expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_BLOCK;
	while (true) {
		if (parser.context.peek().type == (LTS_TT_CLOSE_PAREN)) {
			parser.context.next();
			break;
		}
		result->children.pushBack(parser.nextExpression());
		if (parser.context.peek().type == (LTS_TT_CLOSE_PAREN)) {
			parser.context.next();
			break;
		}
		parser.context.expectNext(LTS_TT_COMMA);
		if (parser.context.peek().type == LTS_TT_CLOSE_PAREN)
			parser.context.error("Expected expression after the comma!");
	}
	parser.context.expect(LTS_TT_CLOSE_PAREN);
	MAKAILIB_DEBUGLN_FULL("SubExpression:DONE!");
	if (parser.context.peek().type == (LTS_TT_LITTLE_ARROW)) {
		result->content = Node::Content::AV2_TANC_FN_PROTOTYPE;
		result->leftSide = parser.nextExpression();
	}
	return result;
}

Node::Instance FunctionCallResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	MAKAILIB_DEBUGLN_FULL("Resolving function call expression...");
	if (!leftSide)
		parser.context.error("["+Makai::toString(__LINE__)+"] INTERNAL_ERROR :: Uh oh, hehe :D");
	result->base = token;
	result->leftSide = leftSide;
	result->content = Node::Content::AV2_TANC_FN_CALL;
	if (token.type != LTS_TT_OPEN_PAREN)
		return result;
	while (true) {
		if (parser.context.peek().type == (LTS_TT_CLOSE_PAREN)) {
			parser.context.next();
			break;
		}
		result->children.pushBack(parser.nextExpression());
		if (!result->children.back())
			parser.context.error("[" + Makai::toString(__LINE__) + "] INTERNAL_ERROR :: Oops :/");
		MAKAILIB_DEBUGLN_FULL(":::::: Argument: ", result->children.back()->base.text);
		MAKAILIB_DEBUGLN_FULL(":::::: Followup: ", parser.context.peek().text);
		if (parser.context.peek().type == (LTS_TT_CLOSE_PAREN)) {
			MAKAILIB_DEBUGLN_FULL("No more arguments!");
			parser.context.next();
			break;
		}
		parser.context.expectNext(LTS_TT_COMMA);
		if (parser.context.peek().type == LTS_TT_CLOSE_PAREN)
			parser.context.error("Expected expression after the comma!");
	}
	parser.context.expect(LTS_TT_CLOSE_PAREN);
	MAKAILIB_DEBUGLN_FULL("FunctionCall:DONE!");
	return result;
}

Node::Instance BlockResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	MAKAILIB_DEBUGLN_FULL("Resolving block expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	while (true) {
		if (parser.context.peek().type == (LTS_TT_CLOSE_CURLY)) {
			parser.context.next();
			break;
		}
		MAKAILIB_DEBUGLN_FULL("!!!!!!!!!!!!!!!!!!!!!! Resolving block statement...");
		result->children.pushBack(parser.nextExpression());
		MAKAILIB_DEBUGLN_FULL("!!!!!!!!!!!!!!!!!!!!!! Block:Statement:DONE!");
		if (parser.context.peek().type == (LTS_TT_CLOSE_CURLY)) {
			parser.context.next();
			break;
		}
	}
	parser.context.expect(LTS_TT_CLOSE_CURLY);
	result->content = Node::Content::AV2_TANC_BLOCK;
	MAKAILIB_DEBUGLN_FULL("Block:DONE!");
	return result;
}

Node::Instance ArrayResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	MAKAILIB_DEBUGLN_FULL("Resolving array expression...");
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
		parser.context.expectNext(LTS_TT_COMMA);
		if (parser.context.peek().type == LTS_TT_CLOSE_BRACKET)
			parser.context.error("Expected expression after the comma!");
	}
	if (result->children.size())
		result->rightSide = result->children.front();
	parser.context.expect(LTS_TT_CLOSE_BRACKET);
	result->content = leftSide ? Node::Content::AV2_TANC_SUBSCRIPT : Node::Content::AV2_TANC_ARRAY;
	MAKAILIB_DEBUGLN_FULL("Array:DONE!");
	return result;
}

Node::Instance BranchResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	MAKAILIB_DEBUGLN_FULL("Resolving branch expression...");
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_BRANCH;
	result->value = token.text.toString();
	result->middle		= parser.nextExpression();
	result->leftSide	= parser.nextExpression();
	if (parser.context.peek().text == "else") {
		parser.context.next();
		result->rightSide = parser.nextExpression();
	}
	MAKAILIB_DEBUGLN_FULL("Branch:DONE!");
	return result;
}

Node::Instance LoopResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_LOOP;
	result->value = null;
	if (token.text == "do") {
		result->rightSide	= parser.nextExpression();
		if (parser.context.peek().text == "while") {
			parser.context.next();
			result->leftSide = parser.nextExpression();
		}
	} else if (token.text == "for") {
		result->leftSide	= parser.nextExpression();
		parser.context.expectNext(LTS_TT_BIG_ARROW);
		result->middle = parser.nextExpression();
		result->rightSide	= parser.nextExpression();
	} else if (token.text == "repeat") {
		result->leftSide	= parser.nextExpression();
		if (parser.context.peek().type == LTS_TT_BIG_ARROW) {
			result->middle = result->leftSide;
			parser.context.next();
			result->leftSide = parser.nextExpression();
		}
		result->rightSide	= parser.nextExpression();
	} else {
		result->leftSide	= parser.nextExpression();
		result->rightSide	= parser.nextExpression();
	}
	return result;
}

Node::Instance ImportResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	MAKAILIB_DEBUGLN_FULL("Resolving import expression...");
	result->base = token;
	result->content = Node::Content::AV2_TANC_IMPORT;
	result->value = token.text.toString();
	MAKAILIB_DEBUGLN_FULL("Follows: ", parser.context.token().text);
	MAKAILIB_DEBUGLN_FULL("Follows: ", parser.context.peek().text);
	if (parser.context.type() == LTS_TT_DOT) {
		MAKAILIB_DEBUGLN_FULL("Here!");
		parser.context.next();
	}
	result->leftSide = parser.nextExpression();
	return result;
}

Node::Instance AssignmentResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_ASSIGNMENT;
	if (leftSide->content == Node::Content::AV2_TANC_SUBSCRIPT) {
		result->leftSide = leftSide->leftSide;
		result->middle = leftSide->rightSide;
	} if (leftSide->content == Node::Content::AV2_TANC_PATH) {
		result->leftSide = leftSide;
		result->forAssignment = true;
	} else result->leftSide = leftSide;
	result->rightSide = parser.nextExpression(precedence);
	return result;
}

Node::Instance ExtensionResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_TYPE_EXTENSION;
	result->leftSide = parser.nextExpression();
	if (!result->leftSide->isPathOrName())
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
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_DECLARATION;
	if (leftSide) {
		if (
			leftSide->content == Node::Content::AV2_TANC_NAME
		or	(leftSide->content == Node::Content::AV2_TANC_DECLARATION && leftSide->base.type == LTS_TT_COMMA)
		or	(leftSide->content == Node::Content::AV2_TANC_ASSIGNMENT && leftSide->base.type == LTS_TT_EQUALS)
		) result->children.pushBack(leftSide);
		else if (leftSide->content == Node::Content::AV2_TANC_BLOCK && leftSide->base.type == LTS_TT_OPEN_PAREN)
			result->children = leftSide->children;
		else parser.context.error("Expected parenthesized arguments here!");
		result->leftSide = parser.nextExpression();
		return result;
	}
	MAKAILIB_DEBUGLN_FULL("Resolving function prototype expression...");
	parser.context.expectNext(LTS_TT_OPEN_PAREN);
	while (true) {
		if (parser.context.peek().type == (LTS_TT_CLOSE_PAREN)) {
			parser.context.next();
			break;
		}
		MAKAILIB_DEBUGLN_FULL(">>>>>>>>>>>> Argument");
		result->children.pushBack(parser.nextExpression(precedence));
		MAKAILIB_DEBUGLN_FULL("<<<<<<<<<<<< Follows: ", parser.context.peek().text);
		if (parser.context.peek().type == (LTS_TT_CLOSE_PAREN)) {
			parser.context.next();
			break;
		}
		parser.context.expectNext(LTS_TT_COMMA);
		if (parser.context.peek().type == LTS_TT_CLOSE_PAREN)
			parser.context.error("Expected expression after the comma!");
	}
	parser.context.expect(LTS_TT_CLOSE_PAREN);
	MAKAILIB_DEBUGLN_FULL("FunctionPrototype:Arguments:DONE!");
	if (parser.context.peek().type == LTS_TT_LITTLE_ARROW) {
		MAKAILIB_DEBUGLN_FULL("Resolving result type...");
		parser.context.next();
		result->leftSide = parser.nextExpression();
		MAKAILIB_DEBUGLN_FULL("FunctionPrototype:Result:DONE!");
	}
	MAKAILIB_DEBUGLN_FULL("FunctionPrototype:DONE!");
	return result;
}

Node::Instance VariableDeclResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	MAKAILIB_DEBUGLN_FULL("Resolving variable declaration expression...");
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	result->leftSide = leftSide;
	if (token.type == LTS_TT_ASSIGN)
		result->rightSide = parser.nextExpression();
	else if (token.type == LTS_TT_DECLARE)
		result->rightSide = parser.nextExpression();
	else {
		auto const v = parser.nextExpression(precedence);
		if (v->content == Node::Content::AV2_TANC_ASSIGNMENT) {
			result->middle = v->leftSide;
			result->rightSide = v->rightSide;
		} else if (
			v->content == Node::Content::AV2_TANC_RANGE
		or	v->content == Node::Content::AV2_TANC_EXPANSION
		) {
			result->content = Node::Content::AV2_TANC_ITERATION;
			result->middle = v->leftSide;
			result->rightSide = v->rightSide;
		} else {
			result->middle = v;
		}
	}
	MAKAILIB_DEBUGLN_FULL("VariableDecl:DONE!");
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
	MAKAILIB_DEBUGLN_FULL("Resolving named block expression...");
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	auto name = parser.nextExpression();
	if (optionalName && name->isBlock()) {
		result->rightSide = name;
		MAKAILIB_DEBUGLN_FULL("NamedBlock:DONE!");
		return result;
	} else if (canInherit && name->content == Node::Content::AV2_TANC_DECLARATION) {
		if (name->base.text != ":")
			parser.context.error("Invalid inheritance expression!");
		MAKAILIB_DEBUGLN_FULL("+++++++++++++++ DECL::LHS is ", Node::asString(name->leftSide->content));
		MAKAILIB_DEBUGLN_FULL("+++++++++++++++ DECL::LHS = ", name->leftSide->base.text);
		MAKAILIB_DEBUGLN_FULL("+++++++++++++++ DECL::MHS is ", Node::asString(name->middle->content));
		MAKAILIB_DEBUGLN_FULL("+++++++++++++++ DECL::MHS = ", name->middle->base.text);
		if (!name->leftSide->isPathOrName())
			parser.context.error("Expected name or path here!");
		result->middle = name->middle;
		result->leftSide = name->leftSide;
	} else if (!name->isPathOrName())
		parser.context.error("Expected path or name here!");
	else result->leftSide = name;
	auto const def = parser.nextExpression();
	if (def->content != Node::Content::AV2_TANC_BLOCK)
		parser.context.error("Expected block expression here!");
	result->rightSide = def;
	MAKAILIB_DEBUGLN_FULL("NamedBlock:DONE!");
	return result;
}

Node::Instance FunctionDeclResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	MAKAILIB_DEBUGLN_FULL("Resolving function declaration expression...");
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	FunctionPrototypeResolver resolver;
	result->leftSide = leftSide;
	result->middle = resolver.resolve(parser, null, {});
	result->rightSide = FunctionContentResolver().resolve(parser, null, {});
	MAKAILIB_DEBUGLN_FULL("FunctionDecl:DONE!");
	return result;
}

static Node::Instance protoName(Makai::String const& name) {
	Node::Instance pname = Node::Instance::create();
	pname->content = Node::Content::AV2_TANC_NAME;
	pname->value = "__" + name;
	return pname;
}

Node::Instance PropertyDeclResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_DECLARATION;
	result->base = token;
	result->middle = parser.nextExpression();
	if (!result->middle->isPathOrName())
		parser.context.error("Expected path or name here!");
	auto const tok = parser.context.peek();
	if (tok.text == "get" || tok.text == "set") {
		parser.context.next();
		if (tok.text == "set")
			result->rightSide = SetterResolver().resolve(parser, null, tok);
		else result->leftSide = GetterResolver().resolve(parser, null, tok);
	} else if (tok.type == LTS_TT_OPEN_CURLY) {
		parser.context.expectNext(LTS_TT_OPEN_CURLY);
		while (true) {
			if (parser.context.peek().type == (LTS_TT_CLOSE_CURLY)) {
				parser.context.next();
				break;
			}
			auto const expr = parser.nextExpression();
			if (expr->content == Node::Content::AV2_TANC_PROPERTY_GETTER) {
				if (result->leftSide)
					parser.context.error("Redeclaration of property getter!");
				result->leftSide = expr;
			} else if (expr->content == Node::Content::AV2_TANC_PROPERTY_SETTER) {
				if (result->rightSide)
					parser.context.error("Redeclaration of property setter!");
				result->rightSide = expr;
			} else parser.context.error("Expected property getter or setter here!");
			auto const next = parser.context.peek();
			if (parser.context.peek().type == (LTS_TT_CLOSE_CURLY)) {
				parser.context.next();
				break;
			}
		}
		if (!(result->leftSide or result->rightSide))
			parser.context.error("Expected getter or setter!");
	} else parser.context.error("Invalid property declaration!");
	return result;
}

Node::Instance GetterResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	FunctionPrototypeResolver resolver;
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_PROPERTY_GETTER;
	result->base = token;
	result->leftSide = protoName("get" + result->name());
	result->middle = FunctionPrototypeResolver().resolve(parser, null, {});
	result->rightSide = FunctionContentResolver().resolve(parser, null, {});
	return result;
}

Node::Instance SetterResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	FunctionPrototypeResolver resolver;
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_PROPERTY_SETTER;
	result->base = token;
	result->leftSide = protoName("set" + result->name());
	result->middle = FunctionPrototypeResolver().resolve(parser, null, {});
	result->rightSide = FunctionContentResolver().resolve(parser, null, {});
	return result;
}

Node::Instance FunctionContentResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	if (parser.context.peek().type == LTS_TT_BIG_ARROW) {
		parser.context.next();
		return parser.nextExpression();
	} else if (parser.context.peek().type == LTS_TT_OPEN_CURLY)
		return parser.nextExpression();
	else return null;
}


Node::Instance PathResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = token.type == LTS_TT_NULL_ACCESS ? Node::Content::AV2_TANC_FAILABLE_PATH : Node::Content::AV2_TANC_PATH;
	result->leftSide = leftSide;
	result->value = "/" + parser.context.getNext(LTS_TT_IDENTIFIER, "name").getString();
	result->base = token;
	MAKAILIB_DEBUGLN_FULL("Path Expression {");
	MAKAILIB_DEBUGLN_FULL("  LHS: ", result->leftSide->base.text);
	MAKAILIB_DEBUGLN_FULL("  Subpath: ", result->value.getString());
	MAKAILIB_DEBUGLN_FULL("}");
	return result;
}

Node::Instance UsingResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	auto const decl = parser.nextExpression();
	if (
		decl->content == Node::Content::AV2_TANC_PATH
	or	decl->content == Node::Content::AV2_TANC_IMPORT
	) {
		result->content = Node::Content::AV2_TANC_UNSCOPING;
		result->leftSide = decl;
	} else if (decl->content == Node::Content::AV2_TANC_ASSIGNMENT) {
		result->content = Node::Content::AV2_TANC_ALIAS;
		result->leftSide = decl->leftSide;
		result->rightSide = decl->rightSide;
	} else parser.context.error("Invalid alias expression!");
	return result;
}

Node::Instance EmptyDecayResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->content = Node::Content::AV2_TANC_EMPTY_DECAY;
	result->base = token;
	result->leftSide = leftSide;
	result->rightSide = parser.nextExpression(precedence);
	return result;
}

Node::Instance DynamicOperatorResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	switch (opClass) {
		case Class::AV2_TA_DORC_PREFIX: {
			result->leftSide = parser.nextExpression(precedence);
			result->content = Node::Content::AV2_TANC_CUSTOM_PREFIX_OP;
		}
		case Class::AV2_TA_DORC_INFIX: {
			result->leftSide = leftSide;
			result->rightSide = parser.nextExpression(precedence);
			result->content = Node::Content::AV2_TANC_CUSTOM_INFIX_OP;
		}
		case Class::AV2_TA_DORC_POSTFIX: {
			result->leftSide = leftSide;
			result->content = Node::Content::AV2_TANC_CUSTOM_POSTFIX_OP;
		}
	}
	return result;
}

Node::Instance DynamicOperatorDeclResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	auto const opkey = parser.context.expectNext(LTS_TT_IDENTIFIER, "operator name").token().text;
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

Node::Instance DropExpressionResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_DROP;
	result->leftSide = parser.nextExpression();

	return result;
}

Node::Instance CreateExpressionResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_NEW;
	result->leftSide = parser.nextExpression();
	auto const next = parser.context.peek().type;
	if (result->leftSide->content == Node::Content::AV2_TANC_SUBSCRIPT) {
		auto const sz = result->leftSide->rightSide;
		auto const type = result->leftSide->leftSide;
		result->leftSide = nullptr;
		result->leftSide = type;
		result->rightSide = sz;
	}
	if (
		next == LTS_TT_OPEN_CURLY
//	or	next == LTS_TT_OPEN_PAREN
	) {
		parser.context.next();
		while (true) {
			if (parser.context.peek().type == (LTS_TT_CLOSE_CURLY)) {
				parser.context.next();
				break;
			}
			result->children.pushBack(parser.nextExpression());
			auto const next = parser.context.peek();
			if (parser.context.peek().type == (LTS_TT_CLOSE_CURLY)) {
				parser.context.next();
				break;
			}
			parser.context.expectNext(LTS_TT_COMMA);
			if (parser.context.peek().type == LTS_TT_CLOSE_CURLY)
				parser.context.error("Expected expression after the comma!");
		}
	}
	return result;
}

Node::Instance AwaitExpressionResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_AWAIT;
	auto const next = parser.context.peek();
	if (
		next.text == "yield"
	or	next.text == "sync"
	) {
		result->base = parser.context.expectNext(LTS_TT_IDENTIFIER).token();
		parser.context.expectNext(LTS_TT_OPEN_CURLY);
		while (true) {
			if (parser.context.peek().type == (LTS_TT_CLOSE_CURLY)) {
				parser.context.next();
				break;
			}
			result->children.pushBack(parser.nextExpression());
			auto const next = parser.context.peek();
			if (parser.context.peek().type == (LTS_TT_CLOSE_CURLY)) {
				parser.context.next();
				break;
			}
			if (parser.context.peek().type == LTS_TT_CLOSE_CURLY)
				parser.context.error("Expected expression after the comma!");
		}
	} else result->leftSide = parser.nextExpression();
	return result;
}

Node::Instance RangeResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_RANGE;
	result->leftSide	= leftSide;
	result->rightSide	= parser.nextExpression(precedence);
	return result;
}

Node::Instance ExpansionResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_EXPANSION;
	result->leftSide	= parser.nextExpression();
	return result;
}

Node::Instance InsertionResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_STREAM_EXPR;
	result->leftSide = leftSide;
	result->rightSide = parser.nextExpression();
	return result;
}

Node::Instance ExtractionResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_STREAM_EXPR;
	result->leftSide = leftSide;
	result->rightSide = parser.nextExpression();
	return result;
}

Node::Instance LambdaResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_LAMBDA;
	if (
		leftSide->content == Node::Content::AV2_TANC_NAME
	or	(leftSide->content == Node::Content::AV2_TANC_DECLARATION && leftSide->base.type == LTS_TT_COMMA)
	or	(leftSide->content == Node::Content::AV2_TANC_ASSIGNMENT && leftSide->base.type == LTS_TT_EQUALS)
	) result->children.pushBack(leftSide);
	else if (leftSide->content == Node::Content::AV2_TANC_BLOCK && leftSide->base.type == LTS_TT_OPEN_PAREN)
		result->children.appendBack(leftSide->children);
	else if (leftSide->content == Node::Content::AV2_TANC_DECLARATION && leftSide->base.type == LTS_TT_LITTLE_ARROW) {
		result->children.appendBack(leftSide->children);
		result->leftSide = leftSide->leftSide;
	} else parser.context.error("Invalid left-side expression for lambda!");
	result->rightSide = parser.nextExpression();
	return result;
}

Node::Instance EvaluationResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_EVAL_BLOCK;
	result->leftSide = parser.nextExpression();
	return result;
}

Node::Instance SwitchResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_SWITCH;
	result->leftSide = parser.nextExpression();
	parser.context.expectNext(LTS_TT_OPEN_CURLY);
	while (true) {
		if (parser.context.peek().type == (LTS_TT_CLOSE_CURLY)) {
			parser.context.next();
			break;
		}
		auto const caseDecl = Node::Instance::create();
		caseDecl->leftSide = parser.nextExpression();
		parser.context.expectNext(LTS_TT_BIG_ARROW);
		if (parser.context.peek().type == (LTS_TT_CLOSE_CURLY))
			parser.context.error("Missing case statement!");
		caseDecl->rightSide = parser.nextExpression();
		result->children.pushBack(caseDecl);
		if (parser.context.peek().type == (LTS_TT_CLOSE_CURLY)) {
			parser.context.next();
			break;
		}
	}
	if (result->children.size() < 2)
		parser.context.error("Switch statements must have at least two cases!");
	return result;
}

Node::Instance MatchResolver::resolve(Parser& parser, Node::Instance const& leftSide, BaseContext::Axiom const& token) {
	Node::Instance result = Node::Instance::create();
	result->base = token;
	result->content = Node::Content::AV2_TANC_SWITCH;
	if (parser.context.peek().type != LTS_TT_OPEN_CURLY)
		result->leftSide = parser.nextExpression();
	parser.context.expectNext(LTS_TT_OPEN_CURLY);
	while (true) {
		if (parser.context.peek().type == (LTS_TT_CLOSE_CURLY)) {
			parser.context.next();
			break;
		}
		auto const caseDecl = Node::Instance::create();
		caseDecl->leftSide = parser.nextExpression();
		parser.context.expectNext(LTS_TT_BIG_ARROW);
		if (parser.context.peek().type == (LTS_TT_CLOSE_CURLY))
			parser.context.error("Missing case statement!");
		caseDecl->rightSide = parser.nextExpression();
		result->children.pushBack(caseDecl);
		if (parser.context.peek().type == (LTS_TT_CLOSE_CURLY)) {
			parser.context.next();
			break;
		}
	}
	return result;
}

AResolver::AResolver(Parser::Precedence const precedence, bool const rightToLeft):
	precedence(Cast::as<Parser::Precedence>(enumcast(precedence) - !rightToLeft)),
	rightToLeft(rightToLeft) {
}
