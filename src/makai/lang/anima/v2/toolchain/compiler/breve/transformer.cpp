#include "transformer.hpp"
#include "intermediate.hpp"
#include "resolver.hpp"

/*

[MikuTeto (+ Yi Xi)]
↑O↑   ▼O▼    Θ-▐▌
/|\   /|\   /|\!
/ \   / \   / \

 */

// Ignore the ugly .asStrong() calls

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
		else if (stack) context.top()->impl->writeMainLine("push", var.consume(node));
		ATransformer::Result out = {{"move top"}, var.scope.asStrong(), var.type.asStrong()};
		out.parent = var.fieldOf.asStrong();
		return out;
	} else
		return {var.consume(node), var.scope.asStrong(), var.type.asStrong()};
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
	return {{"move top"}, prop.scope.asStrong(), get->result};
}

static Makai::Nullable<Makai::UTF8String> addToStack(
	ATransformer::Context& context,
	Namespace::Instance const& ns,
	Node::Instance const& node
) {
	if (ns->variable) {
		if (ns->variable->context > ExecutionContext::AV2_TCB_EC_RUNTIME)
			return {{ns->variable->value.toString() + " " + ns->variable->type->basicNumberName()}};
		if (!ns->variable->exists())
			context.error(ns->variable->emptyVarError(), node);
		if (ns->variable->fieldOf && !ns->variable->staticEntity) {
			context.top()->impl->writeMainLine("at [", ns->variable->id, "]");
			return {"move top"};
		}
		return ns->variable->consume(node);
	} else if (ns->property) {
		auto const ov = ns->property->getter->overloadFromTypes(
			Makai::List<Namespace::TypeRef>::from(ns->type.asStrong())
		);
		if (!ov)
			return null;
		context.top()->impl->writeMainLine("call", ov->entry);
		return {"move top"};
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
		return {{"move top"}, ns};
	}
	MAKAILIB_DEBUGLN_FULL("Looking for subspace '", sub, "'...");
	if (!ns->isPublic()) context.error("Symbol is not public!", node);
	if (ns->variable) {
		if (!ns->variable->exists())
			context.error(ns->variable->emptyVarError(), node);
		if (ns->variable->type->fields.contains(sub)) {
			auto const f = ns->variable->type->fields[sub];
			context.top()->impl->writeMainLine("push", ns->variable->consume(node));
			if (node->forAssignment)
				return {{"move top"}, f->scope.asStrong(), f->type.asStrong(), {}, ssize(f->id), ns->variable->type.asStrong()};
			context.top()->impl->writeMainLine("at [", f->id, "]");
			return {{f->passBy + " top"}, f->scope.asStrong(), f->type.asStrong(), {}, ssize(f->id), ns->variable->type.asStrong()};
		}
		if (ns->variable->type->scope->subspaces.contains(sub)) {
			auto const f = ns->variable->type->scope->subspaces[sub];
			if (f->function) return {.source = ns->variable->consume(node), .scope = f, .type = ns->variable->type.asStrong()};
			if (f->variable && f->variable->staticEntity) return {.source = {f->variable->consume(node)}, .scope = f, .type = f->variable->type.asStrong()};
			context.error("Invalid expression!", node);
		}
		context.error("Invalid expression!", node);
	}
	if (ns->property) {
		if (ns->property->type->fields.contains(sub)) {
			auto const f = ns->property->type->fields[sub];
			auto const ov = ns->property->getter->overloadFromTypes(
				Makai::List<Namespace::TypeRef>::from(ns->type.asStrong())
			);
			if (!ov)
				return {.scope = ns};
			context.top()->impl->writeMainLine("call", ov->entry);
			return {{"move top"}, ns, ov->result};
		}
		context.error("Symbol does not exist in the given scope!", node);
	}
	if (ns->type) {
		if (ns->type->scope->subspaces.contains(sub)) {
			auto const f = ns->subspaces[sub];
			if (!f->isPublic()) context.error("Symbol is not public!", node);
			if (f->function) return {.scope = f};
			if (f->variable) {
				if (f->variable->context > ExecutionContext::AV2_TCB_EC_RUNTIME)
					return {.source = {f->variable->consume(node)}, .scope = f, .type = f->variable->type.asStrong(), .direct = f->variable->value};
				if (f->variable->staticEntity)
					return {.source = {f->variable->consume(node)}, .scope = f, .type = f->variable->type.asStrong()};
			}
			context.error("Invalid expression!", node);
		}
		context.error("Symbol does not exist in the given scope!", node);
	}
	MAKAILIB_DEBUG_FULL("Available Subspaces: [ ");
	for (auto const& [name, subns]: ns->subspaces)
		MAKAILIB_DEBUG_FULL( "{", name , ":", subns->name, "} ");
	MAKAILIB_DEBUGLN_FULL("]");
	if (!ns->subspaces.contains(sub))
		context.error("Symbol does not exist in the given scope!", node);
	return {.scope = ns->subspaces[sub]};
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
		case LTS_TT_GREATER_THAN:			return "g";
		case LTS_TT_LESS_THAN:				return "l";
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
		&&	tok->meta["Operator"]->value
		&&	tok->meta["Operator"]->value.contains("infix")
		&&	tok->meta["Operator"]->value.fetch<Makai::UTF8String>("infix", "") == bopName(context, node)
		) {
			auto const ov = tok->function->overloadFromTypes(Function::ArgTypes::from(type, type));
			if (!ov) continue;
			context.top()->impl->writeMainLine("call", ov->entry);
			return {{"move top"}, ov->result->scope.asStrong(), ov->result};
		}
	context.error("Invalid operator for type!", node);
}

static ATransformer::Result prefixResolve(ATransformer::Context& context, Node::Instance const& node, Namespace::TypeRef const& type) {
	for (auto& [name, tok]: type->scope->subspaces)
		if (
			tok->function
		&&	tok->meta.contains("Operator")
		&&	tok->meta["Operator"]->value
		&&	tok->meta["Operator"]->value.contains("prefix")
		&&	tok->meta["Operator"]->value.fetch<Makai::UTF8String>("prefix", "") == uopName(context, node)
		) {
			auto const ov = tok->function->overloadFromTypes(Function::ArgTypes::from(type));
			context.top()->impl->writeMainLine("call", ov->entry);
			return {{"move top"}, ov->result->scope.asStrong(), ov->result};
		}
	context.error("Invalid operator for type!", node);
}

static ATransformer::Result postfixResolve(ATransformer::Context& context, Node::Instance const& node, Namespace::TypeRef const& type) {
	for (auto& [name, tok]: type->scope->subspaces)
		if (
			tok->function
		&&	tok->meta.contains("Operator")
		&&	tok->meta["Operator"]->value
		&&	tok->meta["Operator"]->value.contains("postfix")
		&&	tok->meta["Operator"]->value.fetch<Makai::UTF8String>("postfix", "") == uopName(context, node)
		) {
			auto const ov = tok->function->overloadFromTypes(Function::ArgTypes::from(type));
			context.top()->impl->writeMainLine("call", ov->entry);
			return {{"move top"}, ov->result->scope.asStrong(), ov->result};
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
	return path.split(UTF8Char{'/'}).sliced(1);
}

Makai::UTF8StringList ATransformer::Context::pathOf(Node::Instance const& node) {
	if (!node)
		return Makai::UTF8StringList();
	// CPP::Debug::breakpoint();
	MAKAILIB_DEBUGLN_FULL("NODE_ADDR_", node.asStrong());
	if (node->content == Node::Content::AV2_TANC_NAME) {
		MAKAILIB_DEBUGLN_FULL("------ Left:", node->value.getString());
		return Makai::UTF8StringList::from(node->value.getString());
	}
	else if (!node->isPathOrName())
		Context::error("This is not a valid path!", node);
	Makai::UTF8StringList path;
	if (node->rightSide)
		Context::error("This is not a valid path!", node->rightSide);
	if (node->leftSide)
		path.appendBack(pathOf(node->leftSide));
	MAKAILIB_DEBUGLN_FULL("------ Right:", node->value.getString());
	path.appendBack(pathOf(node->value.getString()));
	MAKAILIB_DEBUG_FULL("Path: ");
	for (auto& name: path)
		MAKAILIB_DEBUG_FULL("/", name);
	MAKAILIB_DEBUG_FULL("\n");
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

bool ATransformer::Result::isCopied() const {
	return source && Makai::Regex::contains(*source, R"re(^val(ue)?)re");
}

bool ATransformer::Result::isStackTop() const {
	return source && Makai::Regex::contains(*source, R"re(stack\[\-0\]|top)re");
}

bool ATransformer::Result::isDiscardable() const {
	return type && type->flags.hasNoResult;
}

bool ATransformer::Result::shouldBePushed() const {
	return source && !isDiscardable() && !isStackTop();
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
	var.parentScope = parent.asWeak();
	var.id = parent->varc++;
	TypeRequest t;
	if (node->middle)
		var.type = t.transform(context, node->middle).type.asWeak();
	Makai::Data::Value direct;
	if (node->rightSide) {
		var.fill();
		Expression expr;
		auto const tmp = context.declare(UTF8StringList::from("<init>" + node->name()));
	 	auto const result = expr.transform(context, node->rightSide);
		if (result.mayBeEmpty) context.error("One or more code paths may not result in a value!", node->rightSide);
		if (result.source)
			tmp->impl->writeMainLine("copy", *result.source, "->", var.getSource());
		else context.error("Expected value here!", node->rightSide);
		if (!result.shouldBePushed())
			context.top()->impl->writeMainLine("pop");
		context.pop(1);
		MAKAILIB_DEBUGLN_FULL("Direct Value = ", result.direct.toString());
		var.value = result.direct;
		var.initializer = tmp;
		var.defaulted = true;
		if (!var.type)
			var.type = result.type.asWeak();
	}
	context.pop(path.size());
	if (!var.type)
		context.error("[" + Makai::toString(__LINE__) + "]::INTERNAL_ERROR -> Variable has lost its type!", node);
	else if (var.type->flags.hasNoResult)
		context.error("Variables cannot have discardable types!", node);
	return {{"ref " + var.getSource()}, scope, var.type.asStrong(), direct};
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
			MAKAILIB_DEBUGLN_FULL("Adding ", name, "...");
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
		violate<uint64>(type.flags) |= bitcast<uint64>(base->flags);
		type.flags.isBasic = false;
		type.fields.append(base->fields);
		type.methods.append(base->methods);
		for (auto& [name, method]: base->methods) {
			auto& ns = *(type.scope->subspaces[name] = new Namespace);
			ns.function = method;
		}
		scope->varc += base->scope->varc;
	}
	auto const initer = "__init_" + name.join("_") + node->name();
	List<Node::Instance> fields;
	List<Node::Instance> methods;
	List<Node::Instance> properties;
	Makai::Function<void(Node::Instance const&, Node::Instance const&)> evalDecl;
	evalDecl = [&] (Node::Instance const& node, Node::Instance const& root) {
		if (node->content == Node::Content::AV2_TANC_DECLARATION) {
			if (node->base.type == LTS_TT_NAMESPACE_RESOLVE) {
				MAKAILIB_DEBUGLN_FULL("  > Function");
				methods.pushBack(root);
			} else if (
				node->base.type == LTS_TT_COLON
			or	node->base.type == LTS_TT_DECLARE
			or	node->base.type == LTS_TT_ASSIGN
			) {
				MAKAILIB_DEBUGLN_FULL("  > Field");
				fields.pushBack(root);
			} else if (node->base.text == "prop") {
				MAKAILIB_DEBUGLN_FULL("  > Property");
				properties.pushBack(root);
			}
			else context.error("Invalid declaration inside structure declaration!", node);
		} else if (node->content == Node::Content::AV2_TANC_ATTRIBUTE) {
			evalDecl(node->rightSide, root);
		} else context.error("Invalid expression inside structure declaration!", node);
	};
	MAKAILIB_DEBUGLN_FULL("struct {");
	for (auto& entry: node->rightSide->children)
		evalDecl(entry, entry);
	MAKAILIB_DEBUGLN_FULL("}");
	type.scope = scope.asWeak();
	type.node = node;
	type.name = "__" + name.join("_") + node->name();
	List<Namespace::VariableRef> defaulted;
	List<Namespace::VariableRef> statics;
	List<Namespace::VariableRef> privates;
	List<Namespace::VariableRef> protecteds;
	scope->type->def = TypeDecl::Definition::AV2_TCTD_STRUCT;
	scope->type->flags.isStructure = true;
	MAKAILIB_DEBUGLN_FULL("Parsing fields...");
	MAKAILIB_DEBUGLN_FULL("Field count: ", fields.size());
	for (auto const& [field, id]: Range::expand(fields)) {
		auto const decl = Expression().transform(context, field);
		auto& var = *decl.scope->variable;
		var.fieldOf = scope->type.asWeak();
		type.fields[var.name] = decl.scope->variable;
		scope->subspaces[var.name] = decl.scope;
		MAKAILIB_DEBUGLN_FULL("Field: ", var.name);
		var.id = id;
		if (var.scope->isPrivate())
			privates.pushBack(decl.scope->variable);
		else if (var.scope->isProtected())
			protecteds.pushBack(decl.scope->variable);
		var.scope->makePublic();
		if (var.staticEntity)
			statics.pushBack(decl.scope->variable);
	}
	context.pop(name.size());
	context.registerType(scope);
	auto implName = name;
	implName.back() = "::IMPL__" + implName.back();
	context.declare(implName);
	MAKAILIB_DEBUGLN_FULL("Parsing properties...");
	MAKAILIB_DEBUGLN_FULL("Property count: ", properties.size());
	for (auto& property: properties) {
		auto const decl = Expression().transform(context, property);
		auto& prop = *decl.scope->property;
		if (prop.getter) {
			auto& fn = *prop.getter;
			fn.name += "_" + prop.name + node->name();
			for (auto& ov: fn.current) {
				if (!ov->staticEntity && (ov->arguments.empty() or ov->arguments[0]->type != scope->type))
					context.error("Missing appropriate [this] parameter!", property);
				if (!ov->staticEntity)
					ov->methodOf = scope->type.asWeak();
			}
			if (scope->subspaces.contains(fn.name))
				context.error("Symbol with this name already exists!", property);
		}
		if (prop.setter) {
			auto& fn = *prop.setter;
			fn.name += "_" + prop.name + node->name();
			for (auto& ov: fn.current) {
				if (!ov->staticEntity && (ov->arguments.empty() or ov->arguments[0]->type != scope->type))
					context.error("Missing appropriate [this] parameter!", property);
				if (!ov->staticEntity)
					ov->methodOf = scope->type.asWeak();
			}
			if (scope->subspaces.contains(fn.name))
				context.error("Symbol with this name already exists!", property);
		}
		if (scope->subspaces.contains(prop.name))
			context.error("Symbol with this name already exists!", property);
		prop.fieldOf = scope->type.asWeak();
		type.methods[prop.name] = decl.scope->function;
		scope->subspaces[prop.name] = decl.scope;
	}
	MAKAILIB_DEBUGLN_FULL("Parsing methods...");
	MAKAILIB_DEBUGLN_FULL("Method count: ", methods.size());
	for (auto& method: methods) {
		auto const decl = Expression().transform(context, method);
		auto& fn = *decl.scope->function;
		for (auto& ov: fn.current) {
			if (!ov->staticEntity && (ov->arguments.empty() or ov->arguments[0]->type != scope->type))
				context.error("Missing appropriate [this] parameter!", method);
			if (!ov->staticEntity)
				ov->methodOf = scope->type.asWeak();
		}
		if (scope->subspaces.contains(fn.name))
			context.error("Symbol with this name already exists!", method);
		type.methods[fn.name] = decl.scope->function;
		scope->subspaces[fn.name] = decl.scope;
	}
	context.pop(implName.size());
	for (auto& var: privates)
		var->scope->makePrivate();
	for (auto& var: protecteds)
		var->scope->makeProtected();
	context.pop(1);
	return {.scope = scope, .type = scope->type, .mayBeEmpty = false};
}

ATransformer::Result EnumDecl::transform(Context& context, Node::Instance const& node) {
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
		if (!base->flags.isBasic)
			context.error("Enums can only inherit integers!", node->middle);
		if (!Core::isInteger(*base->basic))
			context.error("Enums can only inherit integers!", node->middle);
		type.base = base;
	} else type.base = context.basicType("int64");
	MAKAILIB_DEBUGLN_FULL("Integer type is ", type.base->name);
	type.flags.isEnum = true;
	type.flags.isValueType = true;
	type.flags.isCopyable = true;
	type.flags.isStructure = false;
	List<Node::Instance> fields;
	List<Node::Instance> methods;
	Makai::Function<void(Node::Instance const&, Node::Instance const&)> evalDecl;
	evalDecl = [&] (Node::Instance const& node, Node::Instance const& root) {
		if (node->content == Node::Content::AV2_TANC_DECLARATION) {
			if (node->base.type == LTS_TT_NAMESPACE_RESOLVE) {
				MAKAILIB_DEBUGLN_FULL("  > Function");
				methods.pushBack(root);
			} else context.error("Invalid declaration inside enum declaration!", node);
		} else if (node->content == Node::Content::AV2_TANC_ATTRIBUTE) {
			evalDecl(node->rightSide, root);
		} else if (
			node->content == Node::Content::AV2_TANC_ASSIGNMENT
		or	node->content == Node::Content::AV2_TANC_NAME
		) {
				fields.pushBack(root);
		} else context.error("Invalid expression inside enum declaration!", node);
	};
	MAKAILIB_DEBUGLN_FULL("struct {");
	for (auto& entry: node->rightSide->children)
		evalDecl(entry, entry);
	MAKAILIB_DEBUGLN_FULL("}");
	type.scope = scope.asWeak();
	type.node = node;
	type.name = "__" + name.join("_") + node->name();
	List<Namespace::VariableRef> defaulted;
	List<Namespace::VariableRef> statics;
	scope->type->def = TypeDecl::Definition::AV2_TCTD_ENUM;
	scope->type->flags.isStructure = true;
	MAKAILIB_DEBUGLN_FULL("Parsing fields...");
	MAKAILIB_DEBUGLN_FULL("Field count: ", fields.size());
	int64 defx = 0;
	for (auto const& [field, id]: Range::expand(fields)) {
		if (field->rightSide) {
			auto const vx = Expression().transform(context, field->rightSide);
			if (!vx.direct.isInteger())
				context.error("Expected direct integer here!", field->rightSide);
			defx = vx.direct.getSigned();
			if (defx < 0 && Core::isUnsigned(type.base->basic))
				context.error("Cannot store negative values in unsigned integers!", field->rightSide);
		}
		auto const varName = (field->base.type == LTS_TT_ASSIGN) ? context.pathOf(field->leftSide) : context.pathOf(field);
		auto const varScope = context.declare(varName);
		auto& var = *(varScope->variable = varScope->variable.create());
		var.value = defx++;
		var.context = ExecutionContext::AV2_TCB_EC_COMPILE;
		var.type = scope->type;
		context.pop(varName.size());
	}
	context.pop(name.size());
	context.registerType(scope);
	auto implName = name;
	implName.back() = "::IMPL__" + implName.back();
	context.declare(implName);
	MAKAILIB_DEBUGLN_FULL("Parsing methods...");
	MAKAILIB_DEBUGLN_FULL("Method count: ", methods.size());
	for (auto& method: methods) {
		auto const decl = Expression().transform(context, method);
		auto& fn = *decl.scope->function;
		for (auto& ov: fn.current) {
			if (!ov->staticEntity && (ov->arguments.empty() or ov->arguments[0]->type != scope->type))
				context.error("Missing appropriate [this] parameter!", method);
			if (!ov->staticEntity)
				ov->methodOf = scope->type.asWeak();
		}
		if (scope->subspaces.contains(fn.name))
			context.error("Symbol with this name already exists!", method);
		scope->subspaces[fn.name] = decl.scope;
	}
	context.pop(1);
	return {};
}

