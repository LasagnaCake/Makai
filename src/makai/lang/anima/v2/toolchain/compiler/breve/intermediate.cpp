#include "intermediate.hpp"
#include "transformer.hpp"

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;

IWritable::~IWritable() {}

#define ATTRIBUTE_TRANSFORMER()  [] (Intermediate& inter, Namespace::Instance const& ns, Makai::Data::Value const& v, Attribute& base)

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
	if (!subspaces.contains(path.front())) return nullptr;
	if (path.size() == 1)
		return subspaces[path.front()];
	else return subspaces[path.front()]->resolve(path.sliced(1));
	return nullptr;
}

Namespace::Instance Intermediate::resolve(UTF8StringList const& path) const {
	if (path.empty()) return nullptr;
	DEBUGLN("Looking for '/", path.join("/"), "'");
	for (auto& scope: Makai::Range::reverse(scopeStack)) {
		DEBUGLN("Scope: ", scope->name);
		DEBUG("Subspaces: [ ");
		for (auto const& [name, subns]: scope->subspaces)
			DEBUG( "{", name , ":", subns->name, "} ");
		DEBUGLN("]");
		if (scope->name == path.front()) {
			if (path.size() == 1)
				return scope;
			else if (auto const ns = scope->resolve(path.sliced(1)))
				return ns;
		} else if (auto const ns = scope->resolve(path)) return ns;
		DEBUGLN("Nope!");
	}
	DEBUGLN("Global scope");
	if (auto const ns = root->resolve(path))
		return ns;
	DEBUGLN("Nope!");
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

void Implementation::writePre(UTF8String const& what) {
	pre += " " + what;
}

void Implementation::writeMain(UTF8String const& what) {
	main += " " + what;
}

void Implementation::writePost(UTF8String const& what) {
	post += " " + what;
}

void Intermediate::writePre(UTF8String const& what) {
	root->impl->pre += " " + what;
}

void Intermediate::writeMain(UTF8String const& what) {
	root->impl->main += " " + what;
}

void Intermediate::writePost(UTF8String const& what) {
	root->impl->post += " " + what;
}

Function::OverloadRef Function::overloadFromVariables(List<Namespace::VariableRef> const& args) const {
	return overloadFromTypes(args.toList<Namespace::TypeRef>([] (auto const& e) {return e->type;}));
}

Function::OverloadRef Function::overloadFromTypes(List<Namespace::TypeRef> const& args) const {
	for (auto& ov: overloads) {
		if (!ov) continue;
		if (ov->arguments.size() != args.size()) continue;
		bool miss = false;
		for (auto const arg: Range::expand(ov->arguments))
			if (!(args[arg.index].exists() || arg.value.exists()))
				continue;
			else if (args[arg.index].exists() != arg.value.exists()) {
				miss = true;
				break;
			} else if (args[arg.index] != arg.value->type) {
				miss = true;
				break;
			}
		if (miss) continue;
		return ov;
	}
	return nullptr;
}

Implementation::Instance Namespace::compose() const {
	Implementation::Instance out = out.create();
	if (function) {
		for (auto& ov: function->overloads)
			out->writePreLine(ov->prototype());
		return out;
	}
	if (impl) {
		if (!variable)
			out->writePreLine("begin", varc);
		if (isPureNamespace())
			out->writePostLine("keep");
		if (!variable) out->writePostLine("end");
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
		attrib->target		= fromString(v.fetch<Makai::UTF8String>("target", "func"));
		attrib->globalMin	= v.fetch<uint64>("min", 0);
		attrib->globalMax	= v.fetch<uint64>("max", Makai::Limit::MAX<uint64>);
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
			attrib->fields[name] = {kind, var->value};
			attrib->transform = ATTRIBUTE_TRANSFORMER() {
			};
		}
		ns->attribute = attrib;
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
		if (globalTypes.contains(srcName) && globalTypes[srcName] != ns->variable->type)
			Transformer::ATransformer::Context::error("Global variable type mismatch!", ns->node);
		ns->variable->source = "move $" + srcName;
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
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		static usize id = 0;
		auto const name = (v["name"].getString());
		auto const lib = (v["lib"].getString());
		for (auto& ov: ns->function->overloads)
			if (ov->entry.empty() && ov->variant == Function::Overload::Variant::AV2_TCB_FOV_NONE) {
				ov->variant = Function::Overload::Variant::AV2_TCB_FOV_DYNLIB;
				ov->outEntry = name;
				ov->dynlib = lib;
				ov->optional = v["optional"];
				ov->entry = "__shared_dynlib_" + Makai::toString(id) + ov->methodOf->node->name();
			}
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
		static Makai::Random::SecureGenerator rng;
		if (ns->variable) {
			if (ns->variable->global)
				Transformer::ATransformer::Context::error("Variable cannot be both Global and Static!", ns->node);
			ns->variable->global = true;
			ns->variable->staticEntity = true;
			ns->variable->source = "move $__STATIC__._ns_" + Makai::toString(rng.integer()) + "._ns_" + ns->node->name() + "._" + ns->variable->name;
		} else if (ns->function) {
			for (auto& ov: ns->function->overloads)
				if (ov->variant == Function::Overload::Variant::AV2_TCB_FOV_NONE)
					ov->variant = Function::Overload::Variant::AV2_TCB_FOV_GLOBAL;
		}
	};
	return attrib;
}

