#include "transformer.hpp"
#include "intermediate.hpp"
#include "resolver.hpp"

/*

[MikuTeto (+ Yi Xi)]
↑O↑   ▼O▼    Θ-▐▌
/|\   /|\   /|\!
/ \   / \   / \

 */

// Ignore the ugly .raw() calls

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;
using namespace Transformer;

using Type = BaseContext::Tokenizer::Token::Type;

using enum BaseContext::Tokenizer::Token::Type;

using Token = BaseContext::Tokenizer::Token;

static ATransformer::Result expandVariable(
	ATransformer::Context& context,
	Node::Instance const& node,
	Makai::UTF8StringList path,
	Variable& var,
	bool const stack = false
);

static ATransformer::Result expandProperty(
	ATransformer::Context& context,
	Node::Instance const& node,
	Makai::UTF8StringList path,
	Property& prop,
	bool const stack = false
);

static ATransformer::Result expandVariable(
	ATransformer::Context& context,
	Node::Instance const& node,
	Makai::UTF8StringList path,
	Variable& var,
	bool const stack
) {
	if (var.fieldOf) {
		auto parent = context.resolve(path = path.sliced(0, -1));
		while (path.size()) {
			parent = context.resolve(path = path.sliced(0, -1));
			if (!parent) continue;
			if (parent->variable && parent->variable->type.asWeak() == var.fieldOf)
				break;
			if (parent->property && parent->property->type.asWeak() == var.fieldOf)
				break;
		}
		if (parent && parent->variable)
			expandVariable(context, node, path, *parent->variable, true);
		else if (parent && parent->property)
			expandProperty(context, node, path, *parent->property, true);
		else if (stack) context.top()->impl->writeMainLine("push", var.source);
		context.top()->impl->writeMainLine("at", var.id);
		return {{"move stack[-0]"}, var.scope.raw(), var.type};
	} else
		return {var.source, var.scope.raw(), var.type};
}

static ATransformer::Result expandProperty(
	ATransformer::Context& context,
	Node::Instance const& node,
	Makai::UTF8StringList path,
	Property& prop,
	bool const stack
) {
	auto const get = prop.getter->overloadFromTypes({});
	auto parent = context.resolve(path = path.sliced(0, -1));
	while (path.size()) {
		parent = context.resolve(path = path.sliced(0, -1));
		if (!parent) continue;
		if (parent->variable && parent->variable->type.asWeak() == prop.fieldOf)
			break;
		if (parent->property && parent->property->type.asWeak() == prop.fieldOf)
			break;
	}
	if (parent && parent->variable)
		expandVariable(context, node, path, *parent->variable, true);
	else if (parent && parent->property)
		expandProperty(context, node, path, *parent->property, true);
	context.top()->impl->writeMainLine("call", get->entry);
	return {{"move stack[-0]"}, prop.scope.raw(), get->result};
}

static void addToStack(
	ATransformer::Context& context,
	Namespace::Instance const& ns
) {
	if (ns->variable) {
		if (ns->variable->fieldOf && !ns->variable->staticEntity)
			context.top()->impl->writeMainLine("at", ns->variable->id);
		context.top()->impl->writeMainLine("push", ns->variable->source);
	} else if (ns->property) {
		auto const ov = ns->property->getter->overloadFromTypes({});
		context.top()->impl->writeMainLine("call", ov->entry);
	}
}

static ATransformer::Result resolveSubfield(
	ATransformer::Context& context,
	Node::Instance const& node,
	Namespace::Instance const& ns,
	Makai::UTF8String const& sub
) {
	if (!ns)
		context.error("Symbol with this name does not exist at the given path!", node);
	if (sub.empty()) {
		return {{"move stack[-0]"}, ns};
	}
	if (ns->variable) {
		if (ns->variable->type->fields.contains(sub)) {
			auto const f = ns->type->fields[sub];
			context.top()->impl->writeMainLine("at", f->id);
			return {{"move stack[-0]"}, f->scope.raw(), f->type};
		}
	}
	if (ns->property) {
		if (ns->property->type->fields.contains(sub)) {
			auto const f = ns->type->fields[sub];
			auto const ov = ns->property->getter->overloadFromTypes({});
			context.top()->impl->writeMainLine("call", ov->entry);
			return {{"move stack[-0]"}, f->scope.raw(), f->type};
		}
	}
	return {};
}

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
			auto const ov = tok->function->overloadFromTypes(Function::ArgTypes::from(type, type));
			context.top()->impl->writeMainLine("call", ov->entry);
			return {{"move stack[-0]"}, ov->result->scope.raw(), ov->result};
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
			auto const ov = tok->function->overloadFromTypes(Function::ArgTypes::from(type));
			context.top()->impl->writeMainLine("call", ov->entry);
			return {{"move stack[-0]"}, ov->result->scope.raw(), ov->result};
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
			auto const ov = tok->function->overloadFromTypes(Function::ArgTypes::from(type));
			context.top()->impl->writeMainLine("call", ov->entry);
			return {{"move stack[-0]"}, ov->result->scope.raw(), ov->result};
		}
	context.error("Invalid operator for type!", node);
}