ATransformer::Result Return::transform(Context& context, Node::Instance const& node) {
	Expression expr;
	auto const val = expr.transform(context, node->leftSide);
	if (!val.source) {
		context.top()->impl->writeMainLine("ret");
		return {.mayBeEmpty = true};
	}
	if (val.mayBeEmpty) context.error("One or more code paths may not result in a value!", node->leftSide);
	if (val.shouldBePushed())
		context.top()->impl->writeMainLine("push", *val.source);
	else if (val.isStackTop() && val.isCopied()) {
		context.top()->impl->writeMainLine("copy", *val.source, "-> top");
	}
	else if (val.isStackTop() && val.isCopied()) {
		context.top()->impl->writeMainLine("copy", *val.source, "-> top");
	}
	if (node->base.text == "return") context.top()->impl->writeMainLine("ret");
	if (node->base.text == "error") {
		if (!(val.type && val.type->basic == Core::BasicType::AV2_BT_STRING))
			context.error("Expected string value here!", node->leftSide);
		context.top()->impl->writeMainLine("error");
	}
	return {{"move top"}, val.scope, val.type, {}, val.likelihood, nullptr, val.mayBeEmpty};
}

ATransformer::Result Exit::transform(Context& context, Node::Instance const& node) {
	if (node->base.text == "exit")		context.top()->impl->writeMainLine("ret");
	else if (node->base.text == "halt")	context.top()->impl->writeMainLine("halt");
	else context.error("Invalid/Unsupported expression!");
	return {.mayBeEmpty = node->base.text == "exit"};
}

ATransformer::Result Block::transform(Context& context, Node::Instance const& node) {
	ATransformer::Result result;
	context.top()->impl->writeMainLine("begin");
	for (auto const& child: node->children) {
		result = Expression().transform(context, child);
		if (result.scope && result.scope->variable) {
			context.top()->impl->writeMainLine(result.scope->impl->toString());
			if (!result.scope->variable->initializer)		continue;
			if (context.nearestVarScope() == context.root)	continue;
			context.top()->impl->writeMainLine(result.scope->variable->initializer->impl->toString());
			result.scope->variable->initializer = null;
		}
	}
	context.top()->impl->writeMainLine("end");
	return result;
}

