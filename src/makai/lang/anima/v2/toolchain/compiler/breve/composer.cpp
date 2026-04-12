#include "composer.hpp"
#include "intermediate.hpp"
#include "transformer.hpp"

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;

static void doFunction(Composer& composer, Namespace::FunctionRef const& fn) {
	for (auto& ov: fn->overloads) {
		composer.functions.pushBack(
			"@fn "
		+	ov->result->name
		+	" ("
		+	ov->arguments
			.toList<Makai::UTF8String>(
			[] (auto const& e) {
				return e->type->name;
			}
		).join(" ")
		+	") "
		+ ov->entry + "\n"
		);
	}
}

static void doVariable(Composer& composer, Namespace::FunctionRef const& fn) {
	// TODO: Variable

}

static void doType(Composer& composer, Namespace::TypeRef const& type) {
	Makai::UTF8String decl;
	decl += "@type " + type->name + " [\n ";
	if (type->flags & Core::Definition::Flags::AV2_DF_BASIC) {
		decl += " basic<";
		switch (*type->basic) {
			case Core::BasicType::AV2_BT_ANY:		decl += "any";	break;
			case Core::BasicType::AV2_BT_INT8:		decl += "i8";	break;
			case Core::BasicType::AV2_BT_INT16:		decl += "i16";	break;
			case Core::BasicType::AV2_BT_INT32:		decl += "i32";	break;
			case Core::BasicType::AV2_BT_INT64:		decl += "i64";	break;
			case Core::BasicType::AV2_BT_UINT8:		decl += "u8";	break;
			case Core::BasicType::AV2_BT_UINT16:	decl += "u16";	break;
			case Core::BasicType::AV2_BT_UINT32:	decl += "u32";	break;
			case Core::BasicType::AV2_BT_UINT64:	decl += "u64";	break;
			case Core::BasicType::AV2_BT_REAL32:	decl += "f32";	break;
			case Core::BasicType::AV2_BT_REAL64:	decl += "f64";	break;
			case Core::BasicType::AV2_BT_REAL128:	decl += "f128";	break;
			case Core::BasicType::AV2_BT_STRING:	decl += "str";	break;
			case Core::BasicType::AV2_BT_VECTOR:	decl += "vec";	break;
			case Core::BasicType::AV2_BT_MATRIX:	decl += "mat";	break;
			case Core::BasicType::AV2_BT_TYPEID:	decl += "type";	break;
			case Core::BasicType::AV2_BT_BYTES:		decl += "bin";	break;
			case Core::BasicType::AV2_BT_NULL:		decl += "nil";	break;
			case Core::BasicType::AV2_BT_VOID:		decl += "void";	break;
			case Core::BasicType::AV2_BT_BOOL:		decl += "bool";	break;
			case Core::BasicType::AV2_BT_CHAR:		decl += "char";	break;
			default: break;
		}
		decl += ">";
	}
	if (type->flags & Core::Definition::Flags::AV2_DF_NULLABLE)
		decl += " nil";
	if (type->flags & Core::Definition::Flags::AV2_DF_VALUE)
		decl += " value";
	if (type->flags & Core::Definition::Flags::AV2_DF_EMPTY)
		decl += " empty";
	if (type->flags & Core::Definition::Flags::AV2_DF_NO_RESULT)
		decl += " discard";
	if (type->flags & Core::Definition::Flags::AV2_DF_DYNAMIC)
		decl += " dyn";
	if (type->flags & Core::Definition::Flags::AV2_DF_STRUCTURE)
		decl += " struct";
	if (type->flags & Core::Definition::Flags::AV2_DF_CLONABLE)
		decl += " copy";
	if (type->flags & Core::Definition::Flags::AV2_DF_ART_EQUIVALENT)
		decl += " bound";
	if (type->flags & Core::Definition::Flags::AV2_DF_FINAL)
		decl += " final";
	if (type->base) {
		if (type->flags & Core::Definition::Flags::AV2_DF_ARRAY)
			decl += " array<" + type->base->name + ">";
		else decl += " base<" + type->base->name + ">";
	}
	if (type->scope) {
		decl += "\n  meta [\n";
		for (auto& [name, attrib]: type->scope->meta)
			if (!attrib->value.isUndefined())
				decl += "    " + name + " ´" + attrib->value.toFLOWString() + "´\n";
		decl += "  ]";
	}
	decl += "\n]\n";
	composer.types.pushBack(decl);
}

static void doNamespace(Composer& composer, Namespace::Instance const& ns) {
	composer.push();
	for (auto& [name, sub]: ns->subspaces) {
		if (composer.visited.contains(sub)) continue;
		if (!sub) continue;
	 	composer.visited[sub] = true;
		if (sub->function) doFunction(composer, sub->function);
		if (sub->variable) {
			if (ns->declaredAsNamespace && !sub->variable->global) {
				sub->variable->id = composer.staticVarCount;
				++composer.staticVarCount;
			}
		}
		if (sub->type) doType(composer, sub->type);
	}
	if (ns->isPureNamespace() && !ns->declaredAsNamespace && ns->impl->main.size()) {
		composer.top()->writePreLine("begin", ns->varc);
		composer.top()->writePreLine("keep");
		composer.top()->writeMainLine(ns->impl->toString());
		composer.top()->writePostLine("end");
	}
	for (auto& [name, sub]: ns->subspaces)
		doNamespace(composer, sub);
	composer.pop();
}

Makai::UTF8String Composer::toMinima() {
	if (cache.size()) return cache;
	doNamespace(*this, inter.root);
	cache = types.join("\n") + functions.join("\n") + impl->toString();
	if (mustHaveMain && !inter.entry)
			Transformer::ATransformer::Context::error("Missing required entrypoint!");
	cache += [this] () -> UTF8String {
		UTF8String out = "__initializer__:\n";
		for (auto& init: preMain)
			out += init->impl->toString();
		if (inter.entry)	out += "call " + inter.entry->entry + "\n";
		out += "stop\n";
		return out;
	} ();
	cache += "@entry __initializer__\n";
	if (inter.exit)	cache += "@exit " + inter.exit->entry + "\n";
	cache = Regex::replace(cache, R"(\n\s+)", "\n");
	cache = Regex::replace(cache, R"(^\s+)", "");
	return cache;
}
