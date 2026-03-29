#include "transformer.hpp"

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;
using namespace Transformer;

using Type = BaseContext::Tokenizer::Token::Type;

using enum BaseContext::Tokenizer::Token::Type;

static Makai::UTF8String bopName(ATransformer::Context& context, Node::Instance const& node) {
	switch (node->base.type) {
		case LTS_TT_SINGLE_QUOTE_STRING:
		case LTS_TT_DOUBLE_QUOTE_STRING:
		case LTS_TT_BACKTICK_STRING:
		case LTS_TT_FR_SINGLE_QUOTE_STRING:
		case LTS_TT_FR_DOUBLE_QUOTE_STRING:
		case LTS_TT_JP_SINGLE_QUOTE_STRING:
		case LTS_TT_JP_DOUBLE_QUOTE_STRING:
		case LTS_TT_IDENTIFIER:				return node->base.text;
		case LTS_TT_PLUS:					return "add";
		case LTS_TT_MINUS:					return "sub";
		case LTS_TT_STAR:					return "mul";
		case LTS_TT_FWD_SLASH:				return "div";
		case LTS_TT_PERCENT:				return "mod";
		case LTS_TT_COMPARE_EQUALS:			return "e";
		case LTS_TT_COMPARE_NOT_EQUALS:		return "n";
		case LTS_TT_COMPARE_GREATER_EQUALS:	return "ge";
		case LTS_TT_COMPARE_LESS_EQUALS:	return "le";
		case LTS_TT_ORDER:					return "o";
		case LTS_TT_LOGIC_AND:				return "land";
		case LTS_TT_LOGIC_OR:				return "lor";
		case LTS_TT_LOGIC_XOR:				return "lxor";
		case LTS_TT_BIT_AND:				return "band";
		case LTS_TT_BIT_OR:					return "bor";
		case LTS_TT_BIT_XOR:				return "bxor";
		default: context.error("Invalid/Unsupported operator!", node);
	}
	context.error("Invalid/Unsupported operator!", node);
}

static Makai::UTF8String uopName(ATransformer::Context& context, Node::Instance const& node) {
	switch (node->base.type) {
		case LTS_TT_SINGLE_QUOTE_STRING:
		case LTS_TT_DOUBLE_QUOTE_STRING:
		case LTS_TT_BACKTICK_STRING:
		case LTS_TT_FR_SINGLE_QUOTE_STRING:
		case LTS_TT_FR_DOUBLE_QUOTE_STRING:
		case LTS_TT_JP_SINGLE_QUOTE_STRING:
		case LTS_TT_JP_DOUBLE_QUOTE_STRING:
		case LTS_TT_IDENTIFIER:				return node->base.text;
		case LTS_TT_PLUS:					return "nop";
		case LTS_TT_MINUS:					return "neg";
		case LTS_TT_INCREMENT:				return "inc";
		case LTS_TT_DECREMENT:				return "dec";
		case LTS_TT_LOGIC_NOT:				return "lnot";
		case LTS_TT_BIT_NOT:				return "bnot";
		default: context.error("Invalid/Unsupported operator!", node);
	}
	context.error("Invalid/Unsupported operator!", node);
}

bool Namespace::isPureNamespace() const {
	return !(type || function || variable || attribute || trait);
}

Namespace::Instance ATransformer::Context::get(UTF8StringList const& path) {
	if (auto const ns = resolve(path))
		return ns;
	push(path);
	return scopeStack.back();
}

Namespace::Instance ATransformer::Context::declare(UTF8StringList const& path) {
	push(path);
	return scopeStack.back();
}

Namespace::Instance ATransformer::Context::fetch(UTF8StringList const& path, Node::Instance const& base) {
	if (auto const ns = resolve(path))
		return ns;
	error("Symbol does not exist!", base);
}

Namespace::Instance ATransformer::Context::fetch(Node::Instance const& nodePath) {
	return fetch(pathOf(nodePath), nodePath);
}

Makai::UTF8StringList ATransformer::Context::pathOf(Node::Instance const& node) {
	if (!node || node->content != Node::Content::AV2_TANC_NAME)
		return {};
	if (node->content == Node::Content::AV2_TANC_NAME)
		return Makai::UTF8StringList::from(node->value.getString());
	else if (!node->isPathOrName())
		Context::error("This is not a valid path!", node);
	Makai::UTF8StringList path;
	path.pushBack(node->leftSide->value.getString());
	path.appendBack(pathOf(node->rightSide));
	return path;
}