static Namespace::AttributeRef createMemberAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Member";
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->fields["type"] = {.type = DVK_STRING, .path = true};
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		auto const bns = inter.resolve(Makai::UTF8String(v["type"].getString()).split(Makai::UTF8Char{'/'}));
		if (!(bns && bns->type))
			Transformer::ATransformer::Context::error("Symbol must be a type!", ns->node);
		auto const bt = bns->type;
		if (bns->subspaces.contains(ns->name))
			Transformer::ATransformer::Context::error("Symbol has already been declared in the type!", ns->node);
		for (auto& ov: ns->function->overloads)
			if (ov->variant == Function::Overload::Variant::AV2_TCB_FOV_NONE) {
				if (!(ov->arguments.size() >= 1 && ov->arguments[0]->type == bt))
					Transformer::ATransformer::Context::error("Missing appropriate [self] parameter for member function!", ns->node);
				ov->variant = Function::Overload::Variant::AV2_TCB_FOV_CLASS;
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
		ns->type->flags |= Core::Definition::Flags::AV2_DF_NULLABLE;
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
		ns->type->flags |= Core::Definition::Flags::AV2_DF_EMPTY;
		if (ns->type->fields.size())
			Transformer::ATransformer::Context::error("Empty type must not contain fields!", ns->node);
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
		ns->type->flags |= Core::Definition::Flags::AV2_DF_DYNAMIC;
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
		ns->type->flags |= Core::Definition::Flags::AV2_DF_CLONABLE;
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
		ns->type->flags |= Core::Definition::Flags::AV2_DF_FINAL;
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
		ns->type->flags |= Core::Definition::Flags::AV2_DF_VALUE;
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
		ns->type->def = TypeDecl::Definition::AV2_TCTD_BASIC;
		ns->type->flags &= ~Core::Definition::Flags::AV2_DF_STRUCTURE;
		ns->type->flags |= Core::Definition::Flags::AV2_DF_BASIC;
		if (ns->type->fields.size())
			Transformer::ATransformer::Context::error("Basic type must not contain fields!", ns->node);
		auto const bt = (v["type"].getString());
		if (bt == "void")			ns->type->basic = AV2_BT_VOID;
		else if (bt == "bool")		ns->type->basic = AV2_BT_BOOL;
		else if (bt == "int8")		ns->type->basic = AV2_BT_INT8;
		else if (bt == "uint8")		ns->type->basic = AV2_BT_UINT8;
		else if (bt == "int16")		ns->type->basic = AV2_BT_INT16;
		else if (bt == "uint16")	ns->type->basic = AV2_BT_UINT16;
		else if (bt == "int32")		ns->type->basic = AV2_BT_INT32;
		else if (bt == "uint32")	ns->type->basic = AV2_BT_UINT32;
		else if (bt == "int64")		ns->type->basic = AV2_BT_INT64;
		else if (bt == "uint64")	ns->type->basic = AV2_BT_UINT64;
		else if (bt == "float32")	ns->type->basic = AV2_BT_REAL32;
		else if (bt == "float64")	ns->type->basic = AV2_BT_REAL64;
		else if (bt == "float128")	ns->type->basic = AV2_BT_REAL128;
		else if (bt == "vector")	ns->type->basic = AV2_BT_VECTOR;
		else if (bt == "bytes")		ns->type->basic = AV2_BT_BYTES;
		else if (bt == "matrix")	ns->type->basic = AV2_BT_MATRIX;
		else if (bt == "type")		ns->type->basic = AV2_BT_TYPEID;
		else if (bt == "string")	ns->type->basic = AV2_BT_STRING;
		else if (bt == "char")		ns->type->basic = AV2_BT_CHAR;
		else if (bt == "any")		ns->type->basic = AV2_BT_ANY;
		else if (bt == "null")		ns->type->basic = AV2_BT_NULL;
		else Transformer::ATransformer::Context::error("Invalid basic type!", ns->node);
	};
	return attrib;
}