ATransformer::Result SubExpression::transform(Context& context, Node::Instance const& node) {
	ATransformer::Result result;
	for (auto const& child: node->children) {
		result = Expression().transform(context, child);
		if (result.scope && result.scope->variable) {
			context.top()->impl->writeMainLine(result.scope->impl->toString());
			if (!result.scope->variable->initializer)		continue;
			if (context.nearestVarScope() == context.root)	continue;
			context.top()->impl->writeMainLine(result.scope->variable->initializer->impl->toString());
			result.scope->variable->initializer = null;
		}
	}
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
			case LTS_TT_BIT_AND:		return a & b;
			case LTS_TT_BIT_OR:			return a | b;
			case LTS_TT_BIT_XOR:		return a ^ b;
			case LTS_TT_LOGIC_AND:		return a && b;
			case LTS_TT_LOGIC_OR:		return a || b;
			case LTS_TT_LOGIC_XOR:		return bool(a) != bool(b);
			case LTS_TT_MODULO:			return a % b;
			default: break;
		}
	}
	if constexpr (Makai::Type::Number<T>) {
		if (tok.type == LTS_TT_IDENTIFIER) {
			auto const id = tok.text;
			if (id == "atan")	return Makai::Math::atan2<double>(a, b);
			if (id == "pow")	return Makai::Math::pow<double>(a, b);
		} else switch (tok.type) {
			case LTS_TT_MODULO:	return Makai::Math::mod<double>(a, b);
			default: break;
		}
	}
	if (tok.type == LTS_TT_IDENTIFIER) {
	} else switch (tok.type) {
		case LTS_TT_INCREMENT:				return a + 1;
		case LTS_TT_DECREMENT:				return a - 1;
		case LTS_TT_PLUS:					return a + b;
		case LTS_TT_MINUS:					return a - b;
		case LTS_TT_STAR:					return a * b;
		case LTS_TT_DIVIDE:					return a / b;
		case LTS_TT_LESS_THAN:				return a < b;
		case LTS_TT_GREATER_THAN:			return a < b;
		case LTS_TT_COMPARE_GREATER_EQUALS:	return a >= b;
		case LTS_TT_COMPARE_LESS_EQUALS:	return a <= b;
		case LTS_TT_COMPARE_EQUALS:			return a == b;
		case LTS_TT_COMPARE_NOT_EQUALS:		return a != b;
		case LTS_TT_ORDER:					return Makai::ValueOrder(a <=> b).order();
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

static ssize likelihoodOf(Node::Instance const& node) {
	switch (node->base.type) {
		case LTS_TT_INCREMENT:
		case LTS_TT_LOGIC_OR: return 1;
		case LTS_TT_DECREMENT:
		case LTS_TT_LOGIC_AND: return -1;
		case LTS_TT_STREAM_INSERT: return 1;
		case LTS_TT_STREAM_EXTRACT: return -1;
		default: [[likely]] return 0;
	}
}

ATransformer::Result PrefixExpression::transform(Context& context, Node::Instance const& node) {
	if (
		node->base.text == "return"
	or	node->base.text == "error"
	)
		return Return().transform(context, node);
	Expression expr;
	auto val = expr.transform(context, node->leftSide);
	if (val.mayBeEmpty) context.error("One or more code paths may not result in a value!", node->leftSide);
	if (
		node->base.text == "likely"
	||	node->base.text == "unlikely"
	) {
		val.likelihood = node->base.text == "likely" ? +1 : -1;
		return val;
	}
	if (!val.source)
		context.error("Invalid expression (Does not result in a value)!", node->leftSide);
	if (val.isCompilable() && node->base.text != "typeof") {
		auto const result = uopDirectResolve(val.direct, node->base);
		if (!result.isUndefined())
			return {{result.toString() + " " + directName(context, result.type())->basicNumberName()}, val.scope, directName(context, result.type()), result, val.likelihood + likelihoodOf(node)};
	}
	if (
		node->base.text == "copy"
	||	node->base.text == "ref"
	||	node->base.text == "move"
	) {
		auto mod = node->base.text;
		MAKAILIB_DEBUGLN_FULL("~~~~~~~~~~~~~ Transfer Mode: [", mod, "]");
		if (mod == "copy") {
			if (!(val.type->flags.isBasic or val.type->flags.isCopyable))
				context.error("Value is not of a copyable type!", node);
			mod = "val";
		}
		MAKAILIB_DEBUGLN_FULL("~~~~~~~~~~~~~ Transfer Mode: [", mod, "]");
		if (val.scope && val.scope->variable) {
			if (!val.scope->isPublic()) context.error("Variable is not public!", node);
			if (val.scope->variable->isConstant && node->base.text != "copy")
				context.error("Constants can only be copied!", node);
			val.scope->variable->setFillState(node->base.text != "move");
		}
		return {{mod + " " + *val.source}, val.scope, val.type, val.direct, val.likelihood, val.parent, val.mayBeEmpty};
	}
	if (val.shouldBePushed()) {
		if (
			node->base.type == LTS_TT_INCREMENT
		||	node->base.type == LTS_TT_DECREMENT
		) context.top()->impl->writeMainLine("push ref", *val.source);
		else context.top()->impl->writeMainLine("push", *val.source);
	} else if (val.isStackTop() && val.isCopied()) {
		context.top()->impl->writeMainLine("copy", *val.source, "-> top");
	}
	if (
		node->base.text == "sizeof"
	||	node->base.text == "countof"
	||	node->base.text == "typeof"
	) {
		context.top()->impl->writeMainLine(node->base.text.sliced(0, -3));
		auto const retType = node->base.text == "typeof" ? context.basicType("type") : context.basicType("uint64");
		return {{"move top"}, retType->scope.asStrong(), retType, 1};
	}
	if (val.type->basic) {
		context.top()->impl->writeMainLine("op", bopName(context, node));
		return {{"move top"}, val.type->scope.asStrong(), val.type, val.direct.undefined(), val.likelihood + likelihoodOf(node)};
	} else return prefixResolve(context, node, val.type);
}

static Makai::String asFastOpQualifier(Core::BasicType const& type, ATransformer::Result const& rhs = {}) {
	Makai::Function<Makai::String(Makai::String const&)> qualifier =
		[] (Makai::String const& in) {
			return "<" + in + ">";
		}
	;
	if (!(rhs.direct.isUndefined() or rhs.direct.isString()))
		qualifier =
			[vx = rhs.direct.toString()] (Makai::String const& in) {
				return "<" + in + ":" + vx + ">";
			}
		;
	switch (type) {
		case Core::BasicType::AV2_BT_INT8:		return qualifier("i8");
		case Core::BasicType::AV2_BT_INT16:		return qualifier("i16");
		case Core::BasicType::AV2_BT_INT32:		return qualifier("i32");
		case Core::BasicType::AV2_BT_INT64:		return qualifier("i64");
		case Core::BasicType::AV2_BT_UINT8:		return qualifier("u8");
		case Core::BasicType::AV2_BT_UINT16:	return qualifier("u16");
		case Core::BasicType::AV2_BT_UINT32:	return qualifier("u32");
		case Core::BasicType::AV2_BT_UINT64:	return qualifier("u64");
		case Core::BasicType::AV2_BT_REAL32:	return qualifier("f32");
		case Core::BasicType::AV2_BT_REAL64:	return qualifier("f64");
		case Core::BasicType::AV2_BT_REAL128:	return qualifier("f128");
		case Core::BasicType::AV2_BT_VECTOR:	return qualifier("vec");
		case Core::BasicType::AV2_BT_MATRIX:	return qualifier("mat");
		case Core::BasicType::AV2_BT_BOOL:		return qualifier("bool");
		case Core::BasicType::AV2_BT_CHAR:		return qualifier("char");
	//	case Core::BasicType::AV2_BT_STRING:	return qualifier("str");
		default: return "";
	}
}

ATransformer::Result PostfixExpression::transform(Context& context, Node::Instance const& node) {
	Expression expr;
	auto const val = expr.transform(context, node->leftSide);
	if (val.mayBeEmpty) context.error("One or more code paths may not result in a value!", node->leftSide);
	if (!val.source)
		context.error("Invalid expression (Does not result in a value)!", node->leftSide);
	if (val.isCompilable() && node->base.text != "typeof") {
		auto const result = uopDirectResolve(val.direct, node->base);
		if (!result.isUndefined())
			return {{result.toString() + " " + directName(context, result.type())->basicNumberName()}, val.type->scope.asStrong(), directName(context, result.type()), result, val.likelihood};
	}
	if (val.shouldBePushed()) {
		if (
			node->base.type == LTS_TT_INCREMENT
		||	node->base.type == LTS_TT_DECREMENT
		) context.top()->impl->writeMainLine("push ref", *val.source);
		else context.top()->impl->writeMainLine("push", *val.source);
	} else if (val.isStackTop() && val.isCopied()) {
		context.top()->impl->writeMainLine("copy", *val.source, "-> top");
	}
	context.top()->impl->writeMainLine("push val top");
	context.top()->impl->writeMainLine("swap");
	if (val.type->basic) {
		context.top()->impl->writeMainLine("op", uopName(context, node) + asFastOpQualifier(*val.type->basic));
		context.top()->impl->writeMainLine("pop");
		return {{"move top"}, val.type->scope.asStrong(), val.type, val.direct.undefined(), val.likelihood};
	} else {
		auto const result = postfixResolve(context, node, val.type);
		context.top()->impl->writeMainLine("pop");
		return result;
	}
}

static bool isLogicOp(Node::Instance const& node) {
	return (
		node->base.type == LTS_TT_LOGIC_OR
	or	node->base.type == LTS_TT_LOGIC_AND
	);
}

static bool isComparison(Node::Instance const& node) {
	return (
		node->base.type == LTS_TT_LESS_THAN
	or	node->base.type == LTS_TT_GREATER_THAN
	or	node->base.type == LTS_TT_COMPARE_GREATER_EQUALS
	or	node->base.type == LTS_TT_COMPARE_LESS_EQUALS
	or	node->base.type == LTS_TT_COMPARE_EQUALS
	or	node->base.type == LTS_TT_COMPARE_NOT_EQUALS
	or	node->base.type == LTS_TT_ORDER
	);
}

Makai::Data::Value directCast(Makai::Data::Value const& value, Core::BasicType const type) {
	switch (type) {
		default: return value;
		case Core::BasicType::AV2_BT_BOOL: return value.getBoolean();
		case Core::BasicType::AV2_BT_INT8:
		case Core::BasicType::AV2_BT_INT16:
		case Core::BasicType::AV2_BT_INT32:
		case Core::BasicType::AV2_BT_INT64: return value.getSigned();
		case Core::BasicType::AV2_BT_UINT8:
		case Core::BasicType::AV2_BT_UINT16:
		case Core::BasicType::AV2_BT_UINT32:
		case Core::BasicType::AV2_BT_UINT64: return value.getUnsigned();
		case Core::BasicType::AV2_BT_REAL32:
		case Core::BasicType::AV2_BT_REAL64:
		case Core::BasicType::AV2_BT_REAL128: return value.getReal();
		case Core::BasicType::AV2_BT_STRING: return (value.isString() ? value.getString() : value.toString());
	}
	return value;
}

ATransformer::Result specialDirectResolve(
	ATransformer::Context& context,
	ATransformer::Result value,
	Namespace::TypeRef type,
	Makai::UTF8String const& kw,
	Node::Instance const& node
) {
	ATransformer::Result out;
	if (kw == "as") {
		if (TypeDecl::stronger(value.type, type)) {
			Makai::Data::Value val;
			if (type->basic)
				val = directCast(value.direct, *type->basic);
			else val = value.direct;
			return {value.source, value.scope, type, value.direct, value.likelihood};
		}
		else context.error("Value cannot be converted to the given type!", node);
	} else if (kw == "is") {
		auto const match = value.type == type;
		return {{Makai::toString(match)}, context.basicType("bool")->scope.asStrong(), context.basicType("bool"), match};
	} else context.error("Unsupported direct operation!", node);
	return out;
}

ATransformer::Result InfixExpression::transform(Context& context, Node::Instance const& node) {
	auto const sseAnd = "__sce_and_" + node->name();
	auto const sseOr = "__sce_or_" + node->name();
	Expression expr;
	bool lhsHasBeenPushed = false;
	auto const lhs = expr.transform(context, node->leftSide);
	if (lhs.mayBeEmpty) context.error("One or more code paths may not result in a value!", node->leftSide);
	bool const comparison = isComparison(node);
	if (!lhs.source)
		context.error("Invalid expression (Does not result in a value)!", node->leftSide);
	if (lhs.isCompilable() && isLogicOp(node)) {
		if (lhs.direct.isFalsy() && node->base.type == LTS_TT_LOGIC_AND) return lhs;
		if (lhs.direct.isTruthy() && node->base.type == LTS_TT_LOGIC_OR) return lhs;
	}
	if (lhs.shouldBePushed() && !lhs.isCompilable()) {
		lhsHasBeenPushed = true;
		context.impl()->writeMainLine("push", *lhs.source);
	} else if (lhs.isStackTop() && lhs.isCopied()) {
		lhsHasBeenPushed = true;
		context.impl()->writeMainLine("copy", *lhs.source, "-> top");
	} else if (lhs.isStackTop()) lhsHasBeenPushed = true;
	if (isLogicOp(node) && !lhs.isCompilable()) {
		if (node->base.type == LTS_TT_LOGIC_AND) {
			context.impl()->writeMainLine("push val top");
			context.impl()->writeMainLine("jump if false", sseAnd);
		} if (node->base.type == LTS_TT_LOGIC_OR) {
			context.impl()->writeMainLine("push val top");
			context.impl()->writeMainLine("jump if true", sseOr);
		}
	}
	if (
		node->base.text == "as"
	||	node->base.text == "is"
	) {
		auto const t = TypeRequest().transform(context, node->rightSide);
		if (lhs.isCompilable())
			return specialDirectResolve(context, lhs, t.type, node->base.text, node->leftSide);
		if (node->base.text == "is") {
			auto const retType = context.basicType("bool");
			context.top()->impl->writeMainLine(node->base.text, t.type->name);
			return {{"move top"}, retType->scope.asStrong(), retType};
		}
		auto const retType = t.type;
		if (t.type->basic != Core::BasicType::AV2_BT_ANY && !TypeDecl::stronger(t.type, retType))
			context.error("Value's type cannot be converted to given type!", node);
		context.top()->impl->writeMainLine(node->base.text, t.type->name);
		return {{"move top"}, retType->scope.asStrong(), retType};
	}
	auto const rhs = expr.transform(context, node->rightSide);
	if (rhs.mayBeEmpty) context.error("One or more code paths may not result in a value!", node->rightSide);
	if (lhs.isCompilable() && isLogicOp(node)) {
		if (lhs.direct.isTruthy() && node->base.type == LTS_TT_LOGIC_AND) return rhs;
		if (lhs.direct.isFalsy() && node->base.type == LTS_TT_LOGIC_OR) return rhs;
	}
	if (!rhs.source)
		context.error("Invalid expression (Does not result in a value)!", node->rightSide);
	auto const likelihood = lhs.likelihood + rhs.likelihood + likelihoodOf(node);
	MAKAILIB_DEBUGLN_FULL("Left-Type: ", lhs.type->name);
	MAKAILIB_DEBUGLN_FULL("Right-Type: ", rhs.type->name);
	MAKAILIB_DEBUGLN_FULL("Left-Type = ", lhs.direct.toString());
	MAKAILIB_DEBUGLN_FULL("Right-Type = ", rhs.direct.toString());
	if (lhs.isCompilable() && rhs.isCompilable()) {
		auto result = bopDirectResolve(lhs.direct, rhs.direct, node->base);
		if (!result.isUndefined()) {
			result = directCast(result, *TypeDecl::stronger(lhs.type, rhs.type)->basic);
			return {
				{result.toString() + " " + directName(context, result.type())->basicNumberName()},
				directName(context, result.type())->scope.asStrong(),
				directName(context, result.type()),
				result,
				likelihood
			};
		}
	}
	if (!lhsHasBeenPushed)
		context.top()->impl->writeMainLine("push", *lhs.source);
	if (
		(lhs.type != rhs.type)
	or	!(rhs.isCompilable() and !rhs.direct.isString())
	) {
		if (rhs.shouldBePushed())
			context.top()->impl->writeMainLine("push", *rhs.source);
		else if (rhs.isStackTop() && rhs.isCopied()) {
			context.top()->impl->writeMainLine("copy", *rhs.source, "-> top");
		}
	}
	if (!lhsHasBeenPushed && !lhs.isStackTop() && rhs.isStackTop())
		context.top()->impl->writeMainLine("swap");
	MAKAILIB_DEBUGLN_FULL("LHS = [", lhs.source.value(), "]");
	MAKAILIB_DEBUGLN_FULL("RHS = [", rhs.source.value(), "]");
	MAKAILIB_DEBUGLN_FULL("LHS Type: ", lhs.type ? lhs.type->name : "NO_TYPE");
	MAKAILIB_DEBUGLN_FULL("RHS Type: ", rhs.type ? rhs.type->name : "NO_TYPE");
	if (isLogicOp(node)) {
		if (node->base.type == LTS_TT_LOGIC_AND)
			context.impl()->writeMainLine("@target", sseAnd);
		if (node->base.type == LTS_TT_LOGIC_OR)
			context.impl()->writeMainLine("@target", sseOr);
		auto const t = TypeDecl::stronger(lhs.type, rhs.type);
		if (!t) context.error("Type mismatch in infix expression!", node);
		return {{"move top"}, t->scope.asStrong(), t, lhs.direct.undefined(), likelihood};
	} else if (auto const t = TypeDecl::stronger(lhs.type, rhs.type)) {
		MAKAILIB_DEBUGLN_FULL("Stronger: ", t->name);
		if (t->flags.isBasic) {
			if (lhs.type->basic == rhs.type->basic)
				context.top()->impl->writeMainLine(comparison ? "cmp" : "op", bopName(context, node) + asFastOpQualifier(*t->basic, rhs));
			else
				context.top()->impl->writeMainLine(comparison ? "cmp" : "op", bopName(context, node));
			return {{"move top"}, t->scope.asStrong(), t, lhs.direct.undefined(), likelihood};
		} else if (t->flags.isEnum && comparison) {
			if (lhs.type->base->basic == rhs.type->base->basic)
				context.top()->impl->writeMainLine("cmp", bopName(context, node) + asFastOpQualifier(*t->base->basic, rhs));
			else
				context.top()->impl->writeMainLine("cmp", bopName(context, node));
			return {{"move top"}, t->scope.asStrong(), t, lhs.direct.undefined(), likelihood};
		} else return infixResolve(context, node, t);
	} else if (
		lhs.type->flags.isArray
	&&	node->base.type == LTS_TT_STREAM_INSERT
	) {
		if (auto const t = TypeDecl::stronger(lhs.type->base, rhs.type))
			context.top()->impl->writeMainLine("op apush");
		else context.error("Array element type mismatch!", node);
		return {{"move top"}, t->scope.asStrong(), t, lhs.direct.undefined(), likelihood};
	}
	context.error("Type mismatch in infix expression!", node);
}

ATransformer::Result Direct::transform(Context& context, Node::Instance const& node) {
	if (!node || node->content != Node::Content::AV2_TANC_VALUE)
		context.error("Expected value here!", node);
	auto value = node->value.toString();
	ATransformer::Result out;
	Namespace::TypeRef type;
	type = context.basicTypeOf(node->value);
	if (node->value.isBoolean())
		out.likelihood	= 0;
	else if (node->value.isString())
		out.likelihood	= 1;
	else if (node->value.isUnsigned()) {
		out.likelihood	= 1;
		value += " u64";
	} else if (node->value.isSigned()) {
		out.likelihood	= 1;
		value += " i64";
	} else if (node->value.isReal()) {
		out.likelihood	= 1;
		value += " f64";
	} else if (node->value.isNull())
		return {.source = {"nil"}, .direct = null};
	else context.error("Invalid constant!", node);
	out.source		= value;
	out.scope		= type->scope.asStrong();
	out.type		= type;
	out.direct		= node->value;
	return out;
}

ATransformer::Result PathExpression::transform(Context& context, Node::Instance const& node) {
	UTF8StringList path;
	ATransformer::Result result;
	if (node->content == Node::Content::AV2_TANC_NAME) {
		auto const [path, ns] = resolve(context, node);
		if (!ns)
			context.error("Symbol does not exist!", node);
		if (!ns->isPublic()) context.error("Variable is not public!", node);
		result.source = addToStack(context, ns.asStrong(), node);
		if (ns->variable) {
			result.type		= ns->variable->type.asStrong();
			result.scope	= ns->variable->scope.asStrong();
			if (ns->variable->isCompiled())
				result.direct = ns->variable->value;
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
		if (nsx.shouldBePushed())
			context.top()->impl->writeMainLine("push", *nsx.source);
		else if (nsx.isStackTop() && nsx.isCopied())
			context.top()->impl->writeMainLine("copy", *nsx.source, "-> top");
		return resolveSubfield(context, node, result.scope, path.front());
	} else if (node->leftSide->content == Node::Content::AV2_TANC_FAILABLE_PATH) {
		auto const success =  "__success_" + node->name();
		auto const fail =  "__fail_" + node->name();
		auto const nsx = PathExpression().transform(context, node->leftSide);
		path = context.pathOf(node->value.getString()).reverse();
		result = nsx;
		if (nsx.shouldBePushed())
			context.top()->impl->writeMainLine("push", *nsx.source);
		else if (nsx.isStackTop() && nsx.isCopied())
			context.top()->impl->writeMainLine("copy", *nsx.source, "-> top");
		context.top()->impl->writeMainLine("push val top");
		context.top()->impl->writeMainLine("jump if nil", fail);
		auto const ox = resolveSubfield(context, node, result.scope, path.front());
		context.top()->impl->writeMainLine("@target", fail);
		return ox;
	} else if (node->leftSide->content == Node::Content::AV2_TANC_NAME) {
		path = context.pathOf(node->leftSide);
		auto const ns = context.resolve(path);
		path = context.pathOf(node->value.getString()).reverse();
		if (!ns)
			context.error("Symbol does not exist!", node);
		return resolveSubfield(context, node, ns, path.front());
	}
	if (!result.scope->subspaces.contains(path.front()))
		context.error("Subpath type doesn't contain the given member!", node->leftSide);
	return resolveSubfield(context, node, result.scope, path.front());
}

ATransformer::Result Expression::transform(Context& context, Node::Instance const& node) {
	if (!node) return {};
	MAKAILIB_DEBUGLN_FULL("Expression Type: ", Node::asString(node->content));
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
		case Node::Content::AV2_TANC_SUBSCRIPT:			return Subscript().transform(context, node);
		case Node::Content::AV2_TANC_TYPE_EXTENSION:	return TypeExtension().transform(context, node);
		case Node::Content::AV2_TANC_EMPTY_DECAY:		return NullDecay().transform(context, node);
		case Node::Content::AV2_TANC_EVAL_BLOCK:		return Evaluation().transform(context, node);
		case Node::Content::AV2_TANC_SWITCH:			return SwitchMatch().transform(context, node);
		case Node::Content::AV2_TANC_NAME:
		case Node::Content::AV2_TANC_FAILABLE_PATH:
		case Node::Content::AV2_TANC_PATH:				return PathExpression().transform(context, node);
		default: context.error("Unsupported expression!", node);
	}
}

ATransformer::Result TypeRequest::transform(Context& context, Node::Instance const& node) {
	ATransformer::Result rest;
	if (node->content == Node::Content::AV2_TANC_ARRAY)
		return ArrayTypeDecl().transform(context, node);
	if (node->content == Node::Content::AV2_TANC_NULLABLE_DECL)
		return NullableTypeDecl().transform(context, node);
	if (node->content == Node::Content::AV2_TANC_DECLARATION)
		return StructureDecl().transform(context, node);
	if (node->content == Node::Content::AV2_TANC_FN_PROTOTYPE)
		return FunctionTypeDecl().transform(context, node);
	auto const t = context.fetch(node)->type;
	if (!t) context.error("Type does not exist!", node);
	++t->uses;
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
			MAKAILIB_DEBUGLN_FULL("~~~~~~~~~~~~ Attribute Field: ", name);
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
	MAKAILIB_DEBUGLN_FULL("<attrib-expr>");
	auto const expr = Expression().transform(context, node->rightSide);
	if (!expr.scope) context.error("Expected scope here!", node->rightSide);
	Makai::Dictionary<Metadata::Instance> attributes;
	MAKAILIB_DEBUGLN_FULL("Resolving attributes for expression '", expr.scope->name, "'...");
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
	MAKAILIB_DEBUGLN_FULL("</attrib-expr>");
	return expr;
}

static Makai::UTF8String overloadName(Makai::List<Namespace::VariableRef> const& args) {
	Makai::UTF8String name;
	for (auto const& arg: args)
		name += "_" + arg->type->name;
	return name;
}

struct FunctionArgument {
	Node::Instance decl;
	Node::Instance arg;
};

static Makai::Nullable<FunctionArgument> resolveFunctionArgument(Node::Instance const& root, Node::Instance const& node) {
	if (node->content == Node::Content::AV2_TANC_ATTRIBUTE)
		return resolveFunctionArgument(root, node->rightSide);
	else if (
		node->base.type == LTS_TT_COLON
	or	node->base.type == LTS_TT_DECLARE
	) return FunctionArgument{root, node};
	else return null;
}

ATransformer::Result FunctionDecl::transform(Context& context, Node::Instance const& node) {
	auto [path, scope] = resolve(context, node->leftSide);
	if (scope) {
		if (!scope->isPureNamespace() && !scope->function)
			context.error("Symbol is already defined as a different kind!", node);
		context.scopeStack.pushBack(scope);
	} else scope = context.declare(path);
	if (!scope->function) {
		scope->function = scope->function.create();
		scope->function->name = path.join("_");
		scope->function->pureName = path.back();
	}
	auto const proto = node->middle;
	auto const fn = scope->function;
	fn->current.clear();
	List<Node::Instance> required, optional;
	for (auto const& arg: proto->children) {
		auto const desc = resolveFunctionArgument(arg, arg);
		if (!desc)
			context.error("Expected variable declaration here!", arg);
		auto const [full, base] = desc.value();
		if (base->rightSide)
			optional.pushBack(full);
		else if (optional.empty())
			required.pushBack(full);
		else context.error("Optional arguments must occur AFTER required ones!", full);
	}
	Namespace::TypeRef retType;
	if (proto->leftSide) {
		auto const ret = Expression().transform(context, proto->leftSide);
		retType = ret.type;
		if (!retType && !ret.scope)
			context.error("Return type does not exist!", proto->leftSide);
		retType = ret.scope->type;
		if (!retType)
			context.error("Return type does not exist!", proto->leftSide);
	}
	auto const impl = context.declare(Makai::UTF8StringList::from("<impl>" + node->name()));
	Function::OverloadRef current = current.create(), prev, first = current;
	current->fullImpl = first->fullImpl = first.asWeak();
	fn->current.pushBack(current);
	current->scope = impl.asWeak();
	for (auto& arg: required) {
		auto const ax = Expression().transform(context, arg);
		current->arguments.pushBack(ax.scope->variable);
		ax.scope->variable->fill();
	}
	current->entry = "__"+ fn->name + overloadName(current->arguments) + node->name();
	impl->impl->writePreLine("@def", current->entry, ":");
	impl->impl->writePreLine("enter", required.size() + optional.size());
	MAKAILIB_DEBUGLN_FULL("Overload: ", current->entry);
	current->scope = impl.asWeak();
	auto const vx = fn->overloadFromVariables(current->arguments);
	if (vx && vx->hasImplementation)
		context.error("An overload already exists for this function!", node);
	fn->overloads.pushBack(current);
	List<Implementation::Instance> ovImpl;
	ovImpl.pushBack(impl->impl);
	for (auto const [opt, i]: Range::expand(optional)) {
		auto const overload = context.declare(Makai::UTF8StringList::from("<impl>" + node->name()));
		if (prev)
			prev->scope = nullptr;
		prev = current;
		current = current.create();
		current->fullImpl = first->fullImpl;
		current->decl = node->rightSide;
		fn->current.pushBack(current);
		current->arguments = prev->arguments;
		overload->varc = current->arguments.size();
		auto const ox = Expression().transform(context, opt);
		current->arguments.pushBack(ox.scope->variable);
		auto const fx = fn->overloadFromVariables(current->arguments);
		if (fx && fx->hasImplementation)
			context.error("An overload already exists for this function!", node);
		current->entry = "__" + fn->name + overloadName(current->arguments) + node->name();
		MAKAILIB_DEBUGLN_FULL("Overload: ", current->entry);
		overload->impl->writePreLine(ox.scope->variable->initializer->impl->toString());
		context.top()->impl->writeMainLine("push", ox.source.value());
		overload->impl->writePreLine("@def", current->entry, ":");
		ox.scope->variable->initializer = nullptr;
		ox.scope->variable->fill();
		fn->overloads.pushBack(current);
		ovImpl.pushBack(overload->impl);
		current->hasImplementation = true;
	}
	auto const argc = required.size() + optional.size();
	if (node->rightSide) {
		current->decl = node->rightSide;
		if (ovImpl.size())
			current = current.create();
		context.functionStack.pushBack(current);
		impl->impl->writePreLine("bind ref", argc, "[0 -> 0]");
		impl->impl->writePreLine("clear", argc);
		auto const def = Expression().transform(context, node->rightSide);
		if (retType && def.type && retType != def.type)
			context.error("Expression return type does not match function return type!", node);
		else if (!retType) {
			if (!def.type)
				retType = context.basicType("void");
			else retType = def.type;
		}
		current->hasImplementation = true;
		if (ovImpl.empty())
			current->scope = context.top().asWeak();
		context.functionStack.popBack();
	} else {
		impl->impl->writePreLine("blit ref", argc, "[0 -> 0]");
		impl->impl->writeMainLine("call", current->entry);
		if (ovImpl.size() == 1)
			current->scope = nullptr;
		first->scope = nullptr;
	}
	if (!retType)
		context.error("Missing function return type!", node);
	//	retType = context.basicType("void");
	impl->impl->writePostLine("exit");
	impl->impl->writePostLine("ret");
	for (usize _ = 0; _ < optional.size() + 1; ++_)
		impl->impl->writePostLine("@def .");
	context.pop(1 + optional.size());
	for (auto& ov: fn->current)
		ov->result = retType;
	current->result = retType;
	while (ovImpl.size() > 1) {
		auto const top = ovImpl.popBack();
		ovImpl.back()->writeMainLine(top->compose()->toString());
	}
	context.registerFunction(scope);
	context.pop(path.size());
	return {.scope = scope};
}

ATransformer::Result Assignment::transform(Context& context, Node::Instance const& node) {
	auto const ndecl = "__exists_" + node->name();
	if (node->middle) {
		auto lhs = Expression().transform(context, node->leftSide);
		if (lhs.mayBeEmpty) context.error("One or more code paths may not result in a value!", node->leftSide);
		if (!(lhs.source && (lhs.type->flags.isArray or lhs.type->basic == Core::BasicType::AV2_BT_VECTOR)))
			context.error("Expected indexable value here!", node->leftSide);
		if (lhs.shouldBePushed())
			context.top()->impl->writeMainLine("push", *lhs.source);
		else if (lhs.isStackTop() && lhs.isCopied()) {
			context.top()->impl->writeMainLine("copy", *lhs.source, "-> top");
		}
		auto i = Expression().transform(context, node->middle);
		if (i.mayBeEmpty) context.error("One or more code paths may not result in a value!", node->middle);
		if (!(i.source && TypeDecl::stronger(i.type, context.basicType("uint64"))))
			context.error("Expected integer here!", node->middle);
		if (i.direct.isUndefined() && i.shouldBePushed())
			context.top()->impl->writeMainLine("push", *i.source);
		if (node->base.type == LTS_TT_NULL_ASSIGN) {
			context.top()->impl->writeMainLine("push stack[-1]");
			if (i.direct.isUndefined()) {
				if (lhs.type->basic == Core::BasicType::AV2_BT_VECTOR)
					context.top()->impl->writeMainLine("mod" + asFastOpQualifier(*i.type->basic, {.direct = 4}));
				context.top()->impl->writeMainLine("push stack[-1]");
				context.top()->impl->writeMainLine("dyn get");
			} else if (i.direct.isInteger()) {
				if (lhs.type->basic == Core::BasicType::AV2_BT_VECTOR && i.direct.getUnsigned() > 3)
					context.error("Index range must be between 0 and 3!", node->rightSide);
				context.top()->impl->writeMainLine("get [", i.direct.getUnsigned(), "]");
			}
			else context.error("Expected integer here!", node->middle);
			context.top()->impl->writeMainLine("jump if not empty", ndecl);
		}
		auto const rhs = Expression().transform(context, node->rightSide);
		if (rhs.mayBeEmpty) context.error("One or more code paths may not result in a value!", node->rightSide);
		if (!rhs.source)
			context.error("Expected value here!", node->rightSide);
		if (auto const t = TypeDecl::stronger(lhs.type->base, rhs.type)) {
			if (t != lhs.type->base)
				context.error("Right-hand type is weaker than left-hand type!", node);
			if (rhs.shouldBePushed())
				context.top()->impl->writeMainLine("push", *rhs.source);
			else if (rhs.isStackTop() && rhs.isCopied()) {
				context.top()->impl->writeMainLine("copy", *rhs.source, "-> top");
			}
			if (i.direct.isUndefined()) {
				if (lhs.type->basic == Core::BasicType::AV2_BT_VECTOR)
					context.top()->impl->writeMainLine("mod" + asFastOpQualifier(*i.type->basic, {.direct = 4}));
				context.top()->impl->writeMainLine("dyn set");
			}
			else if (i.direct.isInteger()) {
				if (lhs.type->basic == Core::BasicType::AV2_BT_VECTOR && i.direct.getUnsigned() > 3)
					context.error("Index range must be between 0 and 3!", node->rightSide);
				context.top()->impl->writeMainLine("set [", i.direct.getUnsigned(), "]");
			}
			else context.error("Expected integer here!", node->middle);
			if (node->base.type == LTS_TT_NULL_ASSIGN)
				context.top()->impl->writeMainLine("@target", ndecl);
			return {{"move top"}, lhs.scope, t, rhs.direct, rhs.likelihood};
		} else context.error("Type mismatch in assignment expression!", node);
	}
	auto lhs = Expression().transform(context, node->leftSide);
	if (lhs.mayBeEmpty) context.error("One or more code paths may not result in a value!", node->leftSide);
	if (lhs.scope && lhs.scope->property) {
		if (!lhs.scope->property->setter)
			context.error("Cannot set a read-only property!", node->leftSide);
		auto& prop = *lhs.scope->property;
		context.impl()->main.popBack();
		if (lhs.shouldBePushed())
			context.top()->impl->writeMainLine("push", *lhs.source);
		else if (lhs.isStackTop() && lhs.isCopied())
			context.top()->impl->writeMainLine("copy", *lhs.source, "-> top");
		auto const rhs = Expression().transform(context, node->rightSide);
		auto const set = prop.setter->overloadFromTypes({lhs.parent, rhs.type}, Function::FuzzySearch::AV2_TCF_FS_ALL_EXCEPT_FIRST);
		if (!set)
			context.error("No suitable setter for expression type", node->rightSide);
		if (rhs.shouldBePushed())
			context.top()->impl->writeMainLine("push", *rhs.source);
		else if (rhs.isStackTop() && rhs.isCopied())
			context.top()->impl->writeMainLine("copy", *rhs.source, "-> top");
		context.top()->impl->writeMainLine("call", set->entry);
	}
	if (lhs.isStackTop() && lhs.isCopied())
		context.top()->impl->writeMainLine("copy", *lhs.source, "-> top");
	auto const rhs = Expression().transform(context, node->rightSide);
	if (rhs.mayBeEmpty) context.error("One or more code paths may not result in a value!", node->rightSide);
	if (lhs.parent) {
		context.top()->impl->main.popBack();
		if (lhs.isStackTop() && lhs.isCopied())
			context.top()->impl->main.popBack();
		if (lhs.shouldBePushed())
			context.top()->impl->writeMainLine("push", *lhs.source);
		if (node->base.type == LTS_TT_NULL_ASSIGN) {
			context.top()->impl->writeMainLine("push val top");
			context.top()->impl->writeMainLine("jump if not empty", ndecl);
		}
		if (rhs.shouldBePushed())
			context.top()->impl->writeMainLine("push", *rhs.source);
		else if (rhs.isStackTop() && rhs.isCopied())
			context.top()->impl->writeMainLine("copy", *rhs.source, "-> top");
		context.top()->impl->writeMainLine("set [", lhs.likelihood, "]");
		return {{"move top"}, lhs.scope, lhs.type, rhs.direct, rhs.likelihood};
	}
	if (lhs.isCompilable() && !lhs.scope) context.error("Cannot assign a value to a direct value!", node->leftSide);
	if (node->base.type == LTS_TT_NULL_ASSIGN) {
		if (lhs.shouldBePushed())
			context.top()->impl->writeMainLine("push", *lhs.source);
		else if (lhs.isStackTop() && lhs.isCopied()) {
			context.top()->impl->writeMainLine("copy", *lhs.source, "-> top");
		}
		context.top()->impl->writeMainLine("jump if not empty", ndecl);
	}
	if (rhs.direct.isNull() && !lhs.type->flags.isNullable)
		context.error("Expected non-null value here!", node->rightSide);
	if (rhs.type && rhs.type->flags.isNullable && !lhs.type->flags.isNullable)
		context.error("Nullable values cannot directly convert to non-nullable values!", node->rightSide);
	if (lhs.type->flags.isNullable) {
		if (lhs.scope && lhs.scope->variable)
			lhs.scope->variable->fill();
		if (lhs.type != rhs.type && lhs.type->base != rhs.type)
			context.error("Type mismatch in assignment expression!", node->rightSide);
		if (lhs.isStackTop() && rhs.isStackTop())
			lhs.source = UTF8String("move stack[-1]");
		if (*lhs.source != *rhs.source)
			context.top()->impl->writeMainLine("copy", *rhs.source, "->", *lhs.source);
		if (!rhs.shouldBePushed())
			context.top()->impl->writeMainLine("pop");
		if (node->base.type == LTS_TT_NULL_ASSIGN)
			context.top()->impl->writeMainLine("@target", ndecl);
		return {lhs.source, lhs.scope, rhs.type, rhs.direct, rhs.likelihood};
	}
	if (auto const t = TypeDecl::stronger(lhs.type, rhs.type)) {
		if (t != lhs.type)
			context.error("Right-hand type is weaker than left-hand type!", node);
		if (lhs.isStackTop() && rhs.isStackTop())
			lhs.source = UTF8String("move stack[-1]");
		if (*lhs.source != *rhs.source)
			context.top()->impl->writeMainLine("copy", *rhs.source, "->", *lhs.source);
		if (!rhs.shouldBePushed())
			context.top()->impl->writeMainLine("pop");
		if (node->base.type == LTS_TT_NULL_ASSIGN)
			context.top()->impl->writeMainLine("@target", ndecl);
		return {lhs.source, lhs.scope, t, rhs.direct, rhs.likelihood};
	} else context.error("Type mismatch in assignment expression!", node);
}

ATransformer::Result Import::transform(Context& context, Node::Instance const& node) {
	auto const path = context.pathOf(node->leftSide);
	auto const fpath = path.join("/").toString();
	MAKAILIB_DEBUG_FULL("Import Path: ");
	for (auto& p: path)
		MAKAILIB_DEBUG_FULL("/", p);
	MAKAILIB_DEBUGLN_FULL("");
	auto const subinter = importer(fpath);
	// This is for testing purposes (probably (I don't know if I'll replace it))
	if (!subinter.content) return {};
	for (auto& [name, imp]: context.root->subspaces["0__@Tx0_IMPORTS"]->subspaces)
		if (imp == subinter.content) return {.scope = subinter.content};
	context.registerImport(subinter.content);
	return {.scope = subinter.content};
}

ATransformer::Result PropertyDecl::transform(Context& context, Node::Instance const& node) {
	auto path = context.pathOf(node->middle);
	auto scope = context.top()->resolve(path);
	if (scope && !scope->property)
		context.error("Redeclaration of previously-declared symbol!", node->middle);
	if (!scope) {
		scope = context.declare(path);
		scope->property = scope->property.create();
	} else {
		context.scopeStack.pushBack(scope);
		path = decltype(path)::from(path.back());
	}
	auto& property = *scope->property;
	if (node->leftSide)
		property.getter = PropertyGetter().transform(context, node->leftSide).scope->function;
	if (node->rightSide)
		property.setter = PropertySetter().transform(context, node->leftSide).scope->function;
	context.pop(path.size());
	return {.scope = scope};
}

ATransformer::Result PropertyGetter::transform(Context& context, Node::Instance const& node) {
	auto const decl = FunctionDecl().transform(context, node);
	if (decl.scope->function->current.size() != 1)
		context.error("Default arguments are not allowed in property declarations!", node);
	if (decl.scope->function->current.back()->arguments.size() != 1)
		context.error("Getters can only have exactly one argument!");
	return decl;
}

ATransformer::Result PropertySetter::transform(Context& context, Node::Instance const& node) {
	auto const decl = FunctionDecl().transform(context, node);
	if (decl.scope->function->current.size() != 1)
		context.error("Default arguments are not allowed in property declarations!", node);
	if (decl.scope->function->current.back()->arguments.size() != 2)
		context.error("Getters can only have exactly two arguments!");
	return decl;
}

ATransformer::Result NamespaceDecl::transform(Context& context, Node::Instance const& node) {
	auto const path = context.pathOf(node->leftSide);
	if ((context.top()->resolve(path) && !context.top()->resolve(path)->isPureNamespace()))
		context.error("Redeclaration of previously-declared symbol!", node->leftSide);
	auto const scope = context.declare(path);
	scope->declaredAsNamespace = true;
	Block().transform(context, node->rightSide);
	context.pop(path.size());
	return {.scope = scope, .mayBeEmpty = false};
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
		if (node->base.text == "enum")
			return EnumDecl().transform(context, node);
	}
	context.error("Invalid declaration!", node);
}

