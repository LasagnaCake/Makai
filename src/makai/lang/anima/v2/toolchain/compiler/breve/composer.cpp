#include "composer.hpp"
#include "intermediate.hpp"
#include "transformer.hpp"

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;

static void doType(Composer& composer, Namespace::TypeRef const& fn);

static void doFunction(Composer& composer, Namespace::FunctionRef const& fn) {
	if (composer.visitedFunctions.contains(fn)) return;
	composer.visitedFunctions[fn] = true;
	for (auto& ov: fn->overloads) {
		Makai::UTF8String ovstr;
		if (ov->variant.context == ExecutionContext::AV2_TCB_EC_COMPILE)
			continue;
		if (!ov->fullImpl->uses) continue;
		MAKAILIB_DEBUGLN_FULL("Name: ", ov->entry);
		MAKAILIB_DEBUGLN_FULL("Variant: ", ov->serialize()["variant"].getString());
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
			[&] (auto const& e) {
				doType(composer, e->type.raw());
				return e->type->name;
			}
		).join(" ")
		+	") "
		+ ov->entry + "\n"
		);
		if (ov->variant.external == Function::Overload::Variant::External::AV2_TCB_FO_VE_NONE)
			if (ov->scope && ov->scope->impl)
				composer.funcDefs.pushBack(ov->scope->impl->compose());
	}
	if (fn->sigCall)
		composer.functions.pushBack("@hook[\"" + fn->sigCall->sigEntry + "\"] " + fn->sigCall->entry);
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
	MAKAILIB_DEBUGLN_FULL("Type Name: '", type->name, "' = ", type->uses);
	if (!type->flags.isBasic && !type->uses) return;
	Makai::UTF8String decl;
	decl += "@type " + type->name + " [\n ";
	if (type->flags.isBasic) {
		decl += " basic<";
		if (!type->basic)
			Transformer::ATransformer::Context::error("Missing basic type analog!");
		decl += Core::asNameString(*type->basic);
		decl += ">";
	}
	if (type->flags.isNullable)
		decl += " nil";
	if (type->flags.isValueType)
		decl += " value";
	if (type->flags.isEmpty)
		decl += " empty";
	if (type->flags.hasNoResult)
		decl += " discard";
	if (type->flags.isDynamic)
		decl += " dyn";
	if (type->flags.isStructure)
		decl += " struct";
	if (type->flags.isCopyable)
		decl += " copy";
	if (type->flags.isProxy)
		decl += " proxy";
	if (type->flags.isFinal)
		decl += " final";
	if (type->base) {
		doType(composer, type->base);
		if (type->flags.isArray)
			decl += " array<" + type->base->name + ">";
		else if (type->flags.isEnum)
			decl += " enum<" + type->base->name + ">";
		else if (type->flags.isNullable)
			decl += " nil<" + type->base->name + ">";
		else decl += " derived<" + type->base->name + ">";
	}
	if (type->scope) {
		auto const vals = type->scope->meta.values();
		bool hasMetaInfo = false;
		for (auto const& [name, val]: type->scope->meta)
			if (val && !val->value.isUndefined()) {
				hasMetaInfo = true;
				break;
			}
		if (hasMetaInfo) {
			decl += "\n  meta [\n";
			for (auto& [name, attrib]: type->scope->meta)
				if (!attrib->value.isUndefined())
					decl += "    " + name + " `" + attrib->value.toFLOWString() + "`\n";
				else
					decl += "    " + name + "!\n";
			decl += "  ]";
		}
	}
	auto fields = copy(type->fields);
	if (fields.size() && !type->flags.isEnum) {
		usize count = 0;
		Makai::UTF8String buf;
		for (auto& [name, field]: fields) {
			if (field) {
				doType(composer, field->type.raw());
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
	if (mustHaveMain && !inter.main)
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
		if (inter.main) out += "call " + inter.main->entry + "\n";
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
