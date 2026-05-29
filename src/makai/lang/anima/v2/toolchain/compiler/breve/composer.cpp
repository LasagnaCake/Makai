#include "composer.hpp"
#include "intermediate.hpp"
#include "transformer.hpp"

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;

static void doFunction(Composer& composer, Namespace::FunctionRef const& fn) {
	if (composer.visitedFunctions.contains(fn)) return;
	composer.visitedFunctions[fn] = true;
	for (auto& ov: fn->overloads) {
		Makai::UTF8String ovstr;
		DEBUGLN("Name: ", ov->entry);
		DEBUGLN("Variant: ", ov->serialize()["variant"].getString());
		if (ov->dynlib.size())
			ovstr += "@shared[\"" + ov->dynlib + "\" : \"" + ov->outEntry + "\"] ";
		else if (ov->outEntry.size())
			ovstr += "@out[\"" + ov->outEntry + "\"] ";
		else ovstr += "@fn ";
		if (ov->outEntry.size() or ov->dynlib.size())
			ovstr += ov->optional ? "optional " : "required ";
		composer.functions.pushBack(
			ovstr
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
		if (ov->scope)
			composer.funcDefs.pushBack(ov->scope->impl);
	}
}

static void doVariable(Composer& composer, Namespace::VariableRef const& var) {
	if (!var->initializer) return;
	if (!var->staticEntity && !composer.visited.contains(var->initializer)) {
		composer.top()->writeMainLine(var->initializer->impl->toString());
		composer.visited[var->initializer] = true;
		var->initializer->impl = null;
	} else if (var->staticEntity)
		composer.staticDefs.pushBack(var->initializer->impl);
}

static void doType(Composer& composer, Namespace::TypeRef const& type) {
	if (composer.visitedTypes.contains(type)) return;
	composer.visitedTypes[type] = true;
	Makai::UTF8String decl;
	decl += "@type " + type->name + " [\n ";
	DEBUGLN("Type Name: '", type->name, "'");
	if (type->flags & Core::Definition::Flags::AV2_DF_BASIC) {
		decl += " basic<";
		if (!type->basic)
			Transformer::ATransformer::Context::error("Missing basic type analog!");
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
		else decl += " derived<" + type->base->name + ">";
	}
	if (type->scope) {
		if (type->scope->meta.values().filter([] (auto const& e) {return !e->value.isUndefined();}).size()) {
			decl += "\n  meta [\n";
			for (auto& [name, attrib]: type->scope->meta)
				if (!attrib->value.isUndefined())
					decl += "    " + name + " `" + attrib->value.toFLOWString("      ") + "`\n";
				else
					decl += "    " + name + " `{}`\n";
			decl += "  ]";
		}
	}
	auto fields = copy(type->fields);
	fields["this"] = nullptr;
	fields["base"] = nullptr;
	if (fields.size()) {
		usize count = 0;
		Makai::UTF8String buf;
		for (auto& [name, field]: fields) {
			if (field) {
				buf += "    " + (field->type->name) + "\n";
				++count;
			}
		}
		if (count) {
			decl += "\n  fields [\n" + buf + "  ]";
		}
	}
	decl += "\n]\n";
	composer.types.pushBack(decl);
}

static void doNamespace(Composer& composer, Namespace::Instance const& ns) {
	if (!ns) return;
	composer.push();
	for (auto& [name, sub]: ns->subspaces) {
		if (composer.visited.contains(sub) && composer.visited[sub]) continue;
		if (!sub) continue;
		if (sub->function) doFunction(composer, sub->function);
		if (sub->variable) {
			if (ns->declaredAsNamespace && !sub->variable->global) {
				sub->variable->id = composer.staticVarCount;
				++composer.staticVarCount;
			} else doVariable(composer, sub->variable);
		}
		if (sub->type) doType(composer, sub->type);
	}
	if(composer.visited.contains(ns) && composer.visited[ns])
		return composer.pop();
	composer.visited[ns] = true;
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
		UTF8String out = "@target __initializer__:\n";
		for (auto& sd: staticDefs)
			if (sd)
				cache += sd->toString() + "\n";
		for (auto& before: inter.before)
			if (before) cache += "call " + before->entry + "\n";
		for (auto& init: preMain)
			out += init->impl->toString();
		if (inter.main) out += "call " + inter.main->main + "\n";
		for (auto& after: inter.after)
			if (after) cache += "call " + after->entry + "\n";
		out += "stop\n";
		return out;
	} ();
	cache += "@entry __initializer__\n";
	for (auto& fd: funcDefs) if (fd)
		cache += fd->toString() + "\n";
	return cache;
}