Makai::KeyValuePair<Makai::UTF8StringList, Namespace::Instance>
ATransformer::resolve(Context& context, Node::Instance const& node) const {
	auto const path = Context::pathOf(node);
	if (!allowPaths && path.size() > 1)
		context.error("Path declarations are forbidden in this context!", node);
	auto scope = context.resolve(path);
	return {path, scope};
}

ATransformer::Result VariableDecl::transform(Context& context, Node::Instance const& node) {
	auto [path, scope] = resolve(context, node);
	if (scope && scope->variable)
		context.error("Redeclaration of variable with the given path!");
	auto const parent = context.top();
	scope = context.declare(path);
	auto& var = *(scope->variable = scope->variable.create());
	var.name = scope->name;
	TypeRequest t;
	var.type = t.transform(context, node->middle).type;
	if (node->rightSide) {
		Expression expr;
	 	expr.transform(context, node);
	}
	context.pop(path.size());
	return {{Makai::toString("local[", parent->varc++, "]")}, scope, var.type};
}

ATransformer::Result StructureDecl::transform(Context& context, Node::Instance const& node) {
}

ATransformer::Result PrefixExpression::transform(Context& context, Node::Instance const& node) {
	Expression expr;
	auto const lhs = expr.transform(context, node->leftSide);
	if (!lhs.source)
		context.error("Invalid expression!", node->leftSide);
	if (lhs.source != "stack[-0]")
		context.top()->impl->writeMainLine("push", lhs.source);
	context.top()->impl->writeMainLine("op", bopName(context, node));
	return {{"stack[-0]"}, nullptr, lhs.type};
}

ATransformer::Result BinaryExpression::transform(Context& context, Node::Instance const& node) {
	Expression expr;
	auto const lhs = expr.transform(context, node->leftSide);
	if (!lhs.source)
		context.error("Invalid expression!", node->leftSide);
	if (lhs.source != "stack[-0]")
		context.top()->impl->writeMainLine("push", lhs.source);
	if (
		node->base.text == "as"
	||	node->base.text == "is"
	) {
		auto const t = TypeRequest().transform(context, node->rightSide);
		context.writeMainLine(node->base.text, t.type->name);
		return {{"stack[-0]"}, nullptr, node->base.text == "is" ? context.basicType("bool") : t.type};
	}
	auto const rhs = expr.transform(context, node->rightSide);
	if (!rhs.source)
		context.error("Invalid expression!", node->rightSide);
	if (lhs.type->derivedFrom(rhs.type))
		context.error("Type mismatch", node);
	if (rhs.source != "stack[-0]")
		context.top()->impl->writeMainLine("push", rhs.source);
	if (auto const t = TypeDecl::stronger(lhs.type, rhs.type)) {
		context.top()->impl->writeMainLine("op", bopName(context, node));
		return {{"stack[-0]"}, nullptr, lhs.type};
	}
}


ATransformer::Result Expression::transform(Context& context, Node::Instance const& node) {

}

ATransformer::Result TypeRequest::transform(Context& context, Node::Instance const& node) {
	return {.type = context.fetch(node)->type};
}

ATransformer::Result FunctionDecl::transform(Context& context, Node::Instance const& node) {
	auto const [path, scope] = resolve(context, node);
	if (scope->impl)
		context.error("Symbol is already defined as a different kind!", node);
	if (!scope->function) {
		scope->function = scope->function.create();
		scope->function->name = path.back();
	}
	auto& fn = *scope->function;
	auto const proto = node->middle;
	Function::OverloadRef ov = ov.create();
	if (proto->leftSide)
		ov->result = context.fetch(path, node->leftSide)->type;
	VariableDecl vd;
	for (auto const& arg: proto->children) {
		auto const decl = vd.transform(context, arg);
		if (!(decl.scope && decl.scope->variable))
			context.error("Expected variable declaration here!", arg);
		ov->arguments.pushBack(decl.scope->variable);
	}
	if (fn.overload(ov->arguments))
		context.error("Redeclaration of function overload!", node);
	else fn.overloads.pushBack(ov);
	return {.scope = scope};
}