ATransformer::~ATransformer() {}

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
	if (node->rightSide->content != Node::Content::AV2_TANC_NAME)
		Context::error("This is not a valid path!", node->rightSide);
	path.appendBack(pathOf(node->leftSide));
	path.pushBack(node->rightSide->value.getString());
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
	auto path = context.pathOf(node->leftSide);
	auto const parent = context.top();
	if (parent->resolve(path))
		context.error("Redeclaration of previously-declared symbol!", node->leftSide);
	auto const scope = context.declare(path);
	auto& var = *(scope->variable = scope->variable.create());
	var.name = scope->name;
	TypeRequest t;
	var.type = t.transform(context, node->middle).type;
	Makai::Data::Value direct;
	if (node->rightSide) {
		Expression expr;
		auto const tmp = context.declare(UTF8StringList::from("<>" + node->name()));
	 	auto const result = expr.transform(context, node);
		context.pop(1);
		direct = result.direct;
		var.initializer = tmp;
		var.defaulted = true;
	}
	var.value = direct;
	var.id = parent->varc++;
	var.source = Makai::toString("move local[", var.id, "]");
	context.pop(path.size());
	return {{var.source}, scope, var.type, direct};
}

ATransformer::Result Aliasing::transform(Context& context, Node::Instance const& node) {
	auto const name = context.pathOf(node->rightSide);
	auto scope = Expression().transform(context, node->rightSide).scope;
	if (!scope)
		context.error("Requested symbol scope does not exist!", node->rightSide);
	if (node->leftSide) {
		auto const alias = context.pathOf(node->leftSide);
		if (context.parent()->resolve(alias))
			context.error("Symbol with this name already exists in the current scope!", node->leftSide);
		auto const tmp = context.declare(alias);
		context.parent()->subspaces[alias.back()] = scope;
		context.pop(alias.size());
	} else {
		if (context.parent()->subspaces.contains(name.back()))
			context.error("Symbol with this name already exists in the current scope!", node->rightSide);
		auto const tmp = context.declare(UTF8StringList::from(name.back()));
		context.parent()->subspaces[name.back()] = scope;
		context.pop(1);
	}
	return {.scope = scope};
}

ATransformer::Result StructureDecl::transform(Context& context, Node::Instance const& node) {
	auto const name = context.pathOf(node->leftSide);
	if (context.top()->subspaces.contains(name.front()))
		context.error("Symbol with this name already exists in the current scope!", node->leftSide);
	auto const scope = context.declare(name);
	auto& type = *(scope->type = scope->type.create());
	auto const initer = name.join("_") + node->name();
	Block().transform(context, node->rightSide);
	type.scope = scope.asWeak();
	type.node = node;
	List<Namespace::VariableRef> defaulted;
	List<Namespace::VariableRef> statics;
	scope->type->flags |= Core::Definition::Flags::AV2_DF_STRUCTURE;
	for (auto const& [name, sub]: scope->subspaces) {
		if (sub->variable) {
			auto& var = *sub->variable;
			var.fieldOf = scope->type.asWeak();
			type.fields[name] = sub->variable;
		}
	}
	context.pop(name.size());
	return {.scope = scope, .type = scope->type};
}

ATransformer::Result Return::transform(Context& context, Node::Instance const& node) {
	Expression expr;
	auto const val = expr.transform(context, node->leftSide);
	if (!val.source)
		context.error("Invalid expression!", node->leftSide);
	if (!val.isStackTop())
		context.top()->impl->writeMainLine("push", *val.source);
	context.top()->impl->writeMainLine("ret");
	return {{"move stack[-0]"}, val.scope, val.type};
}

ATransformer::Result Block::transform(Context& context, Node::Instance const& node) {
	ATransformer::Result result;
	for (auto const& child: node->children)
		result = Expression().transform(context, child);
	return result;
}

ATransformer::Result SubExpression::transform(Context& context, Node::Instance const& node) {
	ATransformer::Result result;
	auto const scope = context.declare(UTF8StringList::from("::" + node->name()));
	scope->subspaces = context.parent()->subspaces;
	scope->varc = context.parent()->varc;
	for (auto const& child: node->children)
		result = Expression().transform(context, child);
	context.pop(1);
	context.writeMainLine("begin", context.top()->varc + scope->varc);
	context.writeMainLine("bring", context.top()->varc, "[0 : 0]");
	context.top()->impl->writeMainLine(scope->impl->compose()->toString());
	context.writeMainLine("end");
	return result;
}

static Namespace::TypeRef directName(ATransformer::Context& context, Makai::Data::Value::Kind const& type) {
	switch (type) {
		case Makai::Data::Value::Kind::DVK_BOOLEAN:		return context.basicType("bool");
		case Makai::Data::Value::Kind::DVK_SIGNED:		return context.basicType("int64");
		case Makai::Data::Value::Kind::DVK_UNSIGNED:	return context.basicType("uint64");
		case Makai::Data::Value::Kind::DVK_REAL:		return context.basicType("float64");
		case Makai::Data::Value::Kind::DVK_VECTOR:		return context.basicType("vector");
		case Makai::Data::Value::Kind::DVK_STRING:		return context.basicType("string");
		case Makai::Data::Value::Kind::DVK_BYTES:		return context.basicType("bytes");
		default: return {};
	}
}

