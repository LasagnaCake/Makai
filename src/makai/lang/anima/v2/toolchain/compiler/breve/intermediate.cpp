#include "intermediate.hpp"
#include "transformer.hpp"
#include "../../../../../../tool/archive/archive.hpp"

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;

IWritable::~IWritable() {}

#define ATTRIBUTE_TRANSFORMER()  [=] (Intermediate& inter, Namespace::Instance const& ns, Makai::Data::Value const& v, Attribute& base)

Namespace::Namespace(UTF8String const& name): Labeled(name) {}
Function::Function(UTF8String const& name): Labeled(name) {}
Trait::Trait(UTF8String const& name): Labeled(name) {}
TypeDecl::TypeDecl(UTF8String const& name): Labeled(name) {}
Variable::Variable(UTF8String const& name): Labeled(name) {}
Property::Property(UTF8String const& name): Labeled(name) {}
Attribute::Attribute(UTF8String const& name): Labeled(name) {}

Namespace::~Namespace() {}
Function::~Function() {}
Trait::~Trait() {}
TypeDecl::~TypeDecl() {}
Property::~Property() {}
Variable::~Variable() {}
Attribute::~Attribute() {}

Intermediate::~Intermediate() {}

Namespace::Instance Namespace::resolve(UTF8StringList const& path) const {
	if (path.empty()) return nullptr;
	MAKAILIB_DEBUG_FULL("Subspaces in '", name,"' : [ ");
	for (auto const& [name, subns]: subspaces)
		MAKAILIB_DEBUG_FULL( "{", name , ":", subns ? subns->name : "???NULL???", "} ");
	MAKAILIB_DEBUGLN_FULL("]");
	MAKAILIB_DEBUGLN_FULL("Looking for ", path.front().toString());
	MAKAILIB_DEBUGLN_FULL("Exists? ", subspaces.contains(path.front()));
	if (!subspaces.contains(path.front())) return nullptr;
	MAKAILIB_DEBUGLN_FULL("You Sure? ", subspaces[path.front()].exists());
	if (path.size() == 1)
		return subspaces[path.front()];
	else if (subspaces[path.front()]) return subspaces[path.front()]->resolve(path.sliced(1));
	return nullptr;
}

Namespace::Instance Intermediate::resolve(UTF8StringList const& path) const {
	if (path.empty()) return nullptr;
	MAKAILIB_DEBUGLN_FULL("Looking for '/", path.join("/"), "'");
	for (auto& scope: Makai::Range::reverse(scopeStack)) {
		MAKAILIB_DEBUGLN_FULL("Scope: ", scope->name);
		MAKAILIB_DEBUG_FULL("Subspaces: [ ");
		for (auto const& [name, subns]: scope->subspaces)
			MAKAILIB_DEBUG_FULL( "{", name , ":", subns ? subns->name : "###__NULL__###", "} ");
		MAKAILIB_DEBUGLN_FULL("]");
		if (scope->name == path.front()) {
			if (path.size() == 1)
				return scope;
			else if (auto const ns = scope->resolve(path.sliced(1)))
				return ns;
		} else if (auto const ns = scope->resolve(path)) return ns;
		MAKAILIB_DEBUGLN_FULL("Nope!");
	}
	MAKAILIB_DEBUGLN_FULL("Global scope");
	MAKAILIB_DEBUG_FULL("Subspaces: [ ");
	for (auto const& [name, subns]: root->subspaces)
		MAKAILIB_DEBUG_FULL( "{", name , ":", subns->name, "} ");
	MAKAILIB_DEBUGLN_FULL("]");
	if (auto const ns = root->resolve(path))
		return ns;
	MAKAILIB_DEBUGLN_FULL("Nope!");
	return nullptr;
}

Namespace::Instance Intermediate::push(UTF8StringList const& path) {
	if (path.empty()) return scopeStack.back();
	if (path.size() == 1) {
		if (scopeStack.empty() && root->subspaces.contains(path.front())) {
			scopeStack.pushBack(root->subspaces[path.front()]);
			return scopeStack.back();
		}
		else if (scopeStack.size() && scopeStack.back()->subspaces.contains(path.front())) {
			scopeStack.pushBack(scopeStack.front()->subspaces[path.front()]);
			return scopeStack.back();
		}
	}
	Namespace::Instance ns = ns.create(path.front());
	if (scopeStack.empty())
		root->subspaces[ns->name] = ns;
	else scopeStack.back()->subspaces[ns->name] = ns;
	scopeStack.pushBack(ns);
	if (path.size() > 1)
		return push(path.sliced(1));
	return scopeStack.back();
}

void Intermediate::pop(usize count) {
	while (scopeStack.size() && count--) {
		auto const scope = scopeStack.popBack();
	}
}

void Implementation::addPreLine(UTF8String const& what) {
	pre.pushBack(what);
}

void Implementation::addMainLine(UTF8String const& what) {
	main.pushBack(what);
}

void Implementation::addPostLine(UTF8String const& what) {
	post.pushBack(what);
}

void Intermediate::addPreLine(UTF8String const& what) {
	root->impl->addPreLine(what);
}

void Intermediate::addMainLine(UTF8String const& what) {
	root->impl->addMainLine(what);
}

void Intermediate::addPostLine(UTF8String const& what) {
	root->impl->addPostLine(what);
}