static Makai::Data::Value callDirect(ATransformer::Context& context, Function::Overload& ov, Makai::Data::Value::ArrayType const& args) {
	// TODO: Direct functions
}

ATransformer::Result Call::transform(Context& context, Node::Instance const& node) {
	MAKAILIB_DEBUGLN_FULL("Left-side: ", node->leftSide->base.text);
	auto const dx = context.impl()->main.size();
	auto const fn = Expression().transform(context, node->leftSide);
	if (!fn.scope)
		context.error("Symbol does not exist!", node->leftSide);
	MAKAILIB_DEBUGLN_FULL(fn.scope->name);
	if (!fn.scope->function)
		context.error("Symbol is not a function!", node->leftSide);
	auto& f = *fn.scope->function;
	Function::ArgTypes args;
	usize const memspot = context.top()->impl->main.size();
	context.top()->impl->writeMainLine("");
	Makai::Data::Value::ArrayType directArgs;
	bool runtimeCall = false;
	for (auto const& arg: node->children) {
		auto const expr = Expression().transform(context, arg);
		if (expr.mayBeEmpty) context.error("One or more code paths may not result in a value!", arg);
		if (!expr.source)
			context.error("Expected value here!", arg);
		if (expr.shouldBePushed())
			context.top()->impl->writeMainLine("push", *expr.source);
		else if (expr.isStackTop() && expr.isCopied()) {
			context.top()->impl->writeMainLine("copy", *expr.source, "-> top");
		}
		args.pushBack(expr.type);
		if (expr.isCompilable())
			directArgs.pushBack(expr.direct);
		else runtimeCall = true;
	}
	MAKAILIB_DEBUGLN_FULL("Function: ", f.name);
	MAKAILIB_DEBUGLN_FULL("Overloads: [ ");
	for (auto const& ov: f.overloads)
		MAKAILIB_DEBUGLN_FULL("  ", ov->prototype(), ov->variadic ? "<variadic> " : " ");
	MAKAILIB_DEBUGLN_FULL("]");
	auto memArgs = args;
	if (fn.type)
		memArgs.reverse().pushBack(fn.type).reverse();
	auto const ovLookupSig = args.toList<UTF8String>([] (auto const& e) {return e->name;}).join(" ");
	auto const ovMemLookupSig = memArgs.toList<UTF8String>([] (auto const& e) {return e->name;}).join(" ");
	MAKAILIB_DEBUGLN_FULL("Looking for: [", ovLookupSig, "] or [", ovMemLookupSig, "]");
	if (!(
		f.overloadFromTypes(args, Function::FuzzySearch::AV2_TCF_FS_ALL_ARGS)
	or	f.overloadFromTypes(memArgs, Function::FuzzySearch::AV2_TCF_FS_ALL_EXCEPT_FIRST)
	))
		context.error("No suitable overload exists!", node);
	auto ovf = f.overloadFromTypes(args, Function::FuzzySearch::AV2_TCF_FS_ALL_ARGS);
	bool isMemFn = false;
	if (!ovf) {
		ovf = f.overloadFromTypes(memArgs, Function::FuzzySearch::AV2_TCF_FS_ALL_EXCEPT_FIRST);
		if (!(ovf && !ovf->staticEntity))
			context.error("No suitable overload exists!", node);
		isMemFn = true;
	}
	auto& ov = *ovf;
	++ov.fullImpl->uses;
	if (isMemFn) {
		if (fn.isCompilable())
			directArgs.insert(fn.direct, 0);
		auto const pushAction = (fn.shouldBePushed()) ? Makai::toString("push ", *fn.source)  : "";
		auto const copyAction = (fn.isStackTop() && fn.isCopied()) ? Makai::toString("\ncopy ", *fn.source, " -> top")  : "";
		context.top()->impl->main[memspot] = pushAction + copyAction;
	}
	if (!runtimeCall && ov.variant.context > ExecutionContext::AV2_TCB_EC_RUNTIME) {
		auto const ret = callDirect(context, ov, directArgs);
		context.impl()->main.eraseRange(-dx, -1);
		if (ret.isUndefined())
			return {.type = context.basicType("void")};
		else if (ret.isObject())
			return Expression().transform(context, context.evaluate(ret["eval"].getString()));
		else return {{ret.isNull() ? Makai::String("nil") : (ret.toString() + " " + directName(context, ret.type())->basicNumberName())}, nullptr, context.basicTypeOf(ret), ret};
	} else if (ov.variant.context < ExecutionContext::AV2_TCB_EC_COMPILE) {
		if (ov.variadic) {
			auto const vat = ov.arguments.back()->type;
			if (args.size() < ov.arguments.size()) {
				context.top()->impl->writeMainLine("new[",vat->name, ":0]");
			} else {
				context.top()->impl->writeMainLine("new[",vat->name, ":", (args.size() - ov.arguments.size()) + 1, "]");
				context.top()->impl->writeMainLine("create", vat->name);
			}
		}
		context.top()->impl->writeMainLine("call", ov.entry);
	}
	else context.error("It is forbidden to call a direct function with indirect arguments!", node);
	if (context.functionStack.size() && ov.variant.context == ExecutionContext::AV2_TCB_EC_RUNTIME) {
		auto& ctx = context.functionStack.back()->variant.context;
		if (ctx < ExecutionContext::AV2_TCB_EC_MIXED)
			context.functionStack.back()->variant.context = ExecutionContext::AV2_TCB_EC_RUNTIME;
		else context.error("Cannot call indirect functions inside mixed or direct functions!", node);
	}
	return {.source = {"move top"}, .scope = ov.result->scope.asStrong(), .type = ov.result, .mayBeEmpty = Makai::Cast::as<bool>(ov.result->flags.hasNoResult)};
}