template<class T>
static Makai::Data::Value uopDirectResolveEX(T const& v, Token const& tok) {
	if constexpr (Makai::Type::Integer<T>) {
		switch (tok.type) {
			case LTS_TT_BIT_NOT: if constexpr (Makai::Type::Equal<T, bool>) return !v; else return ~v;
			default: break;
		}
	}
	if constexpr (Makai::Type::Number<T>) {
		if (tok.type == LTS_TT_IDENTIFIER) {
			auto const id = tok.text;
			if (id == "sin")	return Makai::Math::sin<double>(v);
			if (id == "cos")	return Makai::Math::cos<double>(v);
			if (id == "tan")	return Makai::Math::tan<double>(v);
			if (id == "asin")	return asin(v);
			if (id == "acos")	return acos(v);
			if (id == "atan")	return atan(v);
			if (id == "sinh")	return sinh(v);
			if (id == "cosh")	return cosh(v);
			if (id == "tanh")	return tanh(v);
			if (id == "log2")	return Makai::Math::log2<double>(v);
			if (id == "log10")	return Makai::Math::log10<double>(v);
			if (id == "ln")		return Makai::Math::log<double>(v);
			if (id == "sqrt")	return Makai::Math::sqrt<double>(v);
		} else switch (tok.type) {
			case LTS_TT_LOGIC_NOT:	return !v;
			default: break;
		}
	}
	if (tok.type == LTS_TT_IDENTIFIER) {
		auto const id = tok.text;
		if (id == "inv") return 1.0 / v;
		if (id == "copy") return v;
		if (id == "move") return v;
		if (id == "ref") return v;
		if (id == "sizeof") return sizeof(v);
		if (id == "countof") return Makai::Data::Value(v).size();
	} else switch (tok.type) {
		case LTS_TT_INCREMENT:	return v + 1;
		case LTS_TT_DECREMENT:	return v - 1;
		case LTS_TT_PLUS:		return v * 1;
		case LTS_TT_MINUS:		return v * (-1);
		default: break;
	}
	return {};
}

static Makai::Data::Value uopDirectResolve(Makai::Data::Value const& v, Token const& tok) {
	switch (v.type()) {
		case Makai::Data::Value::Kind::DVK_BOOLEAN:		return uopDirectResolveEX(v.getBoolean(),	tok);
		case Makai::Data::Value::Kind::DVK_SIGNED:		return uopDirectResolveEX(v.getSigned(),	tok);
		case Makai::Data::Value::Kind::DVK_UNSIGNED:	return uopDirectResolveEX(v.getUnsigned(),	tok);
		case Makai::Data::Value::Kind::DVK_REAL:		return uopDirectResolveEX(v.getReal(),		tok);
		case Makai::Data::Value::Kind::DVK_VECTOR:		return uopDirectResolveEX(v.getVector(),	tok);
		default: return {};
	}
}

template<class T>
static Makai::Data::Value bopDirectResolveEX(T const& a, T const& b, Token const& tok) {
	if constexpr (Makai::Type::Equal<T, bool>) {
		switch (tok.type) {
			case LTS_TT_BIT_AND:	return a && b;
			case LTS_TT_BIT_OR:		return a || b;
			case LTS_TT_BIT_XOR:	return a != b;
			default: break;
		}
	}
	if constexpr (Makai::Type::Integer<T>) {
		switch (tok.type) {
			case LTS_TT_BIT_AND:	return a & b;
			case LTS_TT_BIT_OR:		return a | b;
			case LTS_TT_BIT_XOR:	return a ^ b;
			case LTS_TT_LOGIC_AND:	return a && b;
			case LTS_TT_LOGIC_OR:	return a || b;
			case LTS_TT_LOGIC_XOR:	return bool(a) != bool(b);
			case LTS_TT_MODULO:		return a % b;
			default: break;
		}
	}
	if constexpr (Makai::Type::Number<T>) {
		if (tok.type == LTS_TT_IDENTIFIER) {
			auto const id = tok.text;
			if (id == "pow")	return Makai::Math::atan2<double>(a, b);
			if (id == "atan2")	return Makai::Math::pow<double>(a, b);
		} else switch (tok.type) {
			case LTS_TT_MODULO:	return Makai::Math::mod<double>(a, b);
			default: break;
		}
	}
	if (tok.type == LTS_TT_IDENTIFIER) {
	} else switch (tok.type) {
		case LTS_TT_INCREMENT:	return a + 1;
		case LTS_TT_DECREMENT:	return a - 1;
		case LTS_TT_PLUS:		return a + b;
		case LTS_TT_MINUS:		return a - b;
		case LTS_TT_STAR:		return a * b;
		case LTS_TT_DIVIDE:		return a / b;
		default: break;
	}
	return {};
}

static Makai::Data::Value bopDirectResolve(Makai::Data::Value const& a, Makai::Data::Value const& b, Token const& tok) {
	switch ((a.type() > b.type() ? a.type() : b.type())) {
		case Makai::Data::Value::Kind::DVK_BOOLEAN:		return bopDirectResolveEX(a.getBoolean(),	b.getBoolean(),		tok);
		case Makai::Data::Value::Kind::DVK_SIGNED:		return bopDirectResolveEX(a.getSigned(),	b.getSigned(),		tok);
		case Makai::Data::Value::Kind::DVK_UNSIGNED:	return bopDirectResolveEX(a.getUnsigned(),	b.getUnsigned(),	tok);
		case Makai::Data::Value::Kind::DVK_REAL:		return bopDirectResolveEX(a.getReal(),		b.getReal(),		tok);
		case Makai::Data::Value::Kind::DVK_VECTOR:		return bopDirectResolveEX(a.getVector(),	b.getVector(),		tok);
		default: {
			if (a.isString() && b.isString() && tok.type == LTS_TT_PLUS)
				return a.getString() + b.getString();
			return {};
		}
	}
}