Function::OverloadRef Function::overloadFromVariables(List<Namespace::VariableRef> const& args, FuzzySearch const fuzz) const {
	return overloadFromTypes(args.toList<Namespace::TypeRef>([] (auto const& e) -> Namespace::TypeRef {return e->type.asStrong();}), fuzz);
}

static bool const test(Namespace::TypeRef const& a, Namespace::TypeRef const& b, bool const fuzzy) {
	if (fuzzy) return TypeDecl::stronger(a, b) != b;
	else return a != b;
}

static bool validate(Function::OverloadRef ov, Makai::List<Namespace::TypeRef> const& args, Function::FuzzySearch const fuzz) {
	MAKAILIB_DEBUGLN_FULL("");
	MAKAILIB_DEBUGLN_FULL("[", args.size(), " :: ", ov->arguments.size(), "]");
	if (ov->variadic && (args.size()+1) < ov->arguments.size()) return false;
	else if (!ov->variadic && args.size() != ov->arguments.size()) return false;
	auto const paramc = ov->arguments.size();
	auto const isFuzzy = [fuzz] (usize const i) {
		switch (fuzz) {
			using enum Function::FuzzySearch;
			case AV2_TCF_FS_NONE:				return false;
			case AV2_TCF_FS_ALL_ARGS:			return true;
			case AV2_TCF_FS_ALL_EXCEPT_FIRST:	return i > 0;
		}
		return false;
	};
	for (auto const& [arg, index] : Makai::Range::expand(args)) {
		MAKAILIB_DEBUG_FULL("Testing arg[", index, "]... ");
		auto const pIndex = (index < paramc-1 ? index : paramc-1);
		auto const param = ov->arguments[pIndex]->type;
		if (!(arg.exists() and param.exists())) return false;
		auto const inVariadicRegion = ov->variadic && index >= (paramc-1);
		MAKAILIB_DEBUGLN_FULL(arg->name, " :: ", (inVariadicRegion && param->base) ? param->base->name : param->name);
		if (inVariadicRegion && !param->flags.isArray)
			return false;
		if (arg == param) continue;
		if (test(arg, inVariadicRegion ? param->base : Namespace::TypeRef(param.asStrong()), inVariadicRegion || isFuzzy(index)))
			return false;
	}
	return true;
}

Function::OverloadRef Function::overloadFromTypes(List<Namespace::TypeRef> const& args, FuzzySearch const fuzz) const {
	decltype(overloads) matches;
	for (auto& ov: overloads) {
		MAKAILIB_DEBUG_FULL("");
		if (!ov) continue;
		MAKAILIB_DEBUG_FULL("Testing ", ov->entry, "... ");
		if (!validate(ov, args, fuzz)) {
			MAKAILIB_DEBUGLN_FULL("no match");
			continue;
		}
		MAKAILIB_DEBUGLN_FULL("matched!");
		if (fuzz != FuzzySearch::AV2_TCF_FS_NONE) {
			if (validate(ov, args, FuzzySearch::AV2_TCF_FS_NONE))
				return ov;
			matches.pushBack(ov);
		}
		else return ov;
	}
	if (fuzz == FuzzySearch::AV2_TCF_FS_NONE or matches.empty()) return nullptr;
	if (matches.size() == 1) return matches.back();
	Function::OverloadRef match;
	for (auto& ov: matches) {
		if (!ov) continue;
		if (validate(ov, args, FuzzySearch::AV2_TCF_FS_NONE))
			return ov;
		if (!match or match->arguments.size() < ov->arguments.size())
			match = ov;
	}
	return match;
}

Implementation::Instance Namespace::compose() const {
	Implementation::Instance out = out.create();
	if (function) {
		for (auto& ov: function->current)
			out->writePreLine(ov->prototype());
		return out;
	}
	if (impl) {
		if (!variable && varc)
			out->writePreLine("enter", varc);
		if (isPureNamespace())
			out->writePreLine(varc ? "keep" : "begin");
		if (!variable) out->writePostLine("end");
		if (implementContents)
			out->writeMainLine(impl->toString());
	}
	return out;
}

Namespace::TypeRef TypeDecl::stronger(Namespace::TypeRef const& a, Namespace::TypeRef const& b) {
	if (!(a && b))			return nullptr;
	if (a == b)				return a;
	if (a->derivedFrom(b))	return b;
	if (b->derivedFrom(a))	return a;
	if (!(a->basic.exists() && b->basic.exists()))
		return nullptr;
	else {
		auto const at = *a->basic;
		auto const bt = *b->basic;
		if (at == bt) return a;
		if (Core::isAlgebraic(at) && Core::isAlgebraic(bt))
			return (at > bt ? a : b);
		if (Core::isText(at) && Core::isText(bt))
			return Core::isString(at) ? a : b;
	}
	return nullptr;
}

static Attribute::Target fromString(Makai::UTF8String const& name) {
	if (name == "struct")	return Attribute::Target::AV2_TAAT_TYPE;
	if (name == "func")		return Attribute::Target::AV2_TAAT_FUNCTION;
	if (name == "prop")		return Attribute::Target::AV2_TAAT_PROPERTY;
	if (name == "module")	return Attribute::Target::AV2_TAAT_NAMESPACE;
	if (name == "var")		return Attribute::Target::AV2_TAAT_VARIABLE;
	if (name == "attr")		return Attribute::Target::AV2_TAAT_ATTRIBUTE;
	if (name == "prop")		return Attribute::Target::AV2_TAAT_PROPERTY;
	return Attribute::Target::AV2_TAAT_EMPTY;
}