ATransformer::Result Subscript::transform(Context& context, Node::Instance const& node) {
	auto const src = Expression().transform(context, node->leftSide);
	if (!src.source)
		context.error("Expected value here!", node->leftSide);
	if (!(src.type->flags.isArray || src.type->basic == Core::BasicType::AV2_BT_VECTOR))
		context.error("Value is not indexable!", node->leftSide);
	if (src.shouldBePushed())
		context.top()->impl->writeMainLine("push", *src.source);
	else if (src.isStackTop() && src.isCopied()) {
		context.top()->impl->writeMainLine("copy", *src.source, "-> top");
	}
	auto const index = Expression().transform(context, node->rightSide);
	if (index.mayBeEmpty) context.error("One or more code paths may not result in a value!", node->rightSide);
	if (index.isCompilable()) {
		if (!index.direct.isInteger() or index.direct.getSigned() < 0)
			context.error("Direct value must be an unsigned integer here!", node->rightSide);
		if (src.type->basic == Core::BasicType::AV2_BT_VECTOR && index.direct.getUnsigned() > 3)
			context.error("Index range must be between 0 and 3!", node->rightSide);
		context.top()->impl->writeMainLine("at [", index.direct.getUnsigned(), "]");
	} else if (
		index.type == context.basicType("uint8")
	||	index.type == context.basicType("uint16")
	||	index.type == context.basicType("uint32")
	||	index.type == context.basicType("uint64")
	) {
		if (index.shouldBePushed())
			context.top()->impl->writeMainLine("push", *index.source);
		else if (index.isStackTop() && index.isCopied()) {
			context.top()->impl->writeMainLine("copy", *index.source, "-> top");
		}
		if (src.type->basic == Core::BasicType::AV2_BT_VECTOR)
			context.top()->impl->writeMainLine("mod" + asFastOpQualifier(*index.type->basic, {.direct = 4}));
		context.top()->impl->writeMainLine("dyn at");
	} else context.error("Expected unsigned integer here!", node->rightSide);
	auto const t = src.type->base;
	return {{"move top"}, t->scope.asStrong(), t};
}