ATransformer::Result PrefixExpression::transform(Context& context, Node::Instance const& node) {
	if (node->base.text == "return")
		return Return().transform(context, node);
	Expression expr;
	auto const val = expr.transform(context, node->leftSide);
	if (!val.source)
		context.error("Invalid expression!", node->leftSide);
	if (val.direct && node->base.text != "typeof") {
		auto const result = uopDirectResolve(val.direct, node->base);
		if (!result.isUndefined())
			return {{result.toString()}, val.scope, directName(context, result.type()), result};
	}
	if (
		node->base.text == "copy"
	||	node->base.text == "ref"
	||	node->base.text == "move"
	) return {{node->base.text + " " + *val.source}, val.scope, val.type, val.direct};
	if (!val.isStackTop()) {
		if (
			node->base.type == LTS_TT_INCREMENT
		||	node->base.type == LTS_TT_DECREMENT
		) context.top()->impl->writeMainLine("push ref", *val.source);
		else context.top()->impl->writeMainLine("push", *val.source);
	}
	if (
		node->base.text == "sizeof"
	||	node->base.text == "countof"
	||	node->base.text == "typeof"
	) {
		context.top()->impl->writeMainLine(node->base.text.sliced(0, -3));
		auto const retType = node->base.text == "typeof" ? context.basicType("type") : context.basicType("uint64");
		return {{"move stack[-0]"}, retType->scope.raw(), retType};
	}
	if (val.type->basic) {
		context.top()->impl->writeMainLine("op", bopName(context, node));
		return {{"move stack[-0]"}, val.type->scope.raw(), val.type};
	} else return prefixResolve(context, node, val.type);
}

ATransformer::Result PostfixExpression::transform(Context& context, Node::Instance const& node) {
	Expression expr;
	auto const val = expr.transform(context, node->rightSide);
	if (!val.source)
		context.error("Invalid expression!", node->rightSide);
	if (val.direct && node->base.text != "typeof") {
		auto const result = uopDirectResolve(val.direct, node->base);
		if (!result.isUndefined())
			return {{result.toString()}, val.type->scope.raw(), directName(context, result.type()), result};
	}
	if (!val.isStackTop())
		context.top()->impl->writeMainLine("push copy", *val.source);
	context.top()->impl->writeMainLine("op", uopName(context, node));
	if (val.type->basic) {
		context.top()->impl->writeMainLine("op", bopName(context, node));
		return {{"move stack[-0]"}, val.type->scope.raw(), val.type};
	} else return postfixResolve(context, node, val.type);
}

ATransformer::Result InfixExpression::transform(Context& context, Node::Instance const& node) {
	Expression expr;
	auto const lhs = expr.transform(context, node->leftSide);
	if (!lhs.source)
		context.error("Invalid expression!", node->leftSide);
	if (!lhs.isStackTop() && !lhs.direct)
		context.top()->impl->writeMainLine("push", *lhs.source);
	if (
		node->base.text == "as"
	||	node->base.text == "is"
	) {
		auto const t = TypeRequest().transform(context, node->rightSide);
		context.writeMainLine(node->base.text, t.type->name);
		auto const retType = node->base.text == "is" ? context.basicType("bool") : t.type;
		return {{"move stack[-0]"}, retType->scope.raw(), retType};
	}
	auto const rhs = expr.transform(context, node->rightSide);
	if (!rhs.source)
		context.error("Invalid expression!", node->rightSide);
	if (lhs.direct && rhs.direct) {
		auto const result = bopDirectResolve(lhs.direct, rhs.direct, node->base);
		if (!result.isUndefined())
			return {{result.toString()}, directName(context, result.type())->scope.raw(), directName(context, result.type()), result};
	}
	if (!rhs.isStackTop())
		context.top()->impl->writeMainLine("push", *rhs.source);
	if (lhs.direct) {
		context.top()->impl->writeMainLine("push", *rhs.source);
		context.top()->impl->writeMainLine("swap");
	}
	if (auto const t = TypeDecl::stronger(lhs.type, rhs.type)) {
		if (t->basic) {
			context.top()->impl->writeMainLine("op", bopName(context, node));
			return {{"move stack[-0]"}, t->scope.raw(), t};
		} else return infixResolve(context, node, t);
	}
	context.error("Type mismatch!", node);
}

ATransformer::Result Direct::transform(Context& context, Node::Instance const& node) {
	if (!node || node->content != Node::Content::AV2_TANC_VALUE)
		context.error("Expected value here!", node);
	auto const v = node->value.toString();
	if (node->value.isBoolean())	return {{v}, context.basicType("bool")->scope.raw(),	context.basicType("bool"),		node->value	};
	if (node->value.isString())		return {{v}, context.basicType("string")->scope.raw(),	context.basicType("string"),	node->value	};
	if (node->value.isUnsigned())	return {{v}, context.basicType("uint64")->scope.raw(),	context.basicType("uint64"),	node->value	};
	if (node->value.isSigned())		return {{v}, context.basicType("int64")->scope.raw(),	context.basicType("int64"),		node->value	};
	if (node->value.isReal())		return {{v}, context.basicType("float64")->scope.raw(),	context.basicType("float64"),	node->value	};
	context.error("Invalid constant!", node);
}

ATransformer::Result PathExpression::transform(Context& context, Node::Instance const& node) {
	UTF8StringList path;
	Handle<Namespace> ns;
	if (node->leftSide->content == Node::Content::AV2_TANC_FN_CALL) {
		auto const fcall = Call().transform(context, node->leftSide);
		path = context.pathOf(node->rightSide).reverse();
		ns = fcall.type->scope;
	} else if (node->leftSide->content == Node::Content::AV2_TANC_SUBSCRIPT) {
		auto const fcall = Subscript().transform(context, node->leftSide);
		path = context.pathOf(node->rightSide).reverse();
		ns = fcall.type->scope;
	} else if (node->leftSide->content == Node::Content::AV2_TANC_NAME) {
		path = context.pathOf(node->leftSide).reverse();
		ns = context.resolve(Makai::UTF8StringList::from(path.front())).asWeak();
		if (!ns) context.error("Symbol with this name does not exist!", node->leftSide);
		return {.scope = ns.raw()};
		addToStack(context, ns.raw());
	} else if (node->leftSide->content == Node::Content::AV2_TANC_PATH) {
		auto const nsx = PathExpression().transform(context, node->leftSide);
		path = context.pathOf(node->rightSide).reverse();
		ns = nsx.scope;
		return resolveSubfield(context, node, ns.raw(), path.back());
	}
	if (!ns->subspaces.contains(path.front()))
		context.error("Subpath type doesn't contain the given member!", node->leftSide);
	return {.scope = ns->subspaces[path.front()]};
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
		case Node::Content::AV2_TANC_NAME:
		case Node::Content::AV2_TANC_PATH:				return PathExpression().transform(context, node);
		default: context.error("Unsupported expression!", node);
	}
}