static Namespace::AttributeRef createMetaAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Attribute";
	attrib->fields["target"]	= {DVK_STRING								};
	attrib->fields["min"]		= {DVK_UNSIGNED, 0							};
	attrib->fields["max"]		= {DVK_UNSIGNED, Makai::Limit::MAX<uint64>	};
	attrib->target = Attribute::Target::AV2_TAAT_TYPE;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		if (!(ns->type && ns->type->def == TypeDecl::Definition::AV2_TCTD_STRUCT))
			Transformer::ATransformer::Context::error("Expected structure here!", ns->node);
		auto const attrib = Namespace::AttributeRef::create();
		attrib->name			= ns->type->name;
		attrib->baseTypeHash	= Makai::hash(attrib->name);
		attrib->target			= fromString(v.fetch<Makai::UTF8String>("target", "func"));
		attrib->globalMin		= v.fetch<uint64>("min", 0);
		attrib->globalMax		= v.fetch<uint64>("max", Makai::Limit::MAX<uint64>);
		for (auto const& [name, field]: ns->subspaces) {
			if (!field->variable)
				continue;
			auto const& var = field->variable;
			if (!var->type->basic)
				Transformer::ATransformer::Context::error("Variable type must be a basic type!", var->node);
			if (var->defaulted && !var->value)
				Transformer::ATransformer::Context::error("Attribute field defaults must have constant values!", var->node);
			if (attrib->fields.contains(name))
				Transformer::ATransformer::Context::error("Redeclaration of previously-declared field!", var->node);
			Makai::Data::Value::Kind kind;
			switch (*var->type->basic) {
				case AV2_BT_BOOL: kind = DVK_BOOLEAN;
				case AV2_BT_STRING: kind = DVK_STRING;
				case AV2_BT_INT8:
				case AV2_BT_INT16:
				case AV2_BT_INT32:
				case AV2_BT_INT64: kind = DVK_SIGNED;
				case AV2_BT_UINT8:
				case AV2_BT_UINT16:
				case AV2_BT_UINT32:
				case AV2_BT_UINT64: kind = DVK_UNSIGNED;
				case AV2_BT_REAL32:
				case AV2_BT_REAL64:
				case AV2_BT_REAL128: kind = DVK_REAL;
				default: Transformer::ATransformer::Context::error("Invalid basic type for attribute!", var->node);
			}
			attrib->fields[name] = {kind, var->value, var->scope->meta.contains("Path")};
			attrib->fieldMap.pushBack(name);
		}
		attrib->transform = ATTRIBUTE_TRANSFORMER() {
			auto& meta = ns->meta[base.name]->value["::meta"];
			meta["name"]	= base.name.toString();
			meta["hash"]	= base.baseTypeHash;
			meta["map"]		= base.fieldMap.toList<Makai::Data::Value>(
				[] (Makai::UTF8String const& e) -> Makai::Data::Value {
					return e.toString();
				}
			);
		};
		ns->attribute = attrib;
	};
	return attrib;
}

static Namespace::AttributeRef createMetaTransformerAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Transformer";
	attrib->fields["of"] = {DVK_STRING, true};
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {

	};
	return attrib;
}

static Namespace::AttributeRef createOperatorAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Operator";
	attrib->fields["prefix"]	= {DVK_STRING, ""	};
	attrib->fields["infix"]		= {DVK_STRING, ""	};
	attrib->fields["postfix"]	= {DVK_STRING, ""	};
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
	};
	return attrib;
}

static Namespace::AttributeRef createConverterAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Converter";
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
	};
	return attrib;
}

static Namespace::AttributeRef createGetterAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Getter";
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
	};
	return attrib;
}

static Namespace::AttributeRef createSetterAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Setter";
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
	};
	return attrib;
}

static Namespace::AttributeRef createGlobalAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Global";
	attrib->target = Attribute::Target::AV2_TAAT_VARIABLE;
	attrib->fields["source"] = {DVK_STRING};
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		static Makai::UTF8Dictionary<Namespace::TypeRef> globalTypes;
		if (ns->variable->global)
			Transformer::ATransformer::Context::error("Variable cannot be both Global and Static!", ns->node);
		if (ns->variable->initializer)
			Transformer::ATransformer::Context::error("Globals cannot have initializers!", ns->node);
		ns->variable->global = true;
		ns->variable->staticEntity = true;
		auto const srcName = v["source"].getString().replace('\\', '/').replace('/', '.');
		if (globalTypes.contains(srcName) && globalTypes[srcName] != ns->variable->type.asStrong())
			Transformer::ATransformer::Context::error("Global variable type mismatch!", ns->node);
		ns->variable->source = "$" + srcName;
	};
	return attrib;
}

static Namespace::AttributeRef createPassByAttribute(Makai::UTF8String const mode) {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "By" + mode;
	attrib->target = Attribute::Target::AV2_TAAT_VARIABLE;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		if (ns->variable->isConstant)
				Transformer::ATransformer::Context::error("Cannot change transfer mode for constants!", ns->node);
		if (mode == "Copy") {
			if (!ns->variable->type->flags.isCopyable)
				Transformer::ATransformer::Context::error("Variable is not of a copyable type!", ns->node);
			ns->variable->passBy = "val";
		}
		else if (mode == "Ref")		ns->variable->passBy = "ref";
		else if (mode == "Move")	ns->variable->passBy = "move";
	};
	return attrib;
}