ATransformer::Result Array::transform(Context& context, Node::Instance const& node) {
	Namespace::TypeRef prev;
	usize const count = node->children.size();
	for (auto const& arg: node->children) {
		auto const expr = Expression().transform(context, arg);
		if (expr.mayBeEmpty) context.error("One or more code paths may not result in a value!", arg);
		if (!expr.source)
			context.error("Expected value here!", arg);
		if (expr.shouldBePushed())
			context.top()->impl->writeMainLine("push", *expr.source);
		else if (expr.isStackTop() && expr.isCopied()) {
			context.top()->impl->writeMainLine("copy", *expr.source, "-> top");
		}
		if (!prev)
			prev = expr.type;
		else if (!(prev = TypeDecl::stronger(prev, expr.type)))
			context.error("Type mismatch here!", arg);
	}
	auto const arr = context.arrayFor(prev);
	if (count)
		context.top()->impl->writeMainLine(count ? "create" : "new", arr->name);
	return {{"move top"}, arr->scope.asStrong(), arr};
}

ATransformer::Result Create::transform(Context& context, Node::Instance const& node) {
	auto t = TypeRequest().transform(context, node->leftSide).type;
	auto count = node->children.size();
	String opstr;
	if (t->flags.isArray) {
		opstr = "[" + t->name + ":" + Makai::toString(count) + "]";
	} else opstr = t->name;
	if (node->rightSide) {
		t = context.arrayFor(t);
		auto const sz = Expression().transform(context, node->rightSide);
		if (sz.mayBeEmpty) context.error("One or more code paths may not result in a value!", node->rightSide);
		if (!sz.source)
			context.error("Expected value here!", node->rightSide);
		if (sz.direct.isInteger()) {
			if (sz.direct.isSigned() && sz.direct.getSigned() < 0)
				context.error("Arrays cannot have negative sizes!", node->rightSide);
			count = sz.direct.getUnsigned();
		} else if (!sz.direct.isUndefined() and !sz.direct.isInteger())
			context.error("Expected integer here!", node->rightSide);
		else if (!TypeDecl::stronger(sz.type, context.basicType("uint64")))
			context.error("Expected integer here!", node->rightSide);
		else if (sz.shouldBePushed())
			context.top()->impl->writeMainLine("push", *sz.source);
		else if (sz.isStackTop() && sz.isCopied())
			context.top()->impl->writeMainLine("copy", *sz.source, "-> top");
		if (!sz.direct.isUndefined())
			opstr = "[" + t->name + ":" + Makai::toString(count) + "]";
		else opstr = "*" + t->name;
	}
	if (node->children.size()) {
		context.top()->impl->writeMainLine("begin");
		auto const tempStart = context.top()->varc;
		context.top()->impl->writeMainLine("decl", count);
		context.top()->varc += count;
		if (t->flags.isStructure) {
			usize index = 0;
			Map<usize, UTF8String> remap;
			Map<UTF8String, bool> filled;
			for (auto& [name, entry]: t->fields) {
				MAKAILIB_DEBUGLN_FULL("Field: ", name, " -> ", entry->id);
				remap[entry->id] = name;
			}
			for (auto const& arg: node->children) {
				if (arg->content == Node::Content::AV2_TANC_ASSIGNMENT) {
					auto const field = context.pathOf(arg->leftSide);
					MAKAILIB_DEBUGLN_FULL("Setting Field: ", field.join("/"));
					if (field.size() > 1) context.error("Invalid field access!", arg->leftSide);
					if (!t->fields.contains(field.front()))
						context.error("Field does not exist!", arg->leftSide);
					if (filled.contains(field.front()))
						context.error("Field has already been set!", arg->leftSide);
					auto const expr = Expression().transform(context, arg->rightSide);
					if (expr.mayBeEmpty) context.error("One or more code paths may not result in a value!", arg);
					if (!expr.source)
						context.error("Expected value here!", arg->rightSide);
					auto const fx = t->fields[field.front()];
					if (TypeDecl::stronger(expr.type, fx->type.asStrong()) != fx->type)
						context.error("Type mismatch in field!", arg->rightSide);
					context.top()->impl->writeMainLine("copy", expr.source.value(), "-> local[", fx->id + tempStart, "]");
					if (!expr.shouldBePushed())
						context.top()->impl->writeMainLine("pop");
					filled[field.front()] = true;
				} else {
					if (filled.size() >= remap.size())
						context.error("All fields have been already set!", arg);
					while (filled.contains(remap[index]))
						++index;
					if (index >= t->fields.size())
						context.error("All fields have been already set!", arg);
					auto const expr = Expression().transform(context, arg);
					if (expr.mayBeEmpty) context.error("One or more code paths may not result in a value!", arg);
					if (!expr.source)
						context.error("Expected value here!", arg);
					auto const fx = t->fields[remap[index]];
					if (TypeDecl::stronger(expr.type, fx->type.asStrong()) != fx->type)
						context.error("Type mismatch in field!", arg);
					context.top()->impl->writeMainLine("copy", expr.source.value(), "-> local[", fx->id + tempStart, "]");
					if (!expr.shouldBePushed())
						context.top()->impl->writeMainLine("pop");
					filled[remap[index]] = true;
					++index;
				}
			}
		} else if (t->flags.isArray) {
			for (auto const& [arg, i]: Range::expand(node->children)) {
				auto const expr = Expression().transform(context, arg);
				if (TypeDecl::stronger(expr.type, t->base) != t->base)
					context.error("Type mismatch in array element!", arg);
				context.top()->impl->writeMainLine("copy", expr.source.value(), "-> local[", tempStart + i, "]");
				if (!expr.shouldBePushed())
					context.top()->impl->writeMainLine("pop");
			}
		} else if (t->flags.isBasic) {
			if (t->basic != Core::BasicType::AV2_BT_VECTOR && node->children.size() > 1)
				context.error("Scalar types can only take one value!", node->leftSide);
			if (t->basic == Core::BasicType::AV2_BT_VECTOR) {
				context.impl()->main.popBack();
				context.impl()->main.popBack();
				if (node->children.size() > 4)
					context.error("Vectors can only take at most four values!");
				context.top()->impl->writeMainLine("new", opstr);
				for (auto const& [arg, i]: Range::expand(node->children)) {
					auto const expr = Expression().transform(context, arg);
					if (!TypeDecl::stronger(expr.type, context.basicType("float32")))
						context.error("Expected float here!", node->rightSide);
					else if (expr.shouldBePushed())
						context.top()->impl->writeMainLine("push", *expr.source);
					else if (expr.isStackTop() && expr.isCopied())
						context.top()->impl->writeMainLine("copy", *expr.source, "-> top");
					context.top()->impl->writeMainLine("set [", *expr.source, "]");
				}
				return {{"move top"}, t->scope.asStrong(), t};
			}
		}
		context.top()->impl->writeMainLine("blit ref", count, "[", tempStart, "] -> global");
		context.top()->impl->writeMainLine("create", opstr);
		context.top()->impl->writeMainLine("end");
	} else context.top()->impl->writeMainLine("new", opstr);
	return {{"move top"}, t->scope.asStrong(), t};
}

ATransformer::Result Drop::transform(Context& context, Node::Instance const& node) {
	auto const at = PathExpression().transform(context, node->leftSide);
	if (!at.source)
		context.error("Expression does not result in a value!", node->leftSide);
	if (!at.isCompilable())
		context.top()->impl->writeMainLine("drop", *at.source);
	return {};
}

ATransformer::Result InlineIfElse::transform(Context& context, Node::Instance const& node) {
	if (!node->rightSide)
		context.error("Missing [else] expression!");
	MAKAILIB_DEBUGLN_FULL("Handling inline if-else...");
	auto const iif = Branch().transform(context, node);
	if (!(iif.source and iif.type)) context.error("inline if-elses must result in a value!", node);
	return iif;
}

ATransformer::Result Branch::transform(Context& context, Node::Instance const& node) {
	auto const cond = Expression().transform(context, node->middle);
	auto const invert = (node->base.text == "unless" or node->base.text == "except");
	MAKAILIB_DEBUGLN_FULL("If-Condition: ", cond.type ? cond.type->name : "ERR", "(must be ", invert ? "TRUE" : "FALSE", ")");
	auto const varc = context.top()->varc;
	auto const ifScope = context.declare(UTF8StringList::from("<if>" + node->name()));
	bool mayBeEmpty = true;
	ifScope->varc += varc;
	ifScope->implementContents = true;
	if (cond.isCompilable()) {
		context.pop(1);
		ATransformer::Result expr;
		if (cond.direct.isTruthy() != invert)
			expr = Expression().transform(context, node->leftSide);
		else if (node->rightSide)
			expr = Expression().transform(context, node->rightSide);
		return expr;
	} else {
		if (!cond.source)
			context.error("Expression does not result in a value!", node->middle);
		auto const ifTrueLabel = "__if_" + node->name() + "_true_";
		auto const ifFalseLabel = "__if_" + node->name() + "_false_";
		auto const ifEndLabel = "__if_" + node->name() + "_end_";
		if (cond.shouldBePushed())
			context.top()->impl->writeMainLine("push", cond.source.value());
		else if (cond.isStackTop() && cond.isCopied()) {
			context.top()->impl->writeMainLine("copy", *cond.source, "-> top");
		}
		ATransformer::Result ifTrue, ifFalse;
		auto const writeTrueBranch = [&] (bool const skipEndLabel = false) {
			auto const branchScope = context.declare(UTF8StringList::from("<if-true>" + node->name()));
			branchScope->varc += ifScope->varc;
			branchScope->implementContents = true;
			ifTrue = Expression().transform(context, node->leftSide);
			mayBeEmpty = mayBeEmpty or ifTrue.mayBeEmpty;
			if (ifTrue.source && ifTrue.shouldBePushed())
				branchScope->impl->writeMainLine("push", ifTrue.source.value());
			else if (ifTrue.isStackTop() && ifTrue.isCopied()) {
				branchScope->impl->writeMainLine("copy", *ifTrue.source, "-> top");
			}
			context.pop(1);
			ifScope->impl->writeMainLine("@target", ifTrueLabel, ":");
			ifScope->impl->writeMainLine(branchScope->compose()->toString());
			if (!skipEndLabel) ifScope->impl->writeMainLine("jump", ifEndLabel);
		};
		auto const writeFalseBranch = [&] (bool const skipEndLabel = false) {
			if (!node->rightSide) return;
			auto const branchScope = context.declare(UTF8StringList::from("<if-false>" + node->name()));
			branchScope->varc += ifScope->varc;
			branchScope->implementContents = true;
			ifFalse = Expression().transform(context, node->rightSide);
			mayBeEmpty = mayBeEmpty or ifFalse.mayBeEmpty;
			if (ifFalse.source && ifFalse.shouldBePushed())
				branchScope->impl->writeMainLine("push", ifFalse.source.value());
			else if (ifFalse.isStackTop() && ifFalse.isCopied()) {
				branchScope->impl->writeMainLine("copy", *ifFalse.source, "-> top");
			}
			context.pop(1);
			ifScope->impl->writeMainLine("@target", ifFalseLabel, ":");
			ifScope->impl->writeMainLine(branchScope->compose()->toString());
			if (!skipEndLabel) ifScope->impl->writeMainLine("jump", ifEndLabel);
		};
		if (cond.likelihood >= 0) {
			String const condType = (invert) ? "true" : "false";
			ifScope->impl->writeMainLine("jump if", condType, node->rightSide ? ifFalseLabel : ifEndLabel);
			writeTrueBranch();
			writeFalseBranch(true);
		} else {
			String const condType = (invert) ? "false" : "true";
			ifScope->impl->writeMainLine("jump if", condType, ifTrueLabel);
			writeFalseBranch();
			writeTrueBranch(true);
		}
		if (node->rightSide && ifTrue.type != ifFalse.type)
			context.error("Both paths return different types!", node);
		if (node->rightSide) {
			MAKAILIB_DEBUGLN_FULL("If-True-side: ", ifTrue.type ? ifTrue.type->name : "NO TYPE");
			MAKAILIB_DEBUGLN_FULL("If-False-side: ", ifFalse.type ? ifFalse.type->name : "NO TYPE");
		} else {
			MAKAILIB_DEBUGLN_FULL("If-True-side: ", ifTrue.type ? ifTrue.type->name : "NO TYPE");
			MAKAILIB_DEBUGLN_FULL("If-False-side: ", "NONE");
		}
		ifScope->impl->writePostLine("@target", ifEndLabel, ":");
		context.pop(1);
		context.impl()->writeMainLine(ifScope->compose()->toString());
		return {.source = {"move top"}, .type = ifTrue.type, .likelihood = ifTrue.likelihood + ifFalse.likelihood, .mayBeEmpty = mayBeEmpty or !node->rightSide};
	}
}

ATransformer::Result Loop::transform(Context& context, Node::Instance const& node) {
	auto const scope = UTF8StringList::from("__" + node->base.text + "_loop_" + node->name());
	ATransformer::Result exprOut;
	auto const varc = context.top()->varc;
	auto const loopScope = context.declare(scope);
	loopScope->varc += varc;
	loopScope->implementContents = true;
	if (node->base.text == "do")			exprOut = DoLoop().transform(context, node);
	else if (node->base.text == "while")	exprOut = WhileLoop().transform(context, node);
	else if (node->base.text == "repeat")	exprOut = RepeatLoop().transform(context, node);
	else if (node->base.text == "for")		exprOut = ForLoop().transform(context, node);
	context.pop(scope.size());
	auto const lpi = loopScope->compose();
	MAKAILIB_DEBUGLN_FULL(loopScope->serialize().toFLOWString("  "));
	MAKAILIB_DEBUGLN_FULL(lpi->serialize().toFLOWString("  "));
	context.top()->impl->writeMainLine(lpi->toString());
	return exprOut;
}