ATransformer::Result TypeRequest::transform(Context& context, Node::Instance const& node) {
	if (node->content == Node::Content::AV2_TANC_DECLARATION && node->base.text == "*")
		return ArrayTypeDecl().transform(context, node);
	auto const t = context.fetch(node)->type;
	if (!t) context.error("Type does not exist!", node);
	return {.type = t};
}

static void resolveEmptyAttribute(
	ATransformer::Context& context,
	Node::Instance const& node,
	Makai::Dictionary<Metadata::Instance>& attribs,
	Namespace::Instance const& ns
) {
	auto const [path, scope] = ATransformer::resolve(context, node, true);
	if (!(scope && scope->attribute)) context.error("Attribute does not exist!", node);
	if (scope->attribute->useCount < scope->attribute->globalMax)
		++scope->attribute->useCount;
	else context.error("Attribute limit reached!", node);
	if (!Attribute::matchesTarget(*ns, scope->attribute->target))
		context.error("Invalid attribute for given expression!", node);
	if (attribs.contains(scope->attribute->name))
		context.error("Reapplication of previous attribute!", node);
	auto const attr = Metadata::Instance::create();
	attribs[scope->attribute->name] = attr;
	attr->attribute = scope->attribute;
	Makai::UTF8StringList missing;
	for (auto& [name, field]: attr->attribute->fields)
		if (!field.defaultValue)
			missing.pushBack(name);
		else attr->value[name] = field.defaultValue;
	if (missing.size())
		context.error("Required attributes [" + missing.join(",") + "] missing!", node);
	scope->attribute->transform(context, ns, attr->value, *attr->attribute);
}

static Makai::Dictionary<Metadata::Instance> resolveAttribute(
	ATransformer::Context& context,
	Node::Instance const& node,
	Namespace::Instance const& ns,
	Makai::Dictionary<Metadata::Instance>& attribs
) {
	if (node->isPathOrName()) {
		resolveEmptyAttribute(context, node, attribs, ns);
	} else if (node->content == Node::Content::AV2_TANC_FN_CALL) {
		auto const [path, scope] = ATransformer::resolve(context, node->leftSide, true);
		if (!(scope && scope->attribute)) context.error("Attribute does not exist!", node->leftSide);
		if (scope->attribute->useCount < scope->attribute->globalMax)
			++scope->attribute->useCount;
		else context.error("Attribute limit reached!", node);
		if (!Attribute::matchesTarget(*ns, scope->attribute->target))
			context.error("Invalid attribute for given expression!", node);
		if (attribs.contains(scope->attribute->name))
			context.error("Reapplication of previous attribute!", node->leftSide);
		auto const attr = Metadata::Instance::create();
		attr->attribute = scope->attribute;
		for (auto const& at: node->leftSide->children) {
			if (!at)
				context.error("Invalid attribute field!", at);
			if (at->content != Node::Content::AV2_TANC_ASSIGNMENT)
				context.error("Invalid attribute field value!", at);
			if (at->leftSide->content != Node::Content::AV2_TANC_NAME)
				context.error("Expected name here!", at->leftSide);
			auto const name = at->leftSide->value.getString();
			if (attr->value.contains(name))
				context.error("Redeclaration of previously-declared field!", at->leftSide);
			if (!attr->attribute->fields.contains(name))
				context.error("Field does not exist for given attribute!", at);
			{
				if (at->rightSide->content == Node::Content::AV2_TANC_PATH && attr->attribute->fields[name].path) {
					attr->value[name] = context.pathOf(at->rightSide).join("/").toString();
				} else if (attr->attribute->fields[name].path) {
					context.error("Expected path here!", at->rightSide);
				} else if (!(
					at->rightSide->content == Node::Content::AV2_TANC_VALUE
				||	at->rightSide->content == Node::Content::AV2_TANC_NAME
				)) {
					context.error("Expected constant (or name) here!", at->rightSide);
				}
			}
			attr->value[name] = at->rightSide->value;
		}
		for (auto const& [name, desc]: attr->attribute->fields)
			if (!desc.defaultValue && !attr->value.contains(name))
				context.error("Required field does not exist!", node);
			else if (desc.defaultValue && !attr->value.contains(name))
				attr->value[name] = desc.defaultValue;
			else if (attr->value[name].type() != desc.type)
				context.error("Attribute field mismatch!", node);
		attribs[scope->attribute->name] = attr;
		attr->attribute->transform(context, ns, attr->value, *attr->attribute);
	} else if (node->content == Node::Content::AV2_TANC_ARRAY) {
		for (auto const& attrib: node->children) {
			auto const attrs = resolveAttribute(context, attrib, ns, attribs);
			if (attribs.countOf(attrs.keys()))
				context.error("Reapplication of previous attributes [" + attribs.match(attrs.keys()).join(",") + "]!", node);
			attribs.append(attrs);
		}
	}
	return attribs;
}