static Namespace::AttributeRef createPathAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Path";
	attrib->target = Attribute::Target::AV2_TAAT_VARIABLE;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		// Only checked inside Attribute structs
	};
	return attrib;
}

static Namespace::AttributeRef createSharedAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Shared";
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->fields["name"]		= {.type=DVK_STRING};
	attrib->fields["lib"]		= {.type=DVK_STRING, .path=true};
	attrib->fields["optional"]	= {.type=DVK_BOOLEAN, .defaultValue=false};
	attrib->fields["version"]	= {.type=DVK_STRING, .defaultValue="latest", .path = true};
	attrib->fields["key"]		= {.type=DVK_STRING, .defaultValue=""};
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		static usize id = 0;
		auto const name = (v["name"].getString());
		auto const lib = (v["lib"].getString());
		auto const version = (v["version"].getString()).replaced('/', '.');
		auto const key = (v["key"].getString());
		bool hit = false;
		for (auto& ov: ns->function->current)
			if (ov->variant == Function::Overload::Variant::External::AV2_TCB_FO_VE_NONE && !ov->hasImplementation) {
				if (ov->variant.context > ExecutionContext::AV2_TCB_EC_RUNTIME)
					continue;
				if (ov->variant.context == ExecutionContext::AV2_TCB_EC_NONE)
					ov->variant.context = ExecutionContext::AV2_TCB_EC_RUNTIME;
				ov->variant = Function::Overload::Variant::External::AV2_TCB_FO_VE_DYNLIB;
				hit = true;
				MAKAILIB_DEBUGLN_FULL("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ Applying shared attribute...");
				ov->hasImplementation = true;
				ov->outEntry = name;
				ov->dynlib = lib;
				if (version.size() && version != "latest")
					ov->dynlib += + "?ver='" + version + "'";
				if (key.size())
					ov->dynlib += "?key='" + Makai::Convert::toBase<Makai::Convert::Base::CB_BASE64>(
						Makai::Tool::Arch::hashPassword(key)
							.toBytes()
					)  + "'";
				ov->optional = v["optional"];
				ov->entry = "__shared_dynlib_" + Makai::toString(++id) + ov->entry;
			}
		if (!hit)
			Transformer::ATransformer::Context::error("Missing valid shared function declaration!", ns->node);
	};
	return attrib;
}

static Namespace::AttributeRef createNativeAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Native";
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->fields["name"] = {DVK_STRING};
	attrib->fields["optional"]	= {.type=DVK_STRING, .defaultValue=false, .path=true};
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		static usize id = 0;
		auto const name = (v["name"].getString());
		bool hit = false;
		for (auto& ov: ns->function->current)
			if (ov->variant == Function::Overload::Variant::External::AV2_TCB_FO_VE_NONE && !ov->hasImplementation) {
				if (ov->variant.context > ExecutionContext::AV2_TCB_EC_RUNTIME)
					continue;
				if (ov->variant.context == ExecutionContext::AV2_TCB_EC_NONE)
					ov->variant.context = ExecutionContext::AV2_TCB_EC_RUNTIME;
				ov->variant = Function::Overload::Variant::External::AV2_TCB_FO_VE_ART_CALL;
				hit = true;
				ov->hasImplementation = true;
				ov->outEntry = name;
				ov->optional = v["optional"];
				ov->entry = "__art_call_" + Makai::toString(id) + ov->entry;
			}
		if (!hit)
			Transformer::ATransformer::Context::error("Missing valid internal call declaration!", ns->node);
	};
	return attrib;
}

static Namespace::AttributeRef createStaticAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Static";
	attrib->target = Attribute::Target::AV2_TAAT_VARIABLE | Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		static usize id = 0;
		if (ns->variable) {
			if (ns->variable->global)
				Transformer::ATransformer::Context::error("Variable cannot be both Global and Static!", ns->node);
			ns->variable->global = true;
			ns->variable->staticEntity = true;
			ns->variable->source = "move $__STATIC__._ns_" + Makai::toString(++id) + "._ns_" + ns->node->name() + "._" + ns->variable->name;
		} else if (ns->function) {
			for (auto& ov: ns->function->current)
				if (ov->variant == Function::Overload::Variant::Object::AV2_TCB_FO_VO_NONE) {
					ov->variant = Function::Overload::Variant::Object::AV2_TCB_FO_VO_GLOBAL;
					ov->staticEntity = true;
				}
		}
	};
	return attrib;
}

static Namespace::AttributeRef createVariadicAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Variadic";
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		for (auto& ov: ns->function->current)
			if (ov->arguments.size() && ov->arguments.back()->type->flags.isArray)
				ov->variadic = true;
	};
	return attrib;
}

static Namespace::AttributeRef createRemangleAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Remangle";
	attrib->fields["name"] = {.type=DVK_STRING, .path=true};
	attrib->target = Attribute::Target::AV2_TAAT_TYPE | Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
	};
	return attrib;
}

static Namespace::AttributeRef createDoNotMangleAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "DoNotMangle";
	attrib->target = Attribute::Target::AV2_TAAT_TYPE | Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
	};
	return attrib;
}

static Namespace::AttributeRef createMemberAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Member";
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {;
		for (auto& ov: ns->function->current)
			if (ov->variant == Function::Overload::Variant::Object::AV2_TCB_FO_VO_NONE) {
				ov->variant = Function::Overload::Variant::Object::AV2_TCB_FO_VO_CLASS;
			}
	};
	return attrib;
}

