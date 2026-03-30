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
	if (node->base.text == "not") return "lnot";
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

static ATransformer::Result infixResolve(ATransformer::Context& context, Node::Instance const& node, Namespace::TypeRef const& type) {
	for (auto& [name, tok]: type->scope->subspaces)
		if (
			tok->function
		&&	tok->meta.contains("Operator")
		&&	tok->meta["Operator"]->value.contains("infix")
		&&	tok->meta["Operator"]->value.fetch<Makai::UTF8String>("infix", "") == bopName(context, node)
		) {
			auto const ov = tok->function->overload(Function::ArgTypes::from(type, type));
			context.top()->impl->writeMainLine("call", ov->entry);
			return {{"stack[-0]"}, nullptr, ov->result};
		}
	context.error("Invalid operator for type!", node);
}

static ATransformer::Result prefixResolve(ATransformer::Context& context, Node::Instance const& node, Namespace::TypeRef const& type) {
	for (auto& [name, tok]: type->scope->subspaces)
		if (
			tok->function
		&&	tok->meta.contains("Operator")
		&&	tok->meta["Operator"]->value.contains("prefix")
		&&	tok->meta["Operator"]->value.fetch<Makai::UTF8String>("prefix", "") == uopName(context, node)
		) {
			auto const ov = tok->function->overload(Function::ArgTypes::from(type));
			context.top()->impl->writeMainLine("call", ov->entry);
			return {{"stack[-0]"}, nullptr, ov->result};
		}
	context.error("Invalid operator for type!", node);
}