ATransformer::Result AttributeExpression::transform(Context& context, Node::Instance const& node) {
	auto const expr = Expression().transform(context, node->rightSide);
	if (!expr.scope) context.error("Expected scope here!", node->rightSide);
	Makai::Dictionary<Metadata::Instance> attributes;
		resolveAttribute(context, node->leftSide, expr.scope, attributes);
	if (expr.scope->meta.countOf(attributes.keys()))
		context.error("Reapplication of previous attributes [" + attributes.match(expr.scope->meta.keys()).join(",") + "]!", node->rightSide);
	if (attributes.contains("Attribute"))
		if (!expr.scope->type) context.error("Expected structure here!", node->rightSide);
	expr.scope->meta.append(attributes);
	return expr;
}

static Makai::UTF8String overloadName(Function::ArgTypes const& types) {
	Makai::UTF8String name;
	for (auto const& type: types)
		name += "_" + type->name;
	return name;
}

static Makai::UTF8String overloadName(Makai::List<Namespace::VariableRef> const& args) {
	Makai::UTF8String name;
	for (auto const& arg: args)
		name += "_" + arg->type->name;
	return name;
}

ATransformer::Result FunctionDecl::transform(Context& context, Node::Instance const& node) {
	auto const [path, scope] = resolve(context, node);
	if (!(scope->function))
		context.error("Symbol is already defined as a different kind!", node);
	if (!scope->function) {
		scope->function = scope->function.create();
		scope->function->name = path.join("_") + node->name();
	}
	auto& fn = *scope->function;
	auto const proto = node->middle;
	Function::OverloadRef ov = ov.create();
	if (proto->leftSide)
		ov->result = context.fetch(path, node->leftSide)->type;
	auto const newScope = context.declare(Makai::UTF8StringList::from("<>" + node->name()));
	VariableDecl vd;
	List<Namespace::VariableRef> optionals;
	for (auto const& arg: proto->children) {
		auto const decl = vd.transform(context, arg);
		if (!(decl.scope && decl.scope->variable))
			context.error("Expected variable declaration here!", arg);
		if (decl.scope->variable->defaulted)
			optionals.pushBack(decl.scope->variable);
		else if (optionals.empty())
			ov->arguments.pushBack(decl.scope->variable);
		else context.error("Cannot have required arguments follow optional ones!", arg);
	}
	context.pop(1);
	Namespace::Instance implScope;
	Function::OverloadRef implOv;
	for (auto i: Makai::range(optionals.size())) {
		auto args = ov->arguments;
		args.appendBack(optionals.sliced(0, -(i+1)));
		if (auto const f = fn.overloadFromVariables(args)) {
			if (f->scope || f->result != ov->result)
				context.error("Redeclaration of function overload!", node);
			if (!implScope) {
				auto const ovName = scope->function->name + overloadName(args);
				implScope = context.declare(Makai::UTF8StringList::from("<>" + ovName));
				f->entry = "__" + ovName  + node->name();
				context.pop(1);
			}
		} else {
			auto const ovName = scope->function->name + overloadName(args);
			auto const overloadScope = context.declare(Makai::UTF8StringList::from("<>" + ovName));
			if (!implScope)
				implScope = overloadScope;
			auto const oo = ov.create();
			oo->entry = "__" + ovName + node->name();
			oo->arguments = args;
			oo->result = ov->result;
			oo->scope = overloadScope.asWeak();
			for (auto const& arg: args)
				oo->scope->subspaces[arg->name] = arg->scope.raw();
			fn.overloads.pushBack(oo);
			if (!implOv) implOv = oo;
			else {
				context.writePreLine("@fn", oo->entry);
				overloadScope->impl->writePreLine(oo->entry, ":");
				overloadScope->impl->writePreLine("begin", toString(args.size()));
				overloadScope->impl->writePreLine("bind", toString(args.size()), "[0 : 0]");
				overloadScope->impl->writeMainLine(oo->arguments[i+1]->initializer->compose()->toString());
				overloadScope->impl->writePostLine("call", implOv->entry);
				overloadScope->impl->writePostLine("end");
			}
			context.pop(1);
		}
	}
	if (node->rightSide) {
		context.scopeStack.pushBack(implScope);
		implScope->impl->writePreLine(implOv->entry, ":");
		auto const expr = Expression().transform(context, node->rightSide);
		implScope->impl->writePreLine("begin", implScope->varc);
		implScope->impl->writePreLine("bind", implScope->varc, "[0 : 0]");
		implScope->impl->writePreLine("clear", implScope->varc);
		implScope->impl->writePostLine("end");
		implOv->scope = implScope.asWeak();
		context.scopeStack.popBack();
	}
	return {.scope = scope};
}

ATransformer::Result Assignment::transform(Context& context, Node::Instance const& node) {
	auto const lhs = Expression().transform(context, node->leftSide);
	auto const rhs = Expression().transform(context, node->rightSide);
	if (lhs.direct) context.error("Cannot assign a value to a direct value!", node->leftSide);
	if (auto const t = TypeDecl::stronger(lhs.type, lhs.type)) {
		context.top()->impl->writeMainLine("copy", *rhs.source, "->", *lhs.source);
		return {lhs.source, lhs.scope, t, rhs.direct};
	} else context.error("Type mismatch!", node);
}

ATransformer::Result Import::transform(Context& context, Node::Instance const& node) {
	auto const path = context.pathOf(node->rightSide);
	auto const fpath = path.join("/");
	auto const subinter = import(fpath);
	return {.scope = subinter.root};
}