static Namespace::AttributeRef createNullableAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Nullable";
	attrib->target = Attribute::Target::AV2_TAAT_TYPE;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		ns->type->flags.isNullable = true;
	};
	return attrib;
}

static Namespace::AttributeRef createEmptyAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Empty";
	attrib->target = Attribute::Target::AV2_TAAT_TYPE;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		ns->type->flags.isEmpty = true;
		if (ns->type->fields.size() > 1)
			Transformer::ATransformer::Context::error("Empty type must not contain fields!", ns->node);
	};
	return attrib;
}

static Namespace::AttributeRef createDiscardAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Discard";
	attrib->target = Attribute::Target::AV2_TAAT_TYPE;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		ns->type->flags.hasNoResult = true;
	};
	return attrib;
}

static Namespace::AttributeRef createDynamicAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Dynamic";
	attrib->target = Attribute::Target::AV2_TAAT_TYPE;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		ns->type->flags.isDynamic = true;
		if (ns->type->fields.size())
			Transformer::ATransformer::Context::error("Dynamic type must not contain fields!", ns->node);
	};
	return attrib;
}

static Namespace::AttributeRef createCopyAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Copy";
	attrib->target = Attribute::Target::AV2_TAAT_TYPE;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		for (auto& [name, field]: ns->type->fields) {
			if (!field->type->flags.isCopyable)
				Transformer::ATransformer::Context::error("Copyable types must only contain copyable fields!", ns->node);
			ns->variable->passBy = "val";
		}
		ns->type->flags.isCopyable = true;
	};
	return attrib;
}

static Namespace::AttributeRef createFinalAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Final";
	attrib->target = Attribute::Target::AV2_TAAT_TYPE;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		ns->type->flags.isFinal = true;
	};
	return attrib;
}

static Namespace::AttributeRef createValueAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Value";
	attrib->target = Attribute::Target::AV2_TAAT_TYPE;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		if (ns->type->flags.isArray && ns->type->base->flags.isArray)
			Transformer::ATransformer::Context::error("Value arrays of arrays are disallowed!", ns->node);
		if (ns->type->flags.isArray && !ns->type->base->flags.isValueType)
			Transformer::ATransformer::Context::error("Value arrays of non-value-types are disallowed!", ns->node);
		for (auto& [name, field]: ns->type->fields) {
			if (!field->type->flags.isValueType)
				Transformer::ATransformer::Context::error("Value types must only contain value fields!", ns->node);
			ns->variable->passBy = "val";
		}
		ns->type->flags.isValueType = true;
	};
	return attrib;
}

static Namespace::AttributeRef createBasicAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Basic";
	attrib->target = Attribute::Target::AV2_TAAT_TYPE;
	attrib->fields["type"] = {DVK_STRING};
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		auto const bt = (v["type"].getString());
		ns->type->def = TypeDecl::Definition::AV2_TCTD_BASIC;
		ns->type->flags.isStructure = false;
		ns->type->flags.isBasic = true;
		if (bt != "any") {
			ns->type->flags.isCopyable = true;
			ns->type->flags.isFinal = true;
		}
		if (ns->type->fields.size() > 2) {
			Transformer::ATransformer::Context::error("Basic type must not contain fields!", ns->node);
		} else if (ns->type->fields.size() > 1 && !ns->type->fields.contains("base")) {
			Transformer::ATransformer::Context::error("Basic type must not contain fields!", ns->node);
		}
		ns->type->pureName = bt;
		if (bt == "void")			{ns->type->basic = AV2_BT_VOID;		}
		else if (bt == "bool")		{ns->type->basic = AV2_BT_BOOL;		}
		else if (bt == "int8")		{ns->type->basic = AV2_BT_INT8;		}
		else if (bt == "uint8")		{ns->type->basic = AV2_BT_UINT8;	}
		else if (bt == "int16")		{ns->type->basic = AV2_BT_INT16;	}
		else if (bt == "uint16")	{ns->type->basic = AV2_BT_UINT16;	}
		else if (bt == "int32")		{ns->type->basic = AV2_BT_INT32;	}
		else if (bt == "uint32")	{ns->type->basic = AV2_BT_UINT32;	}
		else if (bt == "int64")		{ns->type->basic = AV2_BT_INT64;	}
		else if (bt == "uint64")	{ns->type->basic = AV2_BT_UINT64;	}
		else if (bt == "float32")	{ns->type->basic = AV2_BT_REAL32;	}
		else if (bt == "float64")	{ns->type->basic = AV2_BT_REAL64;	}
		else if (bt == "float128")	{ns->type->basic = AV2_BT_REAL128;	}
		else if (bt == "vector")	{ns->type->basic = AV2_BT_VECTOR;	}
		else if (bt == "bytes")		{ns->type->basic = AV2_BT_BYTES;	}
		else if (bt == "matrix")	{ns->type->basic = AV2_BT_MATRIX;	}
		else if (bt == "type")		{ns->type->basic = AV2_BT_TYPEID;	}
		else if (bt == "string")	{ns->type->basic = AV2_BT_STRING;	}
		else if (bt == "char")		{ns->type->basic = AV2_BT_CHAR;		}
		else if (bt == "any")		{ns->type->basic = AV2_BT_ANY;		}
		else if (bt == "null")		{ns->type->basic = AV2_BT_NULL;		}
		else Transformer::ATransformer::Context::error("Invalid basic type!", ns->node);
		switch (*ns->type->basic) {
			case AV2_BT_STRING:
			case AV2_BT_BYTES:
			case AV2_BT_VOID:
			case AV2_BT_NULL: break;
			default: ns->type->flags.isValueType = true; break;
		}
	};
	return attrib;
}

