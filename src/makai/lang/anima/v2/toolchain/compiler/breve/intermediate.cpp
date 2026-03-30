#include "intermediate.hpp"
#include "transformer.hpp"

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;

IWritable::~IWritable() {}

Namespace::Namespace(UTF8String const& name): Labeled(name) {}

Namespace::Instance Namespace::resolve(UTF8StringList const& path) const {
	if (path.empty()) return nullptr;
	if (!subspaces.contains(path.back())) return nullptr;
	if (path.size() == 1)
		return subspaces[path.back()];
	else return subspaces[path.back()]->resolve(path.sliced(1));
	return nullptr;
}

Namespace::Instance Intermediate::resolve(UTF8StringList const& path) const {
	if (path.empty()) return nullptr;
	for (auto& scope: Makai::Range::reverse(scopeStack)) {
		if (path.size() == 1 && scope->name == path.back())
			return scope;
		else if (auto ns = scope->resolve(path))
			return ns;
	}
	if (auto ns = root->resolve(path))
		return ns;
	return nullptr;
}

usize Intermediate::push(UTF8StringList const& path) {
	if (path.empty()) return 0;
	if (scopeStack.empty() && root->subspaces.contains(path.back()))
		return root->subspaces[path.back()];
	else if (scopeStack.back()->subspaces.contains(path.back()))
		return scopeStack.back()->subspaces[path.back()];
	Namespace::Instance ns = ns.create(path.back());
	if (scopeStack.empty())
		root->subspaces[ns->name] = ns;
	else scopeStack.back()->subspaces[ns->name] = ns;
	scopeStack.pushBack(ns);
	return push(path.sliced(1)) + 1;
}

void Intermediate::pop(usize count) {
	while (scopeStack.size() && count--) {
		auto const scope = scopeStack.popBack();
		if (!scope->impl) continue;
		/*
		if (scopeStack.empty())
			root->impl->pre += scope->impl->compose()->toString();
		else scopeStack.back()->impl->main += scope->impl->compose()->toString();
		 */
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

Function::OverloadRef Function::overload(List<Namespace::VariableRef> const& args) const {
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
			} else if (args[arg.index]->type != arg.value->type) {
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
	if (name == "struct")	return Attribute::Target::AV2_TAAT_STRUCT;
	if (name == "fn")		return Attribute::Target::AV2_TAAT_FUNCTION;
	if (name == "prop")		return Attribute::Target::AV2_TAAT_PROPERTY;
	if (name == "value")	return Attribute::Target::AV2_TAAT_VALUE;
	if (name == "var")		return Attribute::Target::AV2_TAAT_VARIABLE;
	if (name == "attrib")	return Attribute::Target::AV2_TAAT_ATTRIBUTE;
	return Attribute::Target::AV2_TAAT_EMPTY;
}

static Namespace::AttributeRef createMetaAttribute() {
	using enum Makai::Data::Value::Kind;
	using enum Core::BasicType;
	Namespace::AttributeRef attrib = attrib.create();
	attrib->name = "Attribute";
	attrib->fields["target"]	= {DVK_STRING								};
	attrib->fields["globalMax"]	= {DVK_UNSIGNED, Makai::Limit::MAX<uint64>	};
	attrib->fields["localMax"]	= {DVK_UNSIGNED, Makai::Limit::MAX<uint64>	};
	attrib->transform = [] (Namespace::Instance const& ns, Makai::Data::Value const& v, Attribute& base) {
		if (!(ns->type && ns->type->def == TypeDecl::Definition::AV2_TCTD_STRUCT))
			Transformer::ATransformer::Context::error("Expected structure here!", ns->node);
		auto const attrib = Namespace::AttributeRef::create();
		attrib->target		= fromString(v.fetch<Makai::UTF8String>("target", "fn"));
		attrib->localMax	= v.fetch<uint64>("localMax", Makai::Limit::MAX<uint64>);
		attrib->globalMax	= v.fetch<uint64>("globalMax", Makai::Limit::MAX<uint64>);
		for (auto const& [name, field]: ns->subspaces) {
			if (!field->variable)
				Transformer::ATransformer::Context::error("Expected variable declaration here!", field->node);
			auto const& var = field->variable;
			if (!var->type->basic)
				Transformer::ATransformer::Context::error("Variable type must be a basic type!", var->node);
			if (var->defaulted && !var->value)
				Transformer::ATransformer::Context::error("Attribute field defaults must have constant values!", var->node);
			if (attrib->fields.contains(name))
				Transformer::ATransformer::Context::error("Redeclaration of previously-declared field!", var->node);
			Makai::Data::Value::Kind kind;
			switch (*var->type->basic) {
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
		}
		ns->attribute = attrib;
	};
	return attrib;
}

Intermediate::Intermediate() {
	auto const attrib = Namespace::Instance::create();
	attrib->attribute = createMetaAttribute();
	root->subspaces["Attribute"] = attrib;
}