ATransformer::Result PropertyDecl::transform(Context& context, Node::Instance const& node) {
	auto const path = context.pathOf(node->leftSide);
	if (context.top()->resolve(path))
		context.error("Redeclaration of previously-declared symbol!", node->leftSide);
	auto const scope = context.declare(path);
	scope->property = scope->property.create();
	if (node->middle) {
	} else if (node->children.size()) {
		for (auto const& child: node->children) {
			auto const member = Expression().transform(context, child);
			if (!member.scope->function)
				context.error("Properties can only have functions!", child);
			if (member.scope->meta.contains("Getter")) {
				if (scope->property->getter)
					context.error("Property getter has already been declared!");
				scope->property->getter = member.scope->function;
				bool hit = false;
				for (auto const& ov: member.scope->function->overloads)
					if (ov->methodOf && ov->arguments.size() == 0) {
						hit = true;
						break;
					}
				if (!hit) context.error("Missing required overload for getter!", child);
			}
			if (member.scope->meta.contains("Setter")) {
				if (scope->property->setter)
					context.error("Property setter has already been declared!");
				scope->property->setter = member.scope->function;
				bool hit = false;
				for (auto const& ov: member.scope->function->overloads)
					if (ov->methodOf && ov->arguments.size() == 1) {
						hit = true;
						break;
					}
				if (!hit) context.error("Missing required overload for setter!", child);
			}
		}
	}
	context.pop(path.size());
	return {.scope = scope};
}

ATransformer::Result NamespaceDecl::transform(Context& context, Node::Instance const& node) {
	auto const path = context.pathOf(node->leftSide);
	if (context.top()->resolve(path))
		context.error("Redeclaration of previously-declared symbol!", node->leftSide);
	auto const scope = context.declare(path);
	Block().transform(context, node);
	context.pop(path.size());
	return {.scope = scope};
}

ATransformer::Result Declaration::transform(Context& context, Node::Instance const& node) {
	if (node->base.type == LTS_TT_NAMESPACE_RESOLVE)
		return FunctionDecl().transform(context, node);
	if (node->base.type == LTS_TT_COLON || node->base.type == LTS_TT_DECLARE)
		return VariableDecl().transform(context, node);
	if (node->base.type == LTS_TT_IDENTIFIER) {
		if (node->base.text == "struct")
			return StructureDecl().transform(context, node);
		if (node->base.text == "prop")
			return PropertyDecl().transform(context, node);
		if (node->base.text == "module")
			return NamespaceDecl().transform(context, node);
	}
	context.error("Invalid declaration!", node);
}

ATransformer::Result Call::transform(Context& context, Node::Instance const& node) {
	auto const fn = Expression().transform(context, node->leftSide);
	if (!fn.scope->function)
		context.error("Symbol is not a function!", node->leftSide);
	auto& f = *fn.scope->function;
	Function::ArgTypes args;
	for (auto const& arg: node->children) {
		auto const expr = Expression().transform(context, arg);
		if (!expr.source)
			context.error("Expected value here!", arg);
		if (!expr.isStackTop())
			context.top()->impl->writeMainLine("push", *expr.source);
		args.pushBack(expr.type);
	}
	if (!f.overloadFromTypes(args))
		context.error("Requested overload does not exist!", node);
	auto& ov = *f.overloadFromTypes(args);
	if (
		(
			ov.variant == decltype(ov.variant)::AV2_TCB_FOV_NONE
		||	ov.variant == decltype(ov.variant)::AV2_TCB_FOV_GLOBAL
		)
	&&	Makai::Regex::contains(fn.source.orElse("").toString(), "stack")
	) context.top()->impl->writeMainLine("pop");
	context.top()->impl->writeMainLine("call", ov.entry);
	return {{"move stack[-0]"}, ov.result->scope.raw(), ov.result};
}

ATransformer::Result Subscript::transform(Context& context, Node::Instance const& node) {
	auto const src = Expression().transform(context, node->leftSide);
	if (!src.source)
		context.error("Expected value here!", node->leftSide);
	if (!(src.type->flags & Core::Definition::Flags::AV2_DF_ARRAY))
		context.error("Value is not an array!", node->rightSide);
	if (!src.isStackTop())
		context.top()->impl->writeMainLine("push", *src.source);
	auto const index = Expression().transform(context, node->rightSide);
	if (index.direct) {
		if (!index.direct.isUnsigned())
			context.error("Direct value must be an unsigned integer here!", node->rightSide);
		context.top()->impl->writeMainLine("at", index.direct.getUnsigned());
	}
	if (
		index.type == context.basicType("uint8")
	||	index.type == context.basicType("uint16")
	||	index.type == context.basicType("uint32")
	||	index.type == context.basicType("uint64")
	) {
		if (!index.isStackTop())
			context.top()->impl->writeMainLine("push", *src.source);
		context.top()->impl->writeMainLine("dyn at");
	} else context.error("Expected unsigned integer here!", node->rightSide);
	auto const t = src.type->base;
	return {{"move stack[-0]"}, t->scope.raw(), t};
}

ATransformer::Result Array::transform(Context& context, Node::Instance const& node) {
	Namespace::TypeRef prev;
	usize const count = node->children.size();
	for (auto const& arg: node->children) {
		auto const expr = Expression().transform(context, arg);
		if (!expr.source)
			context.error("Expected value here!", arg);
		if (!expr.isStackTop())
			context.top()->impl->writeMainLine("push", *expr.source);
		if (!prev)
			prev = expr.type;
		else if (!(prev = TypeDecl::stronger(prev, expr.type)))
			context.error("Type mismatch here!", arg);
	}
	auto const arr = context.arrayFor(prev);
	context.top()->impl->writeMainLine("new", arr->name);
	for (auto const i: range(count))
		context.top()->impl->writeMainLine("set", count - (i+1));
	return {{"move stack[-0]"}, arr->scope.raw(), arr};
}