static Namespace::AttributeRef createBoundAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Proxy";
	attrib->target = Attribute::Target::AV2_TAAT_TYPE;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		ns->type->flags.isProxy = true;
	};
	return attrib;
}

static Namespace::AttributeRef createBeforeAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Before";
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->globalMin = 0;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		Makai::Handle<Function::Overload> fn;
		MAKAILIB_DEBUGLN_FULL("Overloads: ", ns->function->current.size());
		for (auto const& ov: ns->function->current)
			if (ov->arguments.empty()) {
				fn = ov.asWeak();
				break;
			}
		if (!fn)
			Transformer::ATransformer::Context::error("No valid function overload found!", ns->node);
		inter.before.pushBack(fn);
		inter.before.back()->uses++;
	};
	return attrib;
}

static Namespace::AttributeRef createAfterAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "After";
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->globalMin = 0;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		Makai::Handle<Function::Overload> fn;
		MAKAILIB_DEBUGLN_FULL("Overloads: ", ns->function->current.size());
		for (auto const& ov: ns->function->current)
			if (ov->arguments.empty()) {
				fn = ov.asWeak();
				break;
			}
		if (!fn)
			Transformer::ATransformer::Context::error("No valid function overload found!", ns->node);
		inter.after.pushBack(fn);
		inter.after.back()->uses++;
	};
	return attrib;
}

static Namespace::AttributeRef createMainAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Main";
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->globalMin = 0;
	attrib->globalMax = 1;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		if (inter.main)
			Transformer::ATransformer::Context::error("Redeclaration of previously-declared main!", ns->node);
		for (auto const& ov: ns->function->current)
			if (ov->arguments.empty()) {
				inter.main = ov.asWeak();
				break;
			}
		if (!inter.main)
			Transformer::ATransformer::Context::error("No valid function overload for main!", ns->node);
		inter.main->uses++;
	};
	return attrib;
}

static Namespace::AttributeRef createExposeAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Expose";
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->fields["name"] = {.type = DVK_STRING, .defaultValue = ""};
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		auto const name = (v["name"] ? v["name"].getString() : ns->function->pureName.toString());
		if (ns->function->sigCall) return;
		for (auto& ov: ns->function->current)
			if (ov->sigEntry.empty()) {
				ov->sigEntry =
					name
				+	"("
				+	ov->arguments.toList<Makai::String>(
						[] (auto const& e) -> Makai::String {
							return e->type->basic ? e->type->pureName : e->type->name;
						}
					).join(",")
				+	")"
				;
				ns->function->sigCall = ov;
				break;
			}
	};
	return attrib;
}

static Namespace::AttributeRef createRuntimeAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Indirect";
	attrib->target =
		Attribute::Target::AV2_TAAT_FUNCTION
	|	Attribute::Target::AV2_TAAT_VARIABLE
	;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		for (auto& ov: ns->function->current) {
			if (ov->variant.context > ExecutionContext::AV2_TCB_EC_NONE)
				continue;
			ov->variant.context = ExecutionContext::AV2_TCB_EC_RUNTIME;
		}
	};
	return attrib;
}

static Namespace::AttributeRef createMixedAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Mixed";
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		for (auto& ov: ns->function->current) {
			if (ov->variant.context > ExecutionContext::AV2_TCB_EC_NONE)
				continue;
			ov->variant.context = ExecutionContext::AV2_TCB_EC_MIXED;
		}
	};
	return attrib;
}

static Namespace::AttributeRef createDirectAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Direct";
	attrib->target =
		Attribute::Target::AV2_TAAT_FUNCTION
	|	Attribute::Target::AV2_TAAT_VARIABLE
	;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		if (ns->function) {
			for (auto& ov: ns->function->current) {
				if (ov->variant.context > ExecutionContext::AV2_TCB_EC_NONE)
					continue;
				ov->variant.context = ExecutionContext::AV2_TCB_EC_COMPILE;
			}
		} else if (ns->variable) {
			if (ns->variable->value.isUndefined())
				Transformer::ATransformer::Context::error("Variable does not possess a direct value!", ns->node);
			ns->variable->context = ExecutionContext::AV2_TCB_EC_COMPILE;
			ns->variable->isConstant = true;
			ns->variable->passBy = "copy";
		}
	};
	return attrib;
}

static Namespace::AttributeRef createConstAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Const";
	attrib->target =
		Attribute::Target::AV2_TAAT_VARIABLE
	;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		if (!ns->variable->type->flags.isCopyable)
			Transformer::ATransformer::Context::error("Variable is not of a copyable type!", ns->node);
		ns->variable->isConstant = true;
		ns->variable->passBy = "copy";
	};
	return attrib;
}

static Namespace::AttributeRef createPrivateAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Private";
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		ns->makePrivate();
	};
	return attrib;
}

static Namespace::AttributeRef createProtectedAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Protected";
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		ns->makeProtected();
	};
	return attrib;
}

static Namespace::AttributeRef createPublicAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Public";
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		ns->makePublic();
	};
	return attrib;
}