static Namespace::AttributeRef createBoundAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Bound";
	attrib->target = Attribute::Target::AV2_TAAT_TYPE;
	attrib->fields["artType"] = {DVK_STRING};
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		ns->type->flags |= Core::Definition::Flags::AV2_DF_ART_EQUIVALENT;
		auto const artt = (v["artType"].getString());
		ns->type->artEquivalent = artt;
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
		if (inter.entry)
			Transformer::ATransformer::Context::error("Redeclaration of previously-declared entry!", ns->node);
		inter.entry = ns->function->overloadFromTypes({}).asWeak();
		if (!inter.entry)
			Transformer::ATransformer::Context::error("No valid function overload for entry!", ns->node);
	};
	return attrib;
}

static Namespace::AttributeRef createEntryAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Entry";
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->globalMin = 0;
	attrib->globalMax = 1;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		if (inter.entry)
			Transformer::ATransformer::Context::error("Redeclaration of previously-declared entry!", ns->node);
		inter.entry = ns->function->overloadFromTypes({}).asWeak();
		if (!inter.entry)
			Transformer::ATransformer::Context::error("No valid function overload for entry!", ns->node);
	};
	return attrib;
}

static Namespace::AttributeRef createExitAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Exit";
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->globalMin = 0;
	attrib->globalMax = 1;
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		if (inter.exit)
			Transformer::ATransformer::Context::error("Redeclaration of previously-declared exit!", ns->node);
		inter.exit = ns->function->overloadFromTypes({}).asWeak();
		if (!inter.exit)
			Transformer::ATransformer::Context::error("No valid function overload for exit!", ns->node);
	};
	return attrib;
}

static Namespace::AttributeRef createARTCallAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "ARTCall";
	attrib->target = Attribute::Target::AV2_TAAT_FUNCTION;
	attrib->fields["name"] = {DVK_STRING};
	attrib->fields["optional"]	= {.type=DVK_STRING, .defaultValue=false, .path=true};
	attrib->transform = ATTRIBUTE_TRANSFORMER() {
		static usize id = 0;
		auto const name = (v["name"].getString());
		for (auto& ov: ns->function->overloads)
			if (ov->entry.empty() && ov->variant == Function::Overload::Variant::AV2_TCB_FOV_NONE) {
				ov->variant = Function::Overload::Variant::AV2_TCB_FOV_ART_CALL;
				ov->outEntry = name;
				ov->optional = v["optional"];
				ov->entry = "__art_call_" + Makai::toString(id) + ov->methodOf->node->name();
			}
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
	addGlobalAttribute(createOperatorAttribute());
	addGlobalAttribute(createNullableAttribute());
	addGlobalAttribute(createEmptyAttribute());
	addGlobalAttribute(createDynamicAttribute());
	addGlobalAttribute(createCopyAttribute());
	addGlobalAttribute(createFinalAttribute());
	addGlobalAttribute(createBasicAttribute());
	addGlobalAttribute(createValueAttribute());
	addGlobalAttribute(createBoundAttribute());
	addGlobalAttribute(createGlobalAttribute());
	addGlobalAttribute(createStaticAttribute());
	addGlobalAttribute(createMainAttribute());
	addGlobalAttribute(createGetterAttribute());
	addGlobalAttribute(createSetterAttribute());
	addGlobalAttribute(createConverterAttribute());
	addGlobalAttribute(createMemberAttribute());
	addGlobalAttribute(createSharedAttribute());
	addGlobalAttribute(createARTCallAttribute());
	addGlobalAttribute(createEntryAttribute());
	addGlobalAttribute(createExitAttribute());
}

Makai::Data::Value Implementation::serialize() const {
	Makai::Data::Value out = out.object();
	out["pre"]	= pre.toString();
	out["main"]	= main.toString();
	out["post"]	= post.toString();
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
	switch (variant) {
		using enum Variant;
		case Variant::AV2_TCB_FOV_NONE:
		case Variant::AV2_TCB_FOV_GLOBAL:	out["variant"] = "global";		break;
		case Variant::AV2_TCB_FOV_CLASS:	out["variant"] = "class";		break;
		case Variant::AV2_TCB_FOV_INSTANCE:	out["variant"] = "instance";	break;
		case Variant::AV2_TCB_FOV_ART_CALL:	out["variant"] = "artcall";		break;
		case Variant::AV2_TCB_FOV_DYNLIB:	out["variant"] = "dynlib";		break;
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
		case Definition::AV2_TCTD_STRUCT: out["def"] = "basic"; break;
		case Definition::AV2_TCTD_TEMPLATE: out["def"] = "template"; break;
		case Definition::AV2_TCTD_BASIC: out["def"] = "basic"; break;
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

Makai::UTF8String Function::Overload::prototype() const {
	Makai::UTF8String out = "_Rt0" + (result ? result->name : "!ERR!");
	for (auto const& [arg, index]: Makai::Range::expand(arguments))
		out += "_At" + Makai::toString(index) + (arg->type ? arg->type->name : "!ERR!");
	return out;
}

Function::Overload::Overload() {}
Function::Overload::~Overload() {}
