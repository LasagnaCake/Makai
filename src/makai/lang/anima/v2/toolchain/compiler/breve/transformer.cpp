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
		else if (stack) context.top()->impl->writeMainLine("push", var.getSource());
		context.top()->impl->writeMainLine("at", var.id);
		return {{"move stack[-0]"}, var.scope.raw(), var.type.raw()};
	} else
		return {var.getSource(), var.scope.raw(), var.type.raw()};
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

static Makai::Nullable<Makai::UTF8String> addToStack(
	ATransformer::Context& context,
	Namespace::Instance const& ns
) {
	if (ns->variable) {
		if (ns->variable->fieldOf && !ns->variable->staticEntity) {
			context.top()->impl->writeMainLine("at", ns->variable->id);
			return {"move stack[-0]"};
		}
		return ns->variable->getSource();
	} else if (ns->property) {
		auto const ov = ns->property->getter->overloadFromTypes({});
		context.top()->impl->writeMainLine("call", ov->entry);
		return {"move stack[-0]"};
	}
	return null;
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
			return {{"move stack[-0]"}, f->scope.raw(), f->type.raw()};
		}
	}
	if (ns->property) {
		if (ns->property->type->fields.contains(sub)) {
			auto const f = ns->type->fields[sub];
			auto const ov = ns->property->getter->overloadFromTypes({});
			context.top()->impl->writeMainLine("call", ov->entry);
			return {{"move stack[-0]"}, f->scope.raw(), f->type.raw()};
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

Makai::UTF8StringList ATransformer::Context::pathOf(UTF8String const& path) {
	return path.split(UTF8Char{'/'}).erase(0);
}

Makai::UTF8StringList ATransformer::Context::pathOf(Node::Instance const& node) {
	if (!node)
		return Makai::UTF8StringList();
	// CPP::Debug::breakpoint();
	DEBUGLN("NODE_ADDR_", node.raw());
	if (node->content == Node::Content::AV2_TANC_NAME) {
		DEBUGLN("------ Left:", node->value.getString());
		return Makai::UTF8StringList::from(node->value.getString());
	}
	else if (!node->isPathOrName())
		Context::error("This is not a valid path!", node);
	Makai::UTF8StringList path;
	if (node->rightSide)
		Context::error("This is not a valid path!", node->rightSide);
	path.appendBack(pathOf(node->leftSide));
	DEBUGLN("------ Right:", node->value.getString());
	path.appendBack(pathOf(node->value.getString()));
	DEBUG("Path: ");
	for (auto& name: path)
		DEBUG("/", name);
	DEBUG("\n");
	return copy(path);
}

Makai::KeyValuePair<Makai::UTF8StringList, Namespace::Instance>
ATransformer::resolve(Context& context, Node::Instance const& node) const {
	return resolve(context, node, allowPaths);
}

Makai::KeyValuePair<Makai::UTF8StringList, Namespace::Instance>
ATransformer::resolve(Context& context, Node::Instance const& node, bool allowPaths) {
	auto const path = Context::pathOf(node);
	if (!allowPaths && path.size() > 1)
		context.error("Path declarations are forbidden in this context!", node);
	auto scope = context.resolve(path);
	return {path, scope};
}

bool ATransformer::Result::isStackTop() const {
	return source && Makai::Regex::contains(*source, R"re(stack\[\-0\])re");
}

bool ATransformer::Result::isDiscardable() const {
	return type && type->flags & Core::Definition::Flags::AV2_DF_NO_RESULT;
}

bool ATransformer::Result::shouldBePushed() const {
	return !isDiscardable() && !isStackTop();
}

Namespace::Instance ATransformer::Context::nearestVarScope() const {
	for (auto& sco: Range::reverse(scopeStack)) {
		if (sco->isPureNamespace() && sco->declaredAsNamespace) continue;
		return sco;
	}
	return root;
}

ATransformer::Result VariableDecl::transform(Context& context, Node::Instance const& node) {
	auto path = context.pathOf(node->leftSide);
	auto const parent = context.nearestVarScope();
	if (context.top()->resolve(path))
		context.error("Redeclaration of previously-declared symbol!", node->leftSide);
	auto const scope = context.declare(path);
	auto& var = *(scope->variable = scope->variable.create());
	parent->impl->writeMainLine("decl 1");
	var.name = scope->name;
	TypeRequest t;
	if (node->middle)
		var.type = t.transform(context, node->middle).type.asWeak();
	Makai::Data::Value direct;
	if (node->rightSide) {
		Expression expr;
		auto const tmp = context.declare(UTF8StringList::from("<init>" + node->name()));
	 	auto const result = expr.transform(context, node->rightSide);
		if (result.source)
			tmp->impl->writeMainLine("copy", *result.source, "->", var.getSource());
		else context.error("Expected value here!", node->rightSide);
		context.pop(1);
		direct = result.direct;
		var.initializer = tmp;
		var.defaulted = true;
		if (!var.type)
			var.type = result.type.asWeak();
		if (parent != context.root && !var.staticEntity) {
			parent->impl->writeMainLine(tmp->impl->toString());
			var.initializer = nullptr;
		}
	}
	var.parentScope = parent.asWeak();
	var.value = direct;
	var.id = parent->varc++;
	context.pop(path.size());
	if (!var.type)
		context.error("[" + Makai::toString(__LINE__) + "]::INTERNAL_ERROR -> Variable has lost its type!", node);
	else if (var.type->flags & Core::Definition::Flags::AV2_DF_NO_RESULT)
		context.error("[Variables cannot have discardable types!", node);
	return {{var.getSource()}, scope, var.type.raw(), direct};
}

ATransformer::Result Aliasing::transform(Context& context, Node::Instance const& node) {
	auto const name = context.pathOf(node->leftSide);
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
		if (context.parent()->resolve(UTF8StringList::from(scope->name)))
			context.error("Symbol with this name already exists in the current scope!", node->leftSide);
		auto const tmp = context.declare(UTF8StringList::from(scope->name));
		context.parent()->subspaces[scope->name] = scope;
		context.pop(1);
	}
	return {.scope = scope};
}

ATransformer::Result Using::transform(Context& context, Node::Instance const& node) {
	auto scope = Expression().transform(context, node->leftSide).scope;
	if (!scope)
		context.error("Namespace does not exist!", node->leftSide);
	if (!scope->isPureNamespace())
		context.error("Scope is not a pure namespace!", node->leftSide);
	for (auto& [name, mem]: scope->subspaces) {
		if (!context.top()->subspaces.contains(name)) {
			DEBUGLN("Adding ", name, "...");
			context.top()->subspaces[name] = mem;
		}
	}
	return {.scope = scope};
}

ATransformer::Result StructureDecl::transform(Context& context, Node::Instance const& node) {
	if (!node->leftSide)
		context.error("Expected declaration name here!");
	auto const name = context.pathOf(node->leftSide);
	if (context.top()->subspaces.contains(name.front()))
		context.error("Symbol with this name already exists in the current scope!", node->leftSide);
	auto const scope = context.declare(name);
	auto& type = *(scope->type = scope->type.create());
	if (node->middle) {
		auto const base = TypeRequest().transform(context, node->middle).type;
		if (!base)
			context.error("No type with this name exists!", node->middle);
		type.base = base;
		type.flags |= base->flags & (~Core::Definition::Flags::AV2_DF_BASIC);
		auto baseFields = base->fields;
		baseFields["base"] = baseFields["this"];
		baseFields.erase("this");
		type.fields.append(baseFields);
		scope->varc += base->scope->varc;
	}
	{
		auto const thisVarScope = context.declare(UTF8StringList::from("this"));
		auto& thisVar = *(thisVarScope->variable = thisVarScope->variable.create());
		thisVar.fieldOf = scope->type;
		thisVar.source = "ref stack[-0]";
		thisVar.type = scope->type.asWeak();
		thisVar.parentScope = scope;
		scope->subspaces["this"] = thisVarScope;
		context.pop(1);
	}
	auto const initer = "__init_" + name.join("_") + node->name();
	Block().transform(context, node->rightSide);
	type.scope = scope.asWeak();
	type.node = node;
	type.name = "__" + name.join("_") + node->name();
	List<Namespace::VariableRef> defaulted;
	List<Namespace::VariableRef> statics;
	scope->type->def = TypeDecl::Definition::AV2_TCTD_STRUCT;
	scope->type->flags |= Core::Definition::Flags::AV2_DF_STRUCTURE;
	for (auto const& [name, sub]: scope->subspaces) {
		if (sub->variable) {
			auto& var = *sub->variable;
			var.fieldOf = scope->type.asWeak();
			type.fields[name] = sub->variable;
		}
	}
	context.pop(name.size());
	context.registerType(scope);
	return {.scope = scope, .type = scope->type};
}

ATransformer::Result Return::transform(Context& context, Node::Instance const& node) {
	Expression expr;
	auto const val = expr.transform(context, node->leftSide);
	if (!val.source)
		context.error("Invalid expression!", node->leftSide);
	if (val.shouldBePushed())
		context.top()->impl->writeMainLine("push", *val.source);
	context.top()->impl->writeMainLine("ret");
	return {{"move stack[-0]"}, val.scope, val.type};
}

ATransformer::Result Block::transform(Context& context, Node::Instance const& node) {
	ATransformer::Result result;
	context.writeMainLine("begin", context.top()->varc);
	context.writeMainLine("keep");
	for (auto const& child: node->children) {
		result = Expression().transform(context, child);
		if (result.scope && result.scope->variable) {
			context.writeMainLine(result.scope->impl->toString());
			if (!result.scope->variable->initializer)		continue;
			if (context.nearestVarScope() == context.root)	continue;
			context.writeMainLine(result.scope->variable->initializer->impl->toString());
			result.scope->variable->initializer = null;
		}
	}
	context.writeMainLine("end");
	return result;
}

ATransformer::Result SubExpression::transform(Context& context, Node::Instance const& node) {
	return Block().transform(context, node);
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
	if (val.shouldBePushed()) {
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
	if (val.shouldBePushed())
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
	if (lhs.shouldBePushed() && !lhs.direct)
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
	if (lhs.direct)
		context.top()->impl->writeMainLine("push", *lhs.source);
	if (rhs.shouldBePushed())
		context.top()->impl->writeMainLine("push", *rhs.source);
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
	ATransformer::Result result;
	if (node->content == Node::Content::AV2_TANC_NAME) {
		auto const [path, ns] = resolve(context, node);
		if (!ns)
			context.error("Symbol does not exist!", node);
		result.source = addToStack(context, ns.raw());
		if (ns->variable) {
			result.type		= ns->variable->type.raw();
			result.scope	= ns->variable->scope.raw();
		} else result.scope = ns;
		return result;
	} if (node->leftSide->content == Node::Content::AV2_TANC_FN_CALL) {
		auto const fcall = Call().transform(context, node->leftSide);
		path = context.pathOf(node->value.getString()).reverse();
		result = fcall;
	} else if (node->leftSide->content == Node::Content::AV2_TANC_SUBSCRIPT) {
		auto const sub = Subscript().transform(context, node->leftSide);
		path = context.pathOf(node->value.getString()).reverse();
		result = sub;
	}  else if (node->leftSide->content == Node::Content::AV2_TANC_PATH) {
		auto const nsx = PathExpression().transform(context, node->leftSide);
		path = context.pathOf(node->value.getString()).reverse();
		result = nsx;
		if (result.source && nsx.shouldBePushed())
			context.top()->impl->writeMainLine("push", *result.source);
		return resolveSubfield(context, node, result.scope, path.back());
	} else if (node->leftSide->content == Node::Content::AV2_TANC_NAME) {
		path = context.pathOf(node);
		auto const ns = context.resolve(path);
		if (!ns)
			context.error("Symbol does not exist!", node);
		result.source = addToStack(context, ns.raw());
		if (ns->variable) {
			result.type		= ns->variable->type.raw();
			result.scope	= ns->variable->scope.raw();
		} else result.scope = ns;
		return result;
	}
	if (!result.scope->subspaces.contains(path.front()))
		context.error("Subpath type doesn't contain the given member!", node->leftSide);
	return resolveSubfield(context, node, result.scope, path.front());
}

ATransformer::Result Expression::transform(Context& context, Node::Instance const& node) {
	if (!node) return {};
	DEBUGLN("Expression Type: ", Node::asString(node->content));
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
		case Node::Content::AV2_TANC_DROP:				return Drop().transform(context, node);
		case Node::Content::AV2_TANC_NEW:				return Create().transform(context, node);
		case Node::Content::AV2_TANC_IMPORT:			return Import().transform(context, node);
		case Node::Content::AV2_TANC_ALIAS:				return Aliasing().transform(context, node);
		case Node::Content::AV2_TANC_UNSCOPING:			return Using().transform(context, node);
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
		if (field.defaultValue.isUndefined())
			missing.pushBack(name);
		else attr->value[name] = field.defaultValue;
	if (missing.size())
		context.error("Required attribute parameters [" + missing.join(",") + "] missing!", node);
	scope->attribute->transform(context, ns, attr->value, *attr->attribute);
}

static Makai::UTF8StringList resolveAttribute(
	ATransformer::Context& context,
	Node::Instance const& node,
	Namespace::Instance const& ns,
	Makai::Dictionary<Metadata::Instance>& attribs
) {
	Makai::UTF8StringList newAttrs;
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
		for (auto const& at: node->children) {
			if (!at)
				context.error("Invalid attribute field!", at);
			if (at->content != Node::Content::AV2_TANC_ASSIGNMENT)
				context.error("Invalid attribute field specifier!", at);
			if (at->leftSide->content != Node::Content::AV2_TANC_NAME)
				context.error("Expected name here!", at->leftSide);
			auto const name = at->leftSide->value.getString();
			DEBUGLN("~~~~~~~~~~~~ Attribute Field: ", name);
			if (attr->value.contains(name))
				context.error("Redeclaration of previously-declared field!", at->leftSide);
			if (!attr->attribute->fields.contains(name))
				context.error("Field does not exist for given attribute!", at);
			{
				if (at->rightSide->isPathOrName() && attr->attribute->fields[name].path) {
					attr->value[name] = context.pathOf(at->rightSide).join("/").toString();
				} else if (attr->attribute->fields[name].path) {
					context.error("Expected path here!", at->rightSide);
				} else if (at->rightSide->content != Node::Content::AV2_TANC_VALUE) {
					context.error("Expected constant (or name) here!", at->rightSide);
				}
			}
			attr->value[name] = at->rightSide->value;
		}
		Makai::UTF8StringList missing;
		for (auto const& [name, desc]: attr->attribute->fields) {
			if (!attr->value.contains(name)) {
				if (desc.defaultValue.isUndefined())
					missing.pushBack(name);
				else attr->value[name] = desc.defaultValue;
			} else if (attr->value[name].type() != desc.type)
				context.error("Attribute field ["+name+"] type mismatch!", node);
		}
		if (missing.size())
			context.error("Required attributes [" + missing.join(",") + "] missing!", node);
		attribs[scope->attribute->name] = attr;
		attr->attribute->transform(context, ns, attr->value, *attr->attribute);
		newAttrs.pushBack(scope->attribute->name);
	} else if (node->content == Node::Content::AV2_TANC_ARRAY) {
		for (auto const& attrib: node->children) {
			auto const attrs = resolveAttribute(context, attrib, ns, attribs);
			newAttrs.appendBack(attrs);
		}
	}
	return newAttrs;
}

ATransformer::Result AttributeExpression::transform(Context& context, Node::Instance const& node) {
	auto const expr = Expression().transform(context, node->rightSide);
	if (!expr.scope) context.error("Expected scope here!", node->rightSide);
	Makai::Dictionary<Metadata::Instance> attributes;
	resolveAttribute(context, node->leftSide, expr.scope, attributes);
	Makai::StringList repeat;
	if (!expr.scope->function) {
		for (auto const& attr: expr.scope->meta.keys())
			if (attributes.contains(attr))
				repeat.pushBack(attr);
		if (repeat.size())
			context.error("Reapplication of previous attributes [" + repeat.join(",") + "]!", node->rightSide);
	}
	if (attributes.contains("Attribute"))
		if (!expr.scope->type) context.error("Expected structure here!", node->rightSide);
	expr.scope->meta.append(attributes);
	return expr;
}

static Makai::UTF8String overloadName(Makai::List<Namespace::VariableRef> const& args) {
	Makai::UTF8String name;
	for (auto const& arg: args)
		name += "_" + arg->type->name;
	return name;
}

ATransformer::Result FunctionDecl::transform(Context& context, Node::Instance const& node) {
	auto [path, scope] = resolve(context, node->leftSide);
	DEBUGLN("Path = /", path.join("/"));
	bool isCompletelyNewFunction = false;
	if (scope) {
		if (!scope->isPureNamespace() && !scope->function)
			context.error("Symbol is already defined as a different kind!", node);
		if (!scope->function) {
			scope->function = scope->function.create();
			scope->function->name = path.join("_");
		}
		context.scopeStack.pushBack(scope);
	} else {
		scope = context.declare(path);
		scope->function = scope->function.create();
		scope->function->name = path.join("_");
		isCompletelyNewFunction = true;
	}
	DEBUG("Stack = ");
	for (auto& sco: context.scopeStack)
		DEBUG("/", sco->name);
	DEBUGLN("");
	auto& fn = *scope->function;
	auto const proto = node->middle;
	Function::OverloadRef ov = ov.create();
	if (proto->rightSide && !(ov->result = TypeRequest().transform(context, node->rightSide).type))
		context.error("Type does not exist!");
	auto const newScope = context.declare(Makai::UTF8StringList::from("<fn>" + node->name()));
	VariableDecl vd;
	List<Namespace::VariableRef> optionals;
	for (auto const& arg: proto->children) {
		auto const decl = vd.transform(context, arg);
		if (!decl.scope->variable->type)
			context.error("[" + Makai::toString(__LINE__) + "]::INTERNAL_ERROR -> Variable has lost its type!");
		if (!(decl.scope && decl.scope->variable))
			context.error("Expected variable declaration here!", arg);
		if (decl.scope->variable->defaulted && decl.scope->variable->initializer)
			optionals.pushBack(decl.scope->variable);
		else if (optionals.empty())
			ov->arguments.pushBack(decl.scope->variable);
		else context.error("Cannot have required arguments follow optional ones!", arg);
	}
	context.pop(1);
	Namespace::Instance implScope;
	Function::OverloadRef implOv;
	DEBUGLN("Optionals: ", optionals.size());
	if (optionals.size()) {
		for (auto i: Makai::range(optionals.size())) {
			auto args = ov->arguments;
			args.appendBack(optionals.sliced(0, -(i+1)));
			if (auto const f = fn.overloadFromVariables(args)) {
				if (f->scope || f->result != ov->result || ov->outEntry.size())
					context.error("Redeclaration of function overload!", node);
				if (!implScope) {
					auto const ovName = scope->function->name + overloadName(args);
					implScope = context.declare(Makai::UTF8StringList::from("<overload>" + ovName));
					f->entry = "__" + ovName  + node->name();
					context.pop(1);
				}
			} else {
				auto const ovName = scope->function->name + overloadName(args);
				auto const overloadScope = context.declare(Makai::UTF8StringList::from("<overload>" + ovName));
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
					overloadScope->impl->writePreLine("@def", oo->entry, ":");
					overloadScope->impl->writePreLine("begin", toString(args.size()));
					overloadScope->impl->writePreLine("bind ref", toString(args.size()), "[0 -> 0]");
					overloadScope->impl->writeMainLine(oo->arguments[i+1]->initializer->compose()->toString());
					overloadScope->impl->writePostLine("call", implOv->entry);
					overloadScope->impl->writePostLine("end");
					overloadScope->impl->writePostLine("@def .\n");
				}
				context.pop(1);
			}
		}
	} else {
		implOv = ov;
		if (node->rightSide)
			implScope = newScope;
		auto const ovName = scope->function->name + overloadName(ov->arguments);
		ov->entry = "__" + ovName  + node->name();
		fn.overloads.pushBack(implOv);
	}
	if (node->rightSide) {
		context.scopeStack.pushBack(implScope);
		implScope->impl->writePreLine("@def", implOv->entry, ":");
		implScope->impl->writePreLine("begin", implScope->varc);
		implScope->impl->writePreLine("bind ref", implScope->varc, "[0 -> 0]");
		implScope->impl->writePreLine("clear", implScope->varc);
		auto const expr = Expression().transform(context, node->rightSide);
		if (expr.source && expr.shouldBePushed())
			implScope->impl->writePostLine("push", *expr.source);
		implScope->impl->writePostLine("end");
		implScope->impl->writePostLine("@def .\n");
		implOv->scope = implScope.asWeak();
		context.scopeStack.popBack();
		if (!implOv->result && expr.source)
			implOv->result = expr.type;
	}
	if (!implOv->result)
		implOv->result = context.basicType("void");
	context.pop(isCompletelyNewFunction ? path.size() : 1);
	context.registerFunction(scope);
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
	auto const path = context.pathOf(node->leftSide);
	auto const fpath = path.join("/").toString();
	DEBUG("Path: ");
	for (auto& p: path)
		DEBUG("/", p);
	DEBUGLN("");
	auto const subinter = importer(fpath);
	// This is for testing purposes
	if (!subinter.content) return {};
	for (auto& [name, imp]: context.root->subspaces["##T0_IMPORTS"]->subspaces)
		if (imp == subinter.content) return {.scope = subinter.content};
	context.registerImport(subinter.content);
	return {.scope = subinter.content};
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
	if ((context.top()->resolve(path) && !context.top()->resolve(path)->isPureNamespace()))
		context.error("Redeclaration of previously-declared symbol!", node->leftSide);
	auto const scope = context.declare(path);
	scope->declaredAsNamespace = true;
	Block().transform(context, node->rightSide);
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
	DEBUGLN("Left-side: ", node->leftSide->base.text);
	auto const fn = Expression().transform(context, node->leftSide);
	if (!fn.scope)
		context.error("Symbol does not exist!", node->leftSide);
	DEBUGLN(fn.scope->name);
	if (!fn.scope->function)
		context.error("Symbol is not a function!", node->leftSide);
	auto& f = *fn.scope->function;
	Function::ArgTypes args;
	for (auto const& arg: node->children) {
		auto const expr = Expression().transform(context, arg);
		if (!expr.source)
			context.error("Expected value here!", arg);
		if (expr.shouldBePushed())
			context.top()->impl->writeMainLine("push", *expr.source);
		args.pushBack(expr.type);
	}
	DEBUGLN("Function: ", f.name);
	DEBUG("Overloads: [ ");
	for (auto const& ov: f.overloads)
		DEBUG(ov->prototype(), " ");
	DEBUGLN("]");
	auto const ovLookupSig = args.toList<UTF8String>([] (auto const& e) {return e->name;}).join(" ");
	DEBUGLN("Looking for: [", ovLookupSig, "]");
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
	if (src.shouldBePushed())
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
		if (index.shouldBePushed())
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
		if (expr.shouldBePushed())
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

ATransformer::Result InlineIfElse::transform(Context& context, Node::Instance const& node) {
	auto const iif = Branch().transform(context, node);
	if (!(iif.source and iif.type)) context.error("inline if-elses must result in a value!", node);
	return iif;
}

ATransformer::Result Branch::transform(Context& context, Node::Instance const& node) {
	auto const cond = Expression().transform(context, node->leftSide);
	if (!cond.direct.isUndefined()) {
		if (cond.direct.isTruthy()) return Expression().transform(context, node->middle);
		else if (node->rightSide) return Expression().transform(context, node->rightSide);
		else return {};
	} else {
		if (!cond.source)
			context.error("Expression does not result in a value!", node->leftSide);
		auto const ifTrueLabel = "__if_" + node->name() + "_true_";
		auto const ifFalseLabel = "__if_" + node->name() + "_false_";
		auto const ifEndLabel = "__if_" + node->name() + "_end_";
		if (cond.shouldBePushed())
			context.writeMainLine("push", cond.source.value());
		context.writeMainLine("pick [", ifTrueLabel, node->rightSide ? ifFalseLabel : ifEndLabel, "]");
		context.writeMainLine("@target", ifTrueLabel, ":");
		context.writeMainLine("begin 0");
		context.writeMainLine("keep");
		auto const ifTrue = Expression().transform(context, node->middle);
		if (ifTrue.source && ifTrue.shouldBePushed())
			context.writeMainLine("push", ifTrue.source.value());
		context.writeMainLine("end");
		context.writeMainLine("jump", ifEndLabel);
		if (node->rightSide) {
			context.writeMainLine("@target", ifFalseLabel, ":");
			context.writeMainLine("begin 0");
			context.writeMainLine("keep");
			auto const ifFalse = Expression().transform(context, node->rightSide);
			if (ifFalse.source && ifFalse.shouldBePushed())
				context.writeMainLine("push", ifFalse.source.value());
			if (ifTrue.type != ifFalse.type)
				context.error("Both paths return different types!", node);
			context.writeMainLine("end");
			context.writeMainLine("jump", ifEndLabel);
		}
		context.writeMainLine("@target", ifEndLabel, ":");
		return {.source = {"move stack[-0}"}, .type = ifTrue.type};
	}
}

ATransformer::Result Loop::transform(Context& context, Node::Instance const& node) {
	// TODO: This
	return {};
}

ATransformer::Result Definition::transform(Context& context, Node::Instance const& node) {
	if (node->base.text == "::")			return FunctionDecl().transform(context, node);
	if (node->base.text == ":")				return VariableDecl().transform(context, node);
	if (node->base.type == LTS_TT_DECLARE)	return VariableDecl().transform(context, node);
	if (node->base.text == "prop")			return PropertyDecl().transform(context, node);
	if (node->base.text == "struct")		return StructureDecl().transform(context, node);
	if (node->base.text == "module")		return NamespaceDecl().transform(context, node);
	if (node->base.text == "*")				return ArrayTypeDecl().transform(context, node);
	context.error("Unimplemented support for given declaration!", node);
}

ATransformer::Result InlineAssembly::transform(Context& context, Node::Instance const& node) {
	auto const scope = context.nearestVarScope();
	for (auto& tok: node->interject)
		scope->impl->writeMain(tok.text);
	scope->impl->writeMainLine("");
	context.pop(1);
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
	context.registerType(t->scope.raw());
	return {.type = t};
}

Namespace::TypeRef ATransformer::Context::basicType(UTF8String const& name) {
	auto const scope = resolve(UTF8StringList::from(name));
	if (!(scope && scope->type))
		error("Basic type ["+name+"] does not exist!\nDid you forget to [using import core.types]?");
	return scope->type;
}

Namespace::TypeRef ATransformer::Context::arrayFor(Namespace::TypeRef const& type) {
	if (!type) return nullptr;
	if (!arrays.contains(type.asWeak())) {
		auto const arr = type.create();
		arr->flags |= Core::Definition::Flags::AV2_DF_ARRAY;
		arr->base = type;
		arr->name = type->name + "Array";
		auto const nsp = Namespace::Instance::create(arr->name);
		registerType(nsp);
		auto& ns = *nsp;
		ns.type = arr;
		arrays[type.asWeak()] = arr;
		return arr;
	} else return arrays[type.asWeak()];
}

static Makai::String idName(usize const id) {
	return Makai::Format::pad(Makai::toString(id), '0', 16, CTL::Format::Justify::CFJ_LEFT);
}

void ATransformer::Context::registerType(Namespace::Instance const& ns) {
	static usize id = 0;
	if (!ns) return;
	root->subspaces["##T1_USER_TYPES"]->subspaces[ Makai::toString("#", idName(++id), "::") + ns->name] = ns;
}

void ATransformer::Context::registerFunction(Namespace::Instance const& ns) {
	static usize id = 0;
	if (!ns) return;
	root->subspaces["##T2_FUNCTIONS"]->subspaces[ Makai::toString("#", idName(++id), "::") + ns->name] = ns;
}

void ATransformer::Context::registerImport(Namespace::Instance const& ns) {
	static usize id = 0;
	if (!ns) return;
	root->subspaces["##T0_IMPORTS"]->subspaces[ Makai::toString("#", idName(++id), "::") + ns->name] = ns;
}

ATransformer::Context::Context(): Intermediate() {
	using enum Core::BasicType;
	root->subspaces["##T0_IMPORTS"]		= Namespace::Instance::create("##T0_IMPORTS");
	root->subspaces["##T1_USER_TYPES"]	= Namespace::Instance::create("##T1_USER_TYPES");
	root->subspaces["##T2_FUNCTIONS"]	= Namespace::Instance::create("##T2_FUNCTIONS");
	root->subspaces["##T3_TRAITS"]		= Namespace::Instance::create("##T3_TRAITS");
}

void ATransformer::Context::addBasicType(Core::BasicType const type, uint64 const flags) {
	static usize id = 0;
	using enum Core::BasicType;
	UTF8String name;
	switch (type) {
		default: error("UHHHHHHHHHHH");
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
	root->subspaces["##T1_BASICS"]->subspaces[Makai::toString("#", idName(++id), "::") + name] = ns;
	root->subspaces[name] = ns;
	auto& t = *(ns->type = ns->type.create());
	t.name = name;
	t.basic = type;
	t.flags |= Core::Definition::Flags::AV2_DF_BASIC | flags;
	if (type != AV2_BT_VOID && type != AV2_BT_ANY)
		t.base = basicType("any");
	basics[name] = ns->type;
}

Makai::Function<File(Makai::UTF8String const&)> Import::importer = [] (auto const&) -> File {
	throw Error::InvalidAction("Missing importer!");
};