static ATransformer::Result postfixResolve(ATransformer::Context& context, Node::Instance const& node, Namespace::TypeRef const& type) {
	for (auto& [name, tok]: type->scope->subspaces)
		if (
			tok->function
		&&	tok->meta.contains("Operator")
		&&	tok->meta["Operator"]->value.contains("postfix")
		&&	tok->meta["Operator"]->value.fetch<Makai::UTF8String>("postfix", "") == uopName(context, node)
		) {
			auto const ov = tok->function->overload(Function::ArgTypes::from(type));
			context.top()->impl->writeMainLine("call", ov->entry);
			return {{"stack[-0]"}, nullptr, ov->result};
		}
	context.error("Invalid operator for type!", node);
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
	if (!node)
		return {};
	if (node->content == Node::Content::AV2_TANC_NAME)
		return Makai::UTF8StringList::from(node->value.getString());
	else if (!node->isPathOrName())
		Context::error("This is not a valid path!", node);
	Makai::UTF8StringList path;
	if (node->content != Node::Content::AV2_TANC_NAME)
		Context::error("This is not a valid path!", node->leftSide);
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

bool ATransformer::Result::isStackTop() const {
	return source && Makai::Regex::contains(*source, R"re(stack\[-0\])re");
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
	return {{Makai::toString("move local[", parent->varc++, "]")}, scope, var.type};
}

ATransformer::Result StructureDecl::transform(Context& context, Node::Instance const& node) {
	// TODO: This
}

ATransformer::Result StaticExpression::transform(Context& context, Node::Instance const& node) {
	// TODO: This
}

ATransformer::Result Return::transform(Context& context, Node::Instance const& node) {
	Expression expr;
	auto const val = expr.transform(context, node->leftSide);
	if (!val.source)
		context.error("Invalid expression!", node->leftSide);
	if (!val.isStackTop())
		context.top()->impl->writeMainLine("push", val.source);
	context.top()->impl->writeMainLine("ret");
	return {{"move stack[-0]"}, val.scope, val.type};
}

ATransformer::Result Block::transform(Context& context, Node::Instance const& node) {
	ATransformer::Result result;
	auto const scope = context.declare(UTF8StringList::from("<>" + node->name()));
	for (auto const& child: node->children)
		result = Expression().transform(context, child);
	context.pop(1);
	return result;
}

ATransformer::Result SubExpression::transform(Context& context, Node::Instance const& node) {
	ATransformer::Result result;
	auto const scope = context.declare(UTF8StringList::from("<>" + node->name()));
	for (auto const& child: node->children)
		result = Expression().transform(context, child);
	context.pop(1);
	return result;
}

ATransformer::Result PrefixExpression::transform(Context& context, Node::Instance const& node) {
	if (node->base.text == "static")
		return StaticExpression().transform(context, node);
	if (node->base.text == "return")
		return Return().transform(context, node);
	Expression expr;
	auto const val = expr.transform(context, node->leftSide);
	if (!val.source)
		context.error("Invalid expression!", node->leftSide);
	if (
		node->base.text == "copy"
	||	node->base.text == "ref"
	||	node->base.text == "move"
	) return {{node->base.text + " " + *val.source}, val.scope, val.type};
	if (!val.isStackTop()) {
		if (
			node->base.type == LTS_TT_INCREMENT
		||	node->base.type == LTS_TT_DECREMENT
		) context.top()->impl->writeMainLine("push ref", val.source);
		else context.top()->impl->writeMainLine("push", val.source);
	}
	if (
		node->base.text == "sizeof"
	||	node->base.text == "countof"
	||	node->base.text == "typeof"
	) {
		context.top()->impl->writeMainLine(node->base.text.sliced(0, -3));
		return {{"move stack[-0]"}, val.scope, context.basicType("uint64")};
	}
	if (val.type->basic) {
		context.top()->impl->writeMainLine("op", bopName(context, node));
		return {{"move stack[-0]"}, nullptr, val.type};
	} else return prefixResolve(context, node, val.type);
}

ATransformer::Result PostfixExpression::transform(Context& context, Node::Instance const& node) {
	Expression expr;
	auto const val = expr.transform(context, node->rightSide);
	if (!val.source)
		context.error("Invalid expression!", node->rightSide);
	if (!val.isStackTop())
		context.top()->impl->writeMainLine("push copy", val.source);
	context.top()->impl->writeMainLine("op", uopName(context, node));
	if (val.type->basic) {
		context.top()->impl->writeMainLine("op", bopName(context, node));
		return {{"move stack[-0]"}, nullptr, val.type};
	} else return postfixResolve(context, node, val.type);
}

ATransformer::Result InfixExpression::transform(Context& context, Node::Instance const& node) {
	Expression expr;
	auto const lhs = expr.transform(context, node->leftSide);
	if (!lhs.source)
		context.error("Invalid expression!", node->leftSide);
	if (!lhs.isStackTop())
		context.top()->impl->writeMainLine("push", lhs.source);
	if (
		node->base.text == "as"
	||	node->base.text == "is"
	) {
		auto const t = TypeRequest().transform(context, node->rightSide);
		context.writeMainLine(node->base.text, t.type->name);
		return {{"move stack[-0]"}, nullptr, node->base.text == "is" ? context.basicType("bool") : t.type};
	}
	auto const rhs = expr.transform(context, node->rightSide);
	if (!rhs.source)
		context.error("Invalid expression!", node->rightSide);
	if (!rhs.isStackTop())
		context.top()->impl->writeMainLine("push", rhs.source);
	if (auto const t = TypeDecl::stronger(lhs.type, rhs.type)) {
		if (t->basic) {
			context.top()->impl->writeMainLine("op", bopName(context, node));
			return {{"move stack[-0]"}, nullptr, t};
		} else return infixResolve(context, node, t);
	}
	context.error("Type mismatch!", node);
}


ATransformer::Result Direct::transform(Context& context, Node::Instance const& node) {
	if (!node || node->content != Node::Content::AV2_TANC_VALUE)
		context.error("Expected value here!", node);
	auto const v = node->value.toString();
	if (node->value.isString())		return {{v}, nullptr,	context.basicType("string")		};
	if (node->value.isUnsigned())	return {{v}, nullptr,	context.basicType("uint64")		};
	if (node->value.isSigned())		return {{v}, nullptr,	context.basicType("int64")		};
	if (node->value.isReal())		return {{v}, nullptr,	context.basicType("float64")	};
	context.error("Invalid constant!", node);
}


ATransformer::Result Expression::transform(Context& context, Node::Instance const& node) {
	if (!node) return {};
	switch (node->content) {
		case Node::Content::AV2_TANC_EMPTY:				return {};
		case Node::Content::AV2_TANC_VALUE:				return Direct().transform(context, node);
		case Node::Content::AV2_TANC_BLOCK:				return Block().transform(context, node);
		case Node::Content::AV2_TANC_ASSIGNMENT:		return Assignment().transform(context, node);
		case Node::Content::AV2_TANC_DECLARATION:		return Declaration().transform(context, node);
		case Node::Content::AV2_TANC_FN_CALL:			return Call().transform(context, node);
		case Node::Content::AV2_TANC_DEFINITION:		return Definition().transform(context, node);
		case Node::Content::AV2_TANC_PREFIX_OP:			return PrefixExpression().transform(context, node);
		case Node::Content::AV2_TANC_INFIX_OP:			return InfixExpression().transform(context, node);
		case Node::Content::AV2_TANC_POSTFIX_OP:		return PostfixExpression().transform(context, node);
		case Node::Content::AV2_TANC_BRANCH:			return Branch().transform(context, node);
		case Node::Content::AV2_TANC_INLINE_IF_ELSE:	return InlineIfElse().transform(context, node);
		case Node::Content::AV2_TANC_LOOP:				return Loop().transform(context, node);
		case Node::Content::AV2_TANC_INLINE_MINIMA:		return InlineAssembly().transform(context, node);
		case Node::Content::AV2_TANC_ATTRIBUTE:			return AttributeExpression().transform(context, node);
		case Node::Content::AV2_TANC_NAME: {

		}
		default: context.error("Unsupported expression!", node);
	}
}

ATransformer::Result TypeRequest::transform(Context& context, Node::Instance const& node) {
	auto const t = context.fetch(node)->type;
	if (!t) context.error("Type does not exist!", node);
	return {.type = t};
}

static Makai::Dictionary<Metadata::Instance> resolveAttribute(ATransformer::Context& context, Node::Instance const& node) {
	Makai::Dictionary<Metadata::Instance> attribs;
	if (node->leftSide->content == Node::Content::AV2_TANC_NAME) {
	} else if (node->leftSide->content == Node::Content::AV2_TANC_PATH) {
		auto const [path, scope] = ATransformer::resolve(context, node, true);
		if (!(scope && scope->attribute)) context.error("Attribute does not exist!", node);
		if (attribs.contains(scope->attribute->name))
			context.error("Reapplication of previous attribute!", node);
		auto const attr = Metadata::Instance::create();
		attribs[scope->attribute->name] = attr;
		attr->attribute = scope->attribute;
	} else if (node->leftSide->content == Node::Content::AV2_TANC_FN_CALL) {
		auto const [path, scope] = ATransformer::resolve(context, node->leftSide, true);
		if (!(scope && scope->attribute)) context.error("Attribute does not exist!", node->leftSide);
		if (attribs.contains(scope->attribute->name))
			context.error("Reapplication of previous attribute!", node->leftSide);
		auto const attr = Metadata::Instance::create();
		attribs[scope->attribute->name] = attr;
		attr->attribute = scope->attribute;
	} else if (node->leftSide->content == Node::Content::AV2_TANC_ARRAY) {
		for (auto const& attrib: node->leftSide->children) {
			auto const attrs = resolveAttribute(context, attrib);
			if (attribs.contains(attrs.keys()))
				context.error("Reapplication of previous attributes [" + attribs.match(attrs.keys()).join(",") + "]!", node);
			attribs.append(attrs);
		}
	}
	return attribs;
}

ATransformer::Result AttributeExpression::transform(Context& context, Node::Instance const& node) {
	Makai::Dictionary<Metadata::Instance> attributes;
	if (node->leftSide->content == Node::Content::AV2_TANC_NAME) {
	} else if (node->leftSide->content == Node::Content::AV2_TANC_PATH) {
		auto const [path, scope] = resolve(context, node);
		if (!(scope && scope->attribute)) context.error("Attribute does not exist!", node);
	} else if (node->leftSide->content == Node::Content::AV2_TANC_ARRAY) {
		for (auto const& attrib: node->leftSide->children) {
			auto const [path, scope] = resolve(context, attrib);
			if (!(scope && scope->attribute)) context.error("Attribute does not exist!", node);
		}
	}
	auto const expr = Expression().transform(context, node->rightSide);
	if (!expr.scope) context.error("Expected scope here!", node->rightSide);
	if (expr.scope->meta.contains(attributes.keys()))
		context.error("Reapplication of previous attributes [" + attributes.match(expr.scope->meta.keys()).join(",") + "]!", node->rightSide);
	if (attributes.contains("Attribute"))
		if (!expr.scope->type) context.error("Expected structure here!", node->rightSide);
	expr.scope->meta.append(attributes);
}

ATransformer::Result FunctionDecl::transform(Context& context, Node::Instance const& node) {
	auto const [path, scope] = resolve(context, node);
	if (!(scope->function || scope->isPureNamespace()))
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