ATransformer::Result Create::transform(Context& context, Node::Instance const& node) {
	auto const t = TypeRequest().transform(context, node->leftSide).type;
	context.top()->impl->writeMainLine("new", t->name);
	return {{"move stack[-0]"}, t->scope.raw(), t};
}

ATransformer::Result Drop::transform(Context& context, Node::Instance const& node) {
	auto const at = PathExpression().transform(context, node->leftSide);
	if (!at.source)
		context.error("Expression does not result in a value!", node->leftSide);
	if (!at.direct)
		context.top()->impl->writeMainLine("drop", *at.source);
	return {};
}

ATransformer::Result TheEntireProgram::transform(Context& context, Node::Instance const& node) {
	ATransformer::Result result;
	for (auto const& child: node->children)
		result = Expression().transform(context, child);
	return result;
}

ATransformer::Result ArrayTypeDecl::transform(Context& context, Node::Instance const& node) {
	auto const t = context.arrayFor(TypeRequest().transform(context, node->leftSide).type);
	return {.type = t};
}

Namespace::TypeRef ATransformer::Context::basicType(UTF8String const& name) {
	return basics.contains(name) ? basics[name] : nullptr;
}

Namespace::TypeRef ATransformer::Context::arrayFor(Namespace::TypeRef const& type) {
	if (!type) return nullptr;
	if (!arrays.contains(type.asWeak())) {
		auto const arr = type.create();
		arr->flags |= Core::Definition::Flags::AV2_DF_ARRAY;
		arr->base = type;
		arr->name = type->name + "_array";
		auto& ns = *(root->subspaces["__ARRAYS__"]->subspaces[arr->name] = Namespace::Instance::create(arr->name));
		ns.type = arr;
		return arr;
	} else return arrays[type.asWeak()];
}

ATransformer::Context::Context(): Intermediate() {
	using enum Core::BasicType;
	using Flags = Core::Definition::Flags;
	addBasicType(AV2_BT_ANY);
	addBasicType(AV2_BT_VOID, Flags::AV2_DF_EMPTY | Flags::AV2_DF_NO_RESULT);
	addBasicType(AV2_BT_NULL, Flags::AV2_DF_EMPTY | Flags::AV2_DF_NULLABLE);
	addBasicType(AV2_BT_BOOL);
	addBasicType(AV2_BT_CHAR);
	addBasicType(AV2_BT_INT8);
	addBasicType(AV2_BT_UINT8);
	addBasicType(AV2_BT_INT16);
	addBasicType(AV2_BT_UINT16);
	addBasicType(AV2_BT_INT32);
	addBasicType(AV2_BT_UINT32);
	addBasicType(AV2_BT_INT64);
	addBasicType(AV2_BT_UINT64);
	addBasicType(AV2_BT_UINT32);
	addBasicType(AV2_BT_REAL64);
	addBasicType(AV2_BT_REAL128);
	addBasicType(AV2_BT_STRING, Flags::AV2_DF_NULLABLE);
	addBasicType(AV2_BT_BYTES, Flags::AV2_DF_NULLABLE);
	addBasicType(AV2_BT_VECTOR);
	addBasicType(AV2_BT_MATRIX);
	addBasicType(AV2_BT_TYPEID);
	root->subspaces["__ARRAYS__"] = Namespace::Instance::create("__ARRAYS__");
}

void ATransformer::Context::addBasicType(Core::BasicType const type, uint64 const flags) {
	using enum Core::BasicType;
	UTF8String name;
	switch (type) {
		default: return;
		case AV2_BT_VOID:		name = "void";		break;
		case AV2_BT_ANY:		name = "any";		break;
		case AV2_BT_NULL:		name = "null";		break;
		case AV2_BT_TYPEID:		name = "type";		break;
		case AV2_BT_BOOL:		name = "bool";		break;
		case AV2_BT_CHAR:		name = "char";		break;
		case AV2_BT_INT8:		name = "int8";		break;
		case AV2_BT_INT16:		name = "int16";		break;
		case AV2_BT_INT32:		name = "int32";		break;
		case AV2_BT_INT64:		name = "int64";		break;
		case AV2_BT_UINT8:		name = "uint8";		break;
		case AV2_BT_UINT16:		name = "uint16";	break;
		case AV2_BT_UINT32:		name = "uint32";	break;
		case AV2_BT_UINT64:		name = "uint64";	break;
		case AV2_BT_REAL32:		name = "float32";	break;
		case AV2_BT_REAL64:		name = "float64";	break;
		case AV2_BT_REAL128:	name = "float128";	break;
		case AV2_BT_STRING:		name = "string";	break;
		case AV2_BT_BYTES:		name = "bytes";		break;
		case AV2_BT_VECTOR:		name = "vector";	break;
		case AV2_BT_MATRIX:		name = "matrix";	break;
	}
	if (root->subspaces.contains(name)) return;
	auto const ns = Namespace::Instance::create();
	root->subspaces[name] = ns;
	auto& t = *(ns->type = ns->type.create());
	t.name = name;
	t.basic = type;
	t.flags |= Core::Definition::Flags::AV2_DF_BASIC | flags;
	if (type != AV2_BT_VOID && type != Core::BasicType::AV2_BT_ANY)
		t.base = basicType("name");
	basics[name] = ns->type;
}