static Namespace::AttributeRef createTransformerAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Transformer";
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		bool hit = false;
		for (auto& ov: ns->function->current) {
			if (ov->variant.context > ExecutionContext::AV2_TCB_EC_NONE)
				continue;
			ov->variant.context = ExecutionContext::AV2_TCB_EC_MIXED;
			hit = true;
		}
		if (!hit)
			Transformer::ATransformer::Context::error("Missing valid transformer declaration!", ns->node);
	};
	return attrib;
}

bool Attribute::matchesTarget(Namespace const& ns, Target const target) {
	using enum Lexer::CStyle::TokenStream::Token::Type;
	if (target == Target::AV2_TAAT_EMPTY)
		return false;
	if (ns.variable && enumcast(target & Target::AV2_TAAT_VARIABLE))
		return true;
	if (ns.type && enumcast(target & Target::AV2_TAAT_TYPE))
		return true;
	if (ns.property && enumcast(target & Target::AV2_TAAT_PROPERTY))
		return true;
	if (ns.function && enumcast(target & Target::AV2_TAAT_FUNCTION))
		return true;
	if (ns.trait && enumcast(target & Target::AV2_TAAT_TRAIT))
		return true;
	if (ns.isPureNamespace() && enumcast(target & Target::AV2_TAAT_NAMESPACE))
		return true;
	return false;
}

void Intermediate::addGlobalAttribute(Namespace::AttributeRef const& attrib) {
	auto const scope = Namespace::Instance::create();
	scope->name = attrib->name;
	scope->attribute = attrib;
	root->subspaces[attrib->name] = scope;
}

Intermediate::Intermediate() {
	addGlobalAttribute(createMetaAttribute());
	addGlobalAttribute(createMetaTransformerAttribute());
	addGlobalAttribute(createOperatorAttribute());
	addGlobalAttribute(createNullableAttribute());
	addGlobalAttribute(createEmptyAttribute());
	addGlobalAttribute(createDiscardAttribute());
	addGlobalAttribute(createDynamicAttribute());
	addGlobalAttribute(createCopyAttribute());
	addGlobalAttribute(createFinalAttribute());
	addGlobalAttribute(createBasicAttribute());
	addGlobalAttribute(createValueAttribute());
	addGlobalAttribute(createBoundAttribute());
	addGlobalAttribute(createGlobalAttribute());
	addGlobalAttribute(createStaticAttribute());
	addGlobalAttribute(createBeforeAttribute());
	addGlobalAttribute(createMainAttribute());
	addGlobalAttribute(createAfterAttribute());
	addGlobalAttribute(createGetterAttribute());
	addGlobalAttribute(createSetterAttribute());
	addGlobalAttribute(createConverterAttribute());
	addGlobalAttribute(createMemberAttribute());
	addGlobalAttribute(createSharedAttribute());
	addGlobalAttribute(createNativeAttribute());
	addGlobalAttribute(createPathAttribute());
	addGlobalAttribute(createRemangleAttribute());
	addGlobalAttribute(createDoNotMangleAttribute());
	addGlobalAttribute(createConstAttribute());
	addGlobalAttribute(createPassByAttribute("Move"));
	addGlobalAttribute(createPassByAttribute("Ref"));
	addGlobalAttribute(createPassByAttribute("Copy"));
	addGlobalAttribute(createPrivateAttribute());
	addGlobalAttribute(createProtectedAttribute());
	addGlobalAttribute(createPublicAttribute());
	addGlobalAttribute(createExposeAttribute());
	addGlobalAttribute(createRuntimeAttribute());
	addGlobalAttribute(createMixedAttribute());
	addGlobalAttribute(createDirectAttribute());
	addGlobalAttribute(createTransformerAttribute());
	addGlobalAttribute(createVariadicAttribute());
}

Makai::Data::Value Implementation::serialize() const {
	Makai::Data::Value out = out.object();
	out["pre"]	= pre.join("\n").toString();
	out["main"]	= main.join("\n").toString();
	out["post"]	= post.join("\n").toString();
	return out;
}

Makai::Data::Value Namespace::serialize() const {
	Makai::Data::Value out = out.object();
	out["name"] = name.toString();
	if (function)	out["fn"]		= function->serialize();
	if (type)		out["type"]		= type->serialize();
	if (variable)	out["var"]		= variable->serialize();
	if (attribute)	out["attr"]		= attribute->serialize();
	if (trait)		out["trait"]	= trait->serialize();
	if (property)	out["prop"]		= property->serialize();
	for (auto& [name, props]: meta) {
		out["meta"][name]["name"]	= props->attribute->name.toString();
		out["meta"][name]["value"]	= props->value;
	}
	for (auto& [name, ns]: subspaces)
		out["sub_ns"][name]	= ns->serialize();
	out["varc"]	= varc;
	out["impl"] = impl->serialize();
	return out;
}

Makai::Data::Value Function::serialize() const {
	Makai::Data::Value out = out.object();
	out["name"] = name.toString();
	for (auto const ov: Makai::Range::expand(overloads))
		out["overloads"][ov.index] = ov.value->serialize();
	return out;
}

