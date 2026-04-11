#include "composer.hpp"
#include "intermediate.hpp"
#include "transformer.hpp"

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;

static void doFunction(Composer& composer, Namespace::FunctionRef const& fn) {
	for (auto& ov: fn->overloads) {
		composer.functions.pushBack(
			"@fn "
		+	ov->entry
		+	" ("
		+	ov->arguments
			.toList<Makai::UTF8String>(
			[] (auto const& e) {
				return e->type->name;
			}
		).join(" ")
		+	")"
		);
		composer.impl.writeMainLine("@def", ov->scope->compose()->toString(), "\n");
		composer.impl.writeMainLine("@def .\n");
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
	decl += "\n  meta [\n";
	for (auto& [name, attrib]: type->scope->meta)
		if (!attrib->value.isUndefined())
			decl += "    " + name + " ´" + attrib->value.toFLOWString() + "´\n";
	decl += "  ]\n]";
	composer.types.pushBack(decl);
}

static void doNamespace(Composer& composer, Namespace::Instance const& ns) {
	Implementation curImpl;
	for (auto& [name, sub]: ns->subspaces) {
		if (composer.visited[ns]) continue;
	 	composer.visited[ns] = true;
		if (sub->isPureNamespace()) doNamespace(composer, ns);
		if (sub->function) doFunction(composer, ns->function);
		if (sub->variable) {
			if (ns->declaredAsNamespace && !sub->variable->global) {
				sub->variable->id = composer.staticVarCount;
				++composer.staticVarCount;
			}
		}
		if (sub->type) doType(composer, sub->type);
	}
	if (ns->isPureNamespace() && !ns->declaredAsNamespace) {
		curImpl.writePreLine("begin", ns->varc);
		curImpl.writePreLine("bring", ns->varc, "[0 : 0]");
		curImpl.writeMainLine(ns->impl->toString());
		curImpl.writePostLine("end");
	}
	composer.impl.writeMainLine(curImpl.toString());
}

Makai::UTF8String Composer::toMinima() const {
	auto self = copy(*this);
	doNamespace(self, inter.root);
	UTF8String result = self.types.join("\n") + self.functions.join("\n") + self.impl.toString();
	if (mustHaveMain && !self.inter.entry)
			Transformer::ATransformer::Context::error("Missing required entrypoint!");
	result += [&self] () -> UTF8String {
		UTF8String out = "__initializer__:\n";
		for (auto& init: self.preMain)
			out += init->impl->toString();
		if (self.inter.entry)	out += "call " + self.inter.entry->entry + "\n";
		out += "stop";
		return out;
	} ();
	result += "@entry __initializer__\n";
	if (self.inter.exit)	result += "@exit " + self.inter.exit->entry + "\n";
	return result;
}