ATransformer::Result ForLoop::transform(Context& context, Node::Instance const& node) {
	auto const loopStart = context.top()->name + "_start" + node->name();
	auto const loopEnd = context.top()->name + "_end" + node->name();
	auto const loopScope = context.top();
	loopScope->impl->writePreLine("decl 2");
	Namespace::Instance varScope;
	if (node->leftSide->content == Node::Content::AV2_TANC_DECLARATION)
		varScope = VariableDecl().transform(context, node->leftSide).scope;
	else {
		auto const vname = context.pathOf(node->leftSide);
		auto const scope = context.declare(vname);
		auto& var = *(scope->variable = scope->variable.create());
		var.name = scope->name;
		var.parentScope = loopScope.asWeak();
		var.id = loopScope->varc++;
		varScope = scope;
		context.pop(vname.size());
	}
	auto& elemVar = *varScope->variable;
	auto const toIterate = Expression().transform(context, node->middle);
	String const traversalVar = Makai::toString("local[", loopScope->varc++, "]");
	if (!toIterate.source)
		context.error("Expected value here!", node->middle);
	if (toIterate.mayBeEmpty)
		context.error("One or more code path does not result in a value!", node->middle);
	if (!toIterate.type->flags.isArray) {
		// TODO: iteratable objects (later)
		context.error("Only array for loops are currently supported!", node->middle);
	} else {
		if (!elemVar.type) elemVar.type = toIterate.type->base;
		else if (TypeDecl::stronger(elemVar.type.asStrong(), toIterate.type->base) != elemVar.type)
			context.error("Element type is stronger than variable type!", node->middle);
		context.top()->impl->writeMainLine("copy", toIterate.source.value(), "->", traversalVar);
		if (toIterate.isStackTop())
			context.top()->impl->writeMainLine("pop");
		context.top()->impl->writeMainLine("op inv");
		loopScope->impl->writeMainLine("@target", loopStart, ":");
		loopScope->impl->writeMainLine("push ref", traversalVar);
		loopScope->impl->writeMainLine("count");
		loopScope->impl->writeMainLine("jump if false", loopEnd);
		loopScope->impl->writeMainLine("push ref", traversalVar);
		loopScope->impl->writeMainLine("op apop");
		loopScope->impl->writeMainLine("copy move top ->", elemVar.getSource());
		loopScope->impl->writeMainLine("pop");
		elemVar.fill();
		auto const loopExpr = Expression().transform(context, node->rightSide);
		loopScope->impl->writePostLine("jump", loopStart);
		loopScope->impl->writeMainLine("@target", loopEnd, ":");
	}
	return {.scope = loopScope};
}

ATransformer::Result WhileLoop::transform(Context& context, Node::Instance const& node) {
	auto const loopStart = context.top()->name + "_start" + node->name();
	auto const loopEnd = context.top()->name + "_end" + node->name();
	auto const loopScope = context.top();
	loopScope->impl->writePreLine("@target", loopStart, ":");
	auto const condExpr = Expression().transform(context, node->leftSide);
	if (!condExpr.isCompilable()) {
		if (condExpr.shouldBePushed())
			loopScope->impl->writeMainLine("push", condExpr.source.value());
		else if (condExpr.isStackTop() && condExpr.isCopied()) {
			loopScope->impl->writeMainLine("copy", *condExpr.source, "-> top");
		}
		loopScope->impl->writeMainLine("jump if false", loopEnd);
	} else if (!condExpr.direct) return {};
	auto const loopExpr = Expression().transform(context, node->rightSide);
	loopScope->impl->writePostLine("jump", loopStart);
	loopScope->impl->writePostLine("@target", loopEnd, ":");
	return {.scope = loopScope};
}

ATransformer::Result RepeatLoop::transform(Context& context, Node::Instance const& node) {
	MAKAILIB_DEBUGLN_FULL("Iteration Count: ", node->leftSide->base.text);
	if (node->middle) {
		MAKAILIB_DEBUGLN_FULL("Iterand: ", node->middle->leftSide->base.text);
		MAKAILIB_DEBUGLN_FULL("Iterand Type: ", node->middle->rightSide->base.text);
	}
	auto const loopStart = context.top()->name + "_start" + node->name();
	auto const loopEnd = context.top()->name + "_end" + node->name();
	auto const loopScope = context.top();
	auto const vsn = UTF8StringList::from("##ITERATE::" + node->name());
	auto const varScope = context.declare(vsn);
	varScope->varc += loopScope->varc;
	auto& var = *(varScope->variable = varScope->variable.create());
	var.fill();
	var.id = loopScope->varc++;
	var.type = context.basicType("uint64").asWeak();
	var.name = "##ITERATE::" + node->name();
	context.pop(vsn.size());
	auto const it = Expression().transform(context, node->leftSide);
	String opq;
	if (it.isCompilable() or it.source) {
		if (!it.type)
			context.error("Expression must be an integer!", node->leftSide);
		if (auto const intType = TypeDecl::stronger(it.type, context.basicType("uint64"))) {
			if (intType->basic > Core::BasicType::AV2_BT_UINT64)
				context.error("Expression must be an integer!", node->leftSide);
			if (it.shouldBePushed())
				loopScope->impl->writePreLine("push", it.source.value());
			else if (it.isStackTop() && it.isCopied()) {
				loopScope->impl->writePreLine("copy", *it.source, "-> top");
			}
			opq = asFastOpQualifier(*intType->basic);
		} else context.error("Expression must be an integer!", node->leftSide);
	} else context.error("Expression must be an integer!", node->leftSide);
	loopScope->impl->writePreLine("push val", it.source.value());
	loopScope->impl->writePreLine("jump if false", loopEnd);
	loopScope->impl->writePreLine("copy", it.source.value(), "->", var.getSource());
	if (!it.shouldBePushed())
		context.top()->impl->writeMainLine("pop");
	if (node->middle) {
		auto const refVar = Expression().transform(context, node->middle);
		if (!(refVar.scope and refVar.scope->variable))
			context.error("Expected variable declaration here!", node->middle);
		refVar.scope->variable->fill();
		loopScope->impl->writePreLine("copy ref", var.getSource(), "->", refVar.scope->variable->getSource());
		var.type = refVar.scope->variable->type;
	}
	loopScope->impl->writePreLine("@target", loopStart, ":");
	auto const loopExpr = Expression().transform(context, node->rightSide);
	loopScope->impl->writePostLine("push ref", var.getSource());
	loopScope->impl->writePostLine("op dec", opq);
	loopScope->impl->writePostLine("jump if true", loopStart);
	loopScope->impl->writePostLine("@target", loopEnd, ":");
	return {.scope = loopScope};
}

ATransformer::Result DoLoop::transform(Context& context, Node::Instance const& node) {
	auto const loopStart = context.top()->name + "_start" + node->name();
	auto const loopEnd = context.top()->name + "_end" + node->name();
	auto const loopScope = context.top();
	loopScope->impl->writePreLine("@target", loopStart, ":");
	auto const loopExpr = Expression().transform(context, node->rightSide);
	if (node->leftSide) {
		auto const condExpr = Expression().transform(context, node->leftSide);
		if (!condExpr.isCompilable()) {
			if (condExpr.shouldBePushed())
				loopScope->impl->writePostLine("push", condExpr.source.value());
			else if (condExpr.isStackTop() && condExpr.isCopied()) {
				loopScope->impl->writePostLine("copy", *condExpr.source, "-> top");
			}
			loopScope->impl->writePostLine("jump if true", loopStart);
		} else if (!condExpr.direct)
			return loopExpr;
	} else loopScope->impl->writePostLine("jump", loopStart);
	return {.scope = loopScope};
}

ATransformer::Result Definition::transform(Context& context, Node::Instance const& node) {
	if (node->base.text == "::")			return FunctionDecl().transform(context, node);
	if (node->base.text == ":")				return VariableDecl().transform(context, node);
	if (node->base.type == LTS_TT_DECLARE)	return VariableDecl().transform(context, node);
	if (node->base.text == "prop")			return PropertyDecl().transform(context, node);
	if (node->base.text == "struct")		return StructureDecl().transform(context, node);
	if (node->base.text == "module")		return NamespaceDecl().transform(context, node);
	if (node->base.text == "?")				return NullableTypeDecl().transform(context, node);
	context.error("Unimplemented support for given declaration!", node);
}

ATransformer::Result InlineAssembly::transform(Context& context, Node::Instance const& node) {
	auto const scope = context.top()->impl;
	Makai::UTF8String interject = "";
	for (auto& tok: node->interject)
		interject += tok.text + " ";
	scope->writeMainLine(interject);
	return {};
}

ATransformer::Result TheEntireProgram::transform(Context& context, Node::Instance const& node) {
	ATransformer::Result result;
	for (auto const& child: node->children)
		result = Expression().transform(context, child);
	return result;
}

ATransformer::Result ArrayTypeDecl::transform(Context& context, Node::Instance const& node) {
	if (node->children.empty())
		context.error("Expected array type!", node);
	if (node->children.size() > 1)
		context.error("Arrays can only contain one type!", node);
	auto const t = context.arrayFor(TypeRequest().transform(context, node->children.front()).type);
	context.registerType(t->scope.asStrong());
	return {.type = t};
}

ATransformer::Result NullableTypeDecl::transform(Context& context, Node::Instance const& node) {
	auto const t = context.nullableFor(TypeRequest().transform(context, node).type);
	context.registerType(t->scope.asStrong());
	return {.type = t};
}

ATransformer::Result FunctionTypeDecl::transform(Context& context, Node::Instance const& node) {
	auto const scope = context.declare(UTF8StringList::from("<proto>" + node->name()));
	auto& type = *(scope->type = scope->type.create());
	type.flags.isFunction = true;
	type.base = TypeRequest().transform(context, node->leftSide).type;
	for (auto& arg: node->children)
		type.args.pushBack(TypeRequest().transform(context, arg).type);
	context.pop(1);
	context.registerType(scope);
	return {.type = scope->type};
}

ATransformer::Result TypeExtension::transform(Context& context, Node::Instance const& node) {
	auto const type = TypeRequest().transform(context, node->leftSide);
	AsNonConst<decltype(type)> trait;
	context.scopeStack.pushBack(type.type->scope.asStrong());
	if (node->middle) {
		// TODO: This
	}
	for (auto& extension: node->children) {
		auto const ext = Expression().transform(context, extension);
		if (!ext.scope)
			context.error("Invalid expression!", extension);
		auto const ns = ext.scope;
		if (ns->function) {
			for (auto& ov : ns->function->current)
			if (
				ov->variant == Function::Overload::Variant::Object::AV2_TCB_FO_VO_NONE
			or	ov->variant == Function::Overload::Variant::Object::AV2_TCB_FO_VO_CLASS
			) {
				if (ov->arguments.empty() or ov->arguments.front()->type != type.scope->type.asWeak())
					context.error(
						"Missing valid [this] argument for member function!\n"
						"Did you make sure the type is correct, or to set function as [@Static]?"
						, extension
					);
				ov->variant = Function::Overload::Variant::Object::AV2_TCB_FO_VO_CLASS;
			}
		} else if (ns->property) {

		} else if (ns->isPureNamespace())
			context.error("This type of declaration is disallowed in this context!", extension);
		else context.error("Invalid/unsupported declaration!");
	}
	context.scopeStack.popBack();
	return {};
}

ATransformer::Result Await::transform(Context& context, Node::Instance const& node) {
	if (node->base.text != "await") return AwaitBlock().transform(context, node);
	else return AwaitOne().transform(context, node);
}

ATransformer::Result AwaitOne::transform(Context& context, Node::Instance const& node) {
	auto const scope = UTF8StringList::from("__await_" + node->name());
	auto const awaitScope = context.declare(scope);
	auto const awaitStart =  "__await_start_" + node->name();
	auto const awaitEnd = "__await_end_" + node->name();
	awaitScope->impl->writePreLine("@target", awaitStart, ":");
	auto expr = Expression().transform(context, node->leftSide);
	auto const awaitType = TypeDecl::stronger(expr.type, context.basicType("uint64"));
	if (expr.isCompilable())
		context.error("Cannot await on direct expressions!");
	if (expr.shouldBePushed())
		awaitScope->impl->writeMainLine("push", expr.source.value());
	else if (expr.isStackTop() && expr.isCopied())
		awaitScope->impl->writeMainLine("copy", *expr.source, "-> top");
	if (!expr.source)
		context.error("Await expressions can only be used in checkable values!");
	String check;
	if (expr.type->flags.isNullable)
		check = "exists";
	else if (expr.type->basic && expr.type->basic == Core::BasicType::AV2_BT_BOOL)
		check = "true";
	else context.error("Await expressions can only be used in checkable values!");
	awaitScope->impl->writeMainLine("jump if", check, awaitEnd);
	awaitScope->impl->writePostLine("yield");
	awaitScope->impl->writePostLine("jump", awaitStart);
	awaitScope->impl->writePostLine("@target", awaitEnd, ":");
	context.pop(scope.size());
	context.top()->impl->writeMainLine(awaitScope->compose()->toString());
	if (expr.type->flags.isNullable)
		expr.type = expr.type->base;
	return expr;
}


ATransformer::Result AwaitBlock::transform(Context& context, Node::Instance const& node) {
	auto const scope = UTF8StringList::from("__await_" + node->name());
	auto const awaitScope = context.declare(scope);
	auto const awaitStart =  "__await_start_" + node->name();
	auto const awaitEnd = "__await_end_" + node->name();
	auto const awaitNext = "__await_next_" + node->name();
	StringList awaitExprs;
	auto const awaitVarsStart = awaitScope->varc;
	awaitScope->varc += node->children.size();
	awaitScope->impl->writePreLine("decl", node->children.size());
	bool const awaitAny = node->base.text == "yield";
	for (auto const& [chk, index]: Range::expand(node->children)) {
		auto const expr = Expression().transform(context, node->leftSide);
		if (!expr.source)
			context.error("Await block expressions can only be used in checkable values!");
		if (expr.isCompilable())
			context.error("Cannot await on direct expressions!");
		awaitScope->impl->writeMainLine("copy", *expr.source, "-> local[", awaitVarsStart + index, "]");
		if (expr.type->flags.isNullable)
			awaitExprs.pushBack("exists");
		else context.error("Await block expressions can only be used in nullable values!");
	}
	awaitScope->impl->writePostLine("@target", awaitStart, ":");
	for (auto const& [expr, index]: Range::expand(awaitExprs)) {
		awaitScope->impl->writePostLine("push local[", awaitVarsStart + index, "]");
		if (awaitAny)
			awaitScope->impl->writePostLine("jump if", expr, awaitEnd);
		else awaitScope->impl->writePostLine("jump if not", expr, awaitNext);
	}
	awaitScope->impl->writePostLine("@target", awaitNext, ":");
	awaitScope->impl->writePostLine("yield");
	awaitScope->impl->writePostLine("jump", awaitStart);
	awaitScope->impl->writePostLine("@target", awaitEnd, ":");
	// TODO: Proper return type
	return {};
}