Makai::Data::Value Function::Overload::serialize() const {
	Makai::Data::Value out = out.object();
	out["ret"] = result->name.toString();
	for (auto const arg: Makai::Range::expand(arguments))
		out["args"][arg.index] = arg.value->serialize();
	out["entry"] = entry.toString();
	if (scope)
		out["scope"] = scope->serialize();
	if (methodOf)
		out["method_of"] = methodOf->name.toString();
	switch (variant.external) {
		using enum Variant::External;
		case AV2_TCB_FO_VE_NONE:		out["extern"] = "none";		break;
		case AV2_TCB_FO_VE_ART_CALL:	out["extern"] = "Expose";	break;
		case AV2_TCB_FO_VE_DYNLIB:		out["extern"] = "dynlib";	break;
	}
	switch (variant.object) {
		using enum Variant::Object;
		case AV2_TCB_FO_VO_NONE:		out["variant"] = "none";		break;
		case AV2_TCB_FO_VO_INSTANCE:	out["variant"] = "instance";	break;
		case AV2_TCB_FO_VO_CLASS:		out["variant"] = "class";		break;
		case AV2_TCB_FO_VO_GLOBAL:		out["variant"] = "static";		break;
	}
	return out;
}

Makai::Data::Value Variable::serialize() const {
	Makai::Data::Value out = out.object();
	out["type"] = type->name.toString();
	if (initializer)
		out["init"] = initializer->serialize();
	out["src"] = source.toString();
	if (value)
		out["direct"] = value;
	if (global || staticEntity)
		out["global"] = true;
	if (fieldOf)
		out["field_of"] = fieldOf->name.toString();
	out["id"] = id;
	return out;
}

Makai::Data::Value Trait::serialize() const {return {};}

Makai::Data::Value TypeDecl::serialize() const {
	Makai::Data::Value out = out.object();
	out["name"] = name.toString();
	if (base)
		out["base"] = base->name.toString();
	switch (def) {
		case Definition::AV2_TCTD_ARRAY: out["def"] = "array"; break;
		case Definition::AV2_TCTD_STRUCT: out["def"] = "struct"; break;
		case Definition::AV2_TCTD_TEMPLATE: out["def"] = "template"; break;
		case Definition::AV2_TCTD_BASIC: out["def"] = "basic"; break;
		case Definition::AV2_TCTD_ENUM: out["def"] = "enum"; break;
	}
	if (basic)
		// TODO: Not this
		out["basic"] = enumcast(*basic);
	if (artEquivalent)
		out["art_type"] = artEquivalent.value().toString();
	for (auto const& [name, field]: fields)
		if (field)
			out["fields"][name.toString()] = field->serialize();
	return out;
}

Makai::Data::Value Attribute::serialize() const {
	Makai::Data::Value out = out.object();
	StringList tg;
	if (enumcast(target & Target::AV2_TAAT_ATTRIBUTE))	tg.pushBack("attr");
	if (enumcast(target & Target::AV2_TAAT_TYPE))		tg.pushBack("struct");
	if (enumcast(target & Target::AV2_TAAT_VARIABLE))	tg.pushBack("var");
	if (enumcast(target & Target::AV2_TAAT_FUNCTION))	tg.pushBack("fn");
	if (enumcast(target & Target::AV2_TAAT_NAMESPACE))	tg.pushBack("module");
	if (enumcast(target & Target::AV2_TAAT_PROPERTY))	tg.pushBack("prop");
	out["target"] = tg.toList<Makai::Data::Value>();
	out["uses"] = useCount;
	out["min"] = globalMin;
	out["max"] = globalMax;
	if(fieldMap.size()) out["map"] = fieldMap.toList<Makai::Data::Value>(
		[] (Makai::UTF8String const& e) -> Makai::Data::Value {
			return e.toString();
		}
	);
	if (baseTypeHash)
		out["hash"] = baseTypeHash;
	for (auto const& [name, field]: fields) {
		auto& f = out["fields"][name.toString()];
		f["type"] = Makai::Data::Value::asNameString(field.type);
		if (field.defaultValue)
			f["default"] = field.defaultValue;
		f["is_path"] = field.path;
	}
	return out;
}

Makai::Data::Value Property::serialize() const {
	Makai::Data::Value out = out.object();
	if (type)	out["type"]	= type->name.toString();
	if (getter)	out["get"]	= getter->serialize();
	if (setter)	out["set"]	= setter->serialize();
	if (fieldOf)
		out["field_of"] = fieldOf->name.toString();
	return out;
}

Makai::Data::Value Intermediate::serialize() const {
	Makai::Data::Value out = out.object();
	out["root"] = root->serialize();
	out["stack"] = scopeStack.size();
	return out;
}

bool Namespace::isPureNamespace() const {
	return !(type || function || variable || attribute || trait);
}

bool TypeDecl::derivedFrom(Namespace::TypeRef const& other) const {
	if (!base) return false;
	if (base == other) return true;
	return base->derivedFrom(other);
}

Namespace::Instance Intermediate::top() const {
	if (scopeStack.empty()) return root;
	return scopeStack.back();
}

Namespace::Instance Intermediate::parent() const {
	if (scopeStack.size() < 2) return root;
	return scopeStack[-2];
}

Implementation::Instance Intermediate::impl() const {
	return top()->impl;
}

Makai::UTF8String Function::Overload::prototype() const {
	Makai::UTF8String out = "_Rt0" + (result ? result->name : "!ERR!");
	for (auto const& [arg, index]: Makai::Range::expand(arguments))
		out += "_At" + Makai::toString(index) + (arg->type ? arg->type->name : "!ERR!");
	return out;
}

Function::Overload::Overload() {}
Function::Overload::~Overload() {}