ATransformer::Result NullDecay::transform(Context& context, Node::Instance const& node) {
	auto const exit = "__null_decay_" + node->name() + "_end";
	ATransformer::Result result;
	auto const lhs = Expression().transform(context, node);
	if (lhs.mayBeEmpty) context.error("One or more code paths may not result in a value!", node->leftSide);
	if (!(lhs.type && lhs.type->flags.isNullable))
		context.error("Expression must result in a nullable value!", node->leftSide);
	if (lhs.shouldBePushed())
		context.top()->impl->writeMainLine("push", lhs.source.value());
	else if (lhs.isStackTop() && lhs.isCopied())
		context.top()->impl->writeMainLine("copy", *lhs.source, "-> top");
	context.top()->impl->writeMainLine("push val top");
	context.top()->impl->writeMainLine("jump if not empty", exit);
	auto const rhs = Expression().transform(context, node);
	if (rhs.mayBeEmpty) context.error("One or more code paths may not result in a value!", node->rightSide);
	if (!(lhs.type->base != rhs.type))
		context.error("Type mismatch in null decay!", node->rightSide);
	if (rhs.shouldBePushed())
		context.top()->impl->writeMainLine("push", rhs.source.value());
	else if (rhs.isStackTop() && rhs.isCopied())
		context.top()->impl->writeMainLine("copy", *rhs.source, "-> top");
	context.top()->impl->writeMainLine("@target", exit);
	return {{"move top"}, null, lhs.type->base};
}

ATransformer::Result Evaluation::transform(Context& context, Node::Instance const& node) {
	ATransformer::Result result;
	auto const lhs = Expression().transform(context, node);
	if (lhs.isCompilable() && lhs.direct.isString())
		return Expression().transform(context, context.evaluate(lhs.direct.getString()));
	context.error("Invalid evaluation!", node->leftSide);
}

ATransformer::Result Switch::transform(Context& context, Node::Instance const& node) {
	ATransformer::Result result;
	auto const varc = context.top()->varc;
	auto const switchScope = context.declare(UTF8StringList::from("<switch>" + node->name()));
	switchScope->varc += varc;
	switchScope->implementContents = true;
	auto const switchExpr = Expression().transform(context, node->leftSide);
	if (switchExpr.mayBeEmpty)
		context.error("One or more paths paths may not return a value!", node->leftSide);
	usize pickExprLoc = 0;
	if (!switchExpr.isCompilable()) {
		if (switchExpr.shouldBePushed())
			switchScope->impl->writeMainLine("push", switchExpr.source.value());
		else if (switchExpr.isStackTop() && switchExpr.isCopied())
			switchScope->impl->writeMainLine("copy", *switchExpr.source, "-> top");
		pickExprLoc = switchScope->impl->main.size();
		switchScope->impl->writeMainLine("");
	} else {
		if (!switchExpr.direct.isInteger())
			context.error("Expected enumerable value here!", node->leftSide);
		for (auto& caseExpr: node->children) {
			auto const match = Expression().transform(context, caseExpr->leftSide);
			if (!match.isCompilable())
				context.error("Expected direct value here!", caseExpr->leftSide);
			if (!match.direct.isInteger())
				context.error("Expected enumerable value here!", caseExpr->leftSide);
			if (match.direct.getSigned() != switchExpr.direct.getSigned()) continue;
			context.pop(1);
			return Expression().transform(context, caseExpr->rightSide);
		}
		context.error("No matching case found!", node->leftSide);
	}
	if (!switchExpr.source)
		context.error("Expected value here!", node->leftSide);
	if (!(switchExpr.type->flags.isBasic or switchExpr.type->flags.isEnum))
		context.error("Expected enumeratable value here!", node->leftSide);
	auto switchType = switchExpr.type;
	if (!Core::isInteger(switchType->flags.isEnum ? *switchType->base->basic : *switchType->basic))
		context.error("Expected enumerable value here!", node->leftSide);
	auto const caseMarker = "__switch_case" + node->name();
	auto const switchEnd = "__switch_end" + node->name();
	auto const defaultCase = "__switch_default" + node->name();
	bool hasDefault = false;
	Makai::Map<ssize, String> matches;
	decltype(switchType) prevCaseType;
	bool isFirstCase = true;
	bool mayBeEmpty = false;
	if (node->children.size() < 2)
		context.error("Switch statements must have at least two cases!", node);
	MAKAILIB_DEBUGLN_FULL("Total cases: ", node->children.size());
	for (auto& caseExpr: node->children) {
		bool isDefaultCase = false;
		auto const caseScope = context.declare(UTF8StringList::from("<case>" + caseExpr->name()));
		caseScope->varc += switchScope->varc;
		caseScope->implementContents = true;
		if (caseExpr->leftSide->base.text != "else") {
			auto const match = Expression().transform(context, caseExpr->leftSide);
			if (!match.isCompilable())
				context.error("Expected direct value here!", caseExpr->leftSide);
			if (match.type != switchType)
				context.error("Type mismatch in switch case!", caseExpr->leftSide);
			ssize matchIndex = match.direct.getSigned();
			if (matches.contains(matchIndex))
				context.error("A case for this value was already declared!", caseExpr->leftSide);
			matches[matchIndex] = caseMarker + caseExpr->name();
		} else if (!hasDefault) {
			hasDefault = true;
			isDefaultCase = true;
		} else context.error("Redeclaration of default case!", caseExpr->leftSide);
		auto const then = Expression().transform(context, caseExpr->rightSide);
		mayBeEmpty = mayBeEmpty or then.mayBeEmpty;
		if (!isFirstCase && prevCaseType != then.type)
			context.error("Case result mismatch!", caseExpr->rightSide);
		else if (isFirstCase)
			prevCaseType = then.type;
		result = then;
		if (then.shouldBePushed())
			caseScope->impl->writeMainLine("push", then.source.value());
		else if (then.isStackTop() && then.isCopied())
			caseScope->impl->writeMainLine("copy", *then.source, "-> top");
		context.pop(1);
		switchScope->impl->writeMainLine("@target", isDefaultCase ? defaultCase : (caseMarker + caseExpr->name()), ":");
		switchScope->impl->writeMainLine(caseScope->compose()->toString());
		switchScope->impl->writeMainLine("jump", switchEnd);
		isFirstCase = false;
	}
	auto const matchIndices = matches.keys();
	auto const lowestIndex = matchIndices.front() < matchIndices.back() ? matchIndices.front() : matchIndices.back();
	auto const highestIndex = matchIndices.front() < matchIndices.back() ? matchIndices.back() : matchIndices.front();
	UTF8String choices = "[";
	auto const elseGoHere = (hasDefault ? defaultCase : switchEnd);
	for (ssize i = lowestIndex; i <= highestIndex; ++i)
		choices += " " + (matches.contains(i) ? matches[i] : elseGoHere);
	choices += " " + elseGoHere + " ]";
	switchScope->impl->main[pickExprLoc] = "pick " + choices;
	switchScope->impl->writePostLine("@target", switchEnd, ":");
	context.pop(1);
	context.impl()->writeMainLine(switchScope->compose()->toString());
	if (!result.source) return {};
	return {.source = {"move top"}, .type = result.type, .likelihood = result.likelihood + result.likelihood,.mayBeEmpty = mayBeEmpty or !hasDefault};
}

ATransformer::Result Match::transform(Context& context, Node::Instance const& node) {
	ATransformer::Result result;
	auto const varc = context.top()->varc;
	auto const matchScope = context.declare(UTF8StringList::from("<match>" + node->name()));
	matchScope->varc += varc;
	matchScope->implementContents = true;
	auto const caseMarker = "__match_case" + node->name();
	auto const caseSkipMarker = "__match_case_skip" + node->name();
	auto const matchEnd = "__match_end" + node->name();
	auto const defaultCase = "__match_default" + node->name();
	Node::Instance defaultCaseExpr;
	Namespace::TypeRef prevCaseType;
	bool isFirstCase = true;
	bool mayBeEmpty = false;
	MAKAILIB_DEBUGLN_FULL("Total cases: ", node->children.size());
	for (auto& caseExpr: node->children) {
		auto const caseScope = context.declare(UTF8StringList::from("<case>" + caseExpr->name()));
		caseScope->varc += matchScope->varc;
		caseScope->implementContents = true;
		if (caseExpr->leftSide->base.text != "else") {
			auto const match = Expression().transform(context, caseExpr->leftSide);
			if (match.shouldBePushed())
				caseScope->impl->writeMainLine("push", match.source.value());
			else if (match.isStackTop() && match.isCopied())
				caseScope->impl->writeMainLine("copy", *match.source, "-> top");
			caseScope->impl->writeMainLine("jump if false", caseSkipMarker + caseExpr->name());
		} else if (!defaultCaseExpr) {
			context.pop(1);
			defaultCaseExpr = caseExpr;
			continue;
		} else context.error("Redeclaration of default case!", caseExpr->leftSide);
		auto const then = Expression().transform(context, caseExpr->rightSide);
		mayBeEmpty = mayBeEmpty or then.mayBeEmpty;
		if (!isFirstCase && prevCaseType != then.type)
			context.error("Case result mismatch!", caseExpr->rightSide);
		else if (isFirstCase)
			prevCaseType = then.type;
		result = then;
		if (then.mayBeEmpty)
			context.error("One or more paths may not return a value!");
		if (then.shouldBePushed())
			caseScope->impl->writeMainLine("push", then.source.value());
		else if (then.isStackTop() && then.isCopied())
			caseScope->impl->writeMainLine("copy", *then.source, "-> top");
		context.pop(1);
		matchScope->impl->writeMainLine("@target", (caseMarker + caseExpr->name()), ":");
		matchScope->impl->writeMainLine(caseScope->compose()->toString());
		matchScope->impl->writeMainLine("jump", matchEnd);
		matchScope->impl->writeMainLine("@target", (caseSkipMarker + caseExpr->name()), ":");
		matchScope->impl->writeMainLine("end");
		isFirstCase = false;
	}
	if (defaultCaseExpr) {
		auto const caseScope = context.declare(UTF8StringList::from("<default>" + defaultCaseExpr->name()));
		caseScope->varc += matchScope->varc;
		caseScope->implementContents = true;
		auto const then = Expression().transform(context, defaultCaseExpr->rightSide);
		if (!isFirstCase && prevCaseType != then.type)
			context.error("Case result mismatch!", defaultCaseExpr->rightSide);
		else if (isFirstCase)
			prevCaseType = then.type;
		result = then;
		if (then.mayBeEmpty)
			context.error("One or more paths may not return a value!");
		if (then.shouldBePushed())
			caseScope->impl->writeMainLine("push", then.source.value());
		else if (then.isStackTop() && then.isCopied())
			caseScope->impl->writeMainLine("copy", *then.source, "-> top");
		context.pop(1);
		matchScope->impl->writeMainLine(caseScope->compose()->toString());
		result = then;
	}
	matchScope->impl->writePostLine("@target", matchEnd, ":");
	context.pop(1);
	context.impl()->writeMainLine(matchScope->compose()->toString());
	if (!result.source) return {};
	return {.source = {"move top"}, .type = result.type, .likelihood = result.likelihood + result.likelihood, .mayBeEmpty = mayBeEmpty or defaultCaseExpr};
}

ATransformer::Result SwitchMatch::transform(Context& context, Node::Instance const& node) {
	if (node->leftSide)
		return Switch().transform(context, node);
	else return Match().transform(context, node);
}

Namespace::TypeRef ATransformer::Context::basicType(UTF8String const& name) {
	auto const scope = resolve(UTF8StringList::from(name));
	if (!(scope && scope->type))
		error("Basic type ["+name+"] does not exist!\nDid you forget to [using import core.types]?");
	return scope->type;
}

Namespace::TypeRef ATransformer::Context::basicTypeOf(Makai::Data::Value const& value) {
	if (value.isBoolean()) {
		return basicType("bool");
	} else if (value.isString()) {
		return basicType("string");
	} else if (value.isUnsigned()) {
		return basicType("uint64");
	} else if (value.isSigned()) {
		return basicType("int64");
	} else if (value.isReal()) {
		return basicType("float64");
	} else return nullptr;
}

Namespace::TypeRef ATransformer::Context::arrayFor(Namespace::TypeRef const& type) {
	if (!type) return nullptr;
	if (!arrays.contains(type.asWeak())) {
		auto const arr = type.create();
		arr->flags.isArray = true;
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

Namespace::TypeRef ATransformer::Context::nullableFor(Namespace::TypeRef const& type) {
	if (!type) return nullptr;
	if (!nullables.contains(type.asWeak())) {
		auto const arr = type.create();
		arr->flags.isNullable = true;
		arr->base = type;
		arr->name = type->name + "OrNull";
		auto const nsp = Namespace::Instance::create(arr->name);
		registerType(nsp);
		auto& ns = *nsp;
		ns.type = arr;
		nullables[type.asWeak()] = arr;
		return arr;
	} else return nullables[type.asWeak()];
}

static Makai::String idName(usize const id) {
	return Makai::Format::pad(Makai::toString(id), '0', 16, CTL::Format::Justify::CFJ_LEFT);
}

void ATransformer::Context::registerType(Namespace::Instance const& ns) {
	static usize id = 0;
	if (!ns) return;
	root->subspaces["0__@Tx1_USER_TYPES"]->subspaces[Makai::toString("#", idName(++id), "::") + ns->name] = ns;
}

void ATransformer::Context::registerFunction(Namespace::Instance const& ns) {
	static usize id = 0;
	if (!ns) return;
	root->subspaces["0__@Tx2_FUNCTIONS"]->subspaces[Makai::toString("#", idName(++id), "::") + ns->name] = ns;
}

void ATransformer::Context::registerImport(Namespace::Instance const& ns) {
	static usize id = 0;
	if (!ns) return;
	root->subspaces["0__@Tx0_IMPORTS"]->subspaces[Makai::toString("#", idName(++id), "::") + ns->name] = ns;
}

ATransformer::Context::Context(): Intermediate() {
	using enum Core::BasicType;
	root->subspaces["0__@Tx0_IMPORTS"]		= Namespace::Instance::create("0__@Tx0_IMPORTS");
	root->subspaces["0__@Tx1_USER_TYPES"]	= Namespace::Instance::create("0__@Tx1_USER_TYPES");
	root->subspaces["0__@Tx2_FUNCTIONS"]	= Namespace::Instance::create("0__@Tx2_FUNCTIONS");
	root->subspaces["0__@Tx3_TRAITS"]		= Namespace::Instance::create("0__@Tx3_TRAITS");
}

Node::Instance ATransformer::Context::evaluate(Makai::UTF8String const& eval) {
	BaseContext ctx;
	ctx.append(
		Lexer::CStyle::tokenize(eval)
			.value()
			.orElse({})
			.toList<BaseContext::Axiom>()
	);
	auto const parse = Parser(ctx).parse();
	return parse;
}

Makai::Function<File(Makai::UTF8String const&)> Import::importer = [] (auto const&) -> File {
	throw Error::InvalidAction("Missing importer!");
};
