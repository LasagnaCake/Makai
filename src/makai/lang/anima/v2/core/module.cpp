#include "module.hpp"
#include "../../../../tool/archive/archive.hpp"

using namespace Makai;
using namespace Makai::Anima::V2::Core;

static bool valueExists(Makai::Data::Value const& v) {
	DEBUGLN("WHAT? ", !v.isUndefined());
	return !v.isUndefined();
}

static void deserializeV1(Module& mod, Makai::Data::Value const& v) {
	if (v.contains("strings"))
		mod.strings = v["strings"].getArray().filter(valueExists).toList<String>([](auto const& e){return e.isString() ? e.getString() : "";});
	auto const code		= Makai::Tool::Arch::decompress(v["code"].getBytes());
	auto const jumps	= Makai::Tool::Arch::decompress(v["jumps"].getBytes());
	DEBUGLN("Bytecode Section: ", code.size());
	DEBUGLN("Jump Table Section: ", jumps.size());
	mod.code		= decltype(mod.code){ref<Instruction>(code.data()), ref<Instruction>(code.data()) + (code.size() / sizeof(Instruction))};
	mod.jumpTable	= decltype(mod.jumpTable){ref<uint64>(jumps.data()), ref<uint64>(jumps.data()) + (jumps.size() / sizeof(uint64))};
	DEBUGLN("Instructions: ", mod.code.size());
	DEBUG("Jump Table Entries: ", mod.jumpTable.size(), " [ ");
	for (auto& jump : mod.jumpTable)
		DEBUG(jump, " ");
	DEBUGLN("]");
	if (mod.code.empty()) throw Error::FailedAction(
		"Failed to load file!",
		"Failed to load bytecode section",
		CTL_CPP_PRETTY_SOURCE
	);
	if (mod.jumpTable.empty()) throw Error::FailedAction(
		"Failed to load file!",
		"Failed to load jump table section",
		CTL_CPP_PRETTY_SOURCE
	);
	if (v.contains("sym"))
		mod.sym = Module::Symbols::deserialize(v["sym"]);
	if (v.contains("detail"))
		mod.detail = Module::Detail::deserialize(v["detail"]);
	if (v.contains("ani"))
		*mod.ani = Module::ANI::deserialize(v["ani"]);
	else mod.ani.unbind();
	mod.type = v["type"].get<Module::Type>(Module::Type::AV2_CMT_LIBRARY);
	if (v.contains("entry"))
		mod.entry = v["entry"].getUnsigned();
	if (v.contains("exit"))
		mod.exit = v["exit"].getUnsigned();
}

Module Module::deserialize(Makai::Data::Value const& v) {
	Module mod;
	if (v.contains("art"))
		mod.art		= v["art"];
	else mod.art	= ART_VER;
	if (v.contains("version"))
		mod.version		= v["version"];
	else mod.version	= Module::Version{0};
	switch (mod.art.major) {
		case 1: deserializeV1(mod, v); break;
		default: break;
	}
	return mod;
}

Makai::Data::Value Module::serialize(bool forceSymbolsToBeKept) const {
	Makai::Data::Value out;
	out["strings"]	= strings.toList<Makai::Data::Value>();
	DEBUGLN("Binary: ", code.size());
	out["jumps"]	= Makai::Tool::Arch::compress(jumpTable.toBytes());
	out["code"]		= Makai::Tool::Arch::compress(code.toBytes());
	out["art"]		= art;
	out["version"]	= version;
	out["detail"]	= detail;
	if (type == Module::Type::AV2_CMT_LIBRARY or forceSymbolsToBeKept) {
		out["detail"]	= detail;
		out["sym"]		= sym;
	} else {
		auto dt = copy(detail);
		for (auto& s : dt.methods)	s.name = "";
		for (auto& s : dt.types)	s.name = "";
		dt.methods.clear();
		out["detail"] = dt;
	}
	if (ani)
		out["ani"]	= *ani;
	out["type"]	= enumcast(type);
	if (entry + 1)	out["entry"]	= entry;
	if (exit + 1)	out["exit"]		= exit;
	return out;
}

Module::ANI Module::ANI::deserialize(Makai::Data::Value const& v) {
	ANI ani;
	if (v.contains("in")) {
		auto const signals	= v["in"];
		for (auto [label, id]: signals.items())
			ani.in[label]	= id;
	}
	if (v.contains("out")) {
		auto const externs	= v["out"];
		for (auto const& e: externs.get<Data::Value::ArrayType>())
			ani.out.pushBack(e);
	}
	return ani;
}

Makai::Data::Value Module::ANI::serialize() const {
	auto result = Data::Value::object();
	result["shared"]	= shared;
	result["in"]		= result.object();
	result["out"]		= result.array();
	auto& signals		= result["in"];
	auto& externs		= result["out"];
	for (auto& [name, id]: in)
		signals[name] = id;
	for (auto& name: out)
		externs[externs.size()] = name;
	return result;
}

Module::ANI::Shared Module::ANI::Shared::deserialize(Data::Value const& v) {
	Module::ANI::Shared out;
	out.modules		= v["modules"].getArray().toList<Makai::String>();
	out.interops	= v["interops"].getArray().toList<Makai::String>();
	out.libraries	= v["libraries"].getArray().toList<Makai::String>();
	return out;
}

Makai::Data::Value Module::ANI::Shared::serialize() const {
	auto result = Data::Value::object();
	result["modules"]	= modules.toList<Makai::Data::Value>();
	result["interops"]	= interops.toList<Makai::Data::Value>();
	result["libraries"]	= libraries.toList<Makai::Data::Value>();
	return result;
}

Module::Symbols Module::Symbols::deserialize(Makai::Data::Value const& v) {
	Module::Symbols result;
	if (v.contains("types"))
		for (auto& sym: v["types"].getArray())
			result.types.pushBack(sym);
	if (v.contains("methods"))
		for (auto& sym: v["methods"].getArray())
			result.methods.pushBack(sym);
	return result;
}

Makai::Data::Value Module::Symbols::serialize() const {
	auto result = Data::Value::object();
	result["types"]		= result.array();
	result["methods"]	= result.array();
	for (auto& type: types)
		result["types"][result["types"].size()] = type;
	for (auto& method: methods)
		result["methods"][result["methods"].size()] = method;
	return result;
}

Module::Detail Module::Detail::deserialize(Makai::Data::Value const& v) {
	Module::Detail result;
	if (v.contains("types"))
		for (auto& sym: v["types"].getArray())
			if (!sym.isUndefined())
				result.types.pushBack(sym);
	DEBUGLN("Total types: ", result.types.size());
	if (v.contains("methods"))
		for (auto& sym: v["methods"].getArray())
			if (!sym.isUndefined())
				result.methods.pushBack(sym);
	return result;
}

Makai::Data::Value Module::Detail::serialize() const {
	auto result = Data::Value::object();
	result["types"]		= result.array();
	result["methods"]	= result.array();
	for (auto& type: types)
		result["types"][result["types"].size()] = type.serialize();
	for (auto& method: methods)
		result["methods"][result["methods"].size()] = method.serialize();
	return result;
}

Makai::Data::Value Module::Method::serialize() const {
	auto result = Data::Value::object();
	result["id"] = id;
	if (name.size())
		result["name"] = name.toString();
	result["hash"] = hash;
	result["return"] = retType;
	result["args"] = argTypes.toList<Data::Value>();
	result["out"] = out;
	result["shared"] = shared;
	result["entry"] = entrypoint;
	result["size"] = size;
	result["meta"] = meta;
	return result;
}


Module::Method Module::Method::deserialize(Data::Value const& v) {
	Module::Method result;
	if (!v) return result;
	DEBUGLN ("Method ", v.toFLOWString("  "));
	result.id = v["id"];
	if (v.contains("name"))
		result.name = v["name"].getString();
	result.hash = v["hash"].getUnsigned();
	result.retType = v["return"];
	result.argTypes = v["args"].getArray().toList<uint64>().filter(valueExists);
	result.out = v["out"];
	result.shared = v["shared"];
	result.entrypoint = v["entry"];
	result.size = v["size"];
	if (v.contains("meta"))
		result.meta = v["meta"];
	return result;
}

Makai::Data::Value Module::Declaration::serialize() const {
	auto result = Data::Value::object();
	result["id"]	= id;
	result["flags"]	= flags;
	result["hash"]	= hash;
	if (name.size())
		result["name"] = name.toString();
	if (basic)
		result["basic"] = *basic;
	if (base)
		result["base"] = *base;
	if (byteSize)
		result["bytes"] = byteSize;
	if (alignment)
		result["align"] = alignment;
	if (fields.size())
		result["fields"] = fields.toList<Data::Value>().filter(valueExists);
	if (!meta.isUndefined())
		result["meta"] = meta;
	return result;
}


Module::Declaration Module::Declaration::deserialize(Data::Value const& v) {
	Module::Declaration result;
	if (!v) return result;
	DEBUGLN ("Type ", v.toFLOWString("  "));
	result.id		= v["id"].getUnsigned();
	result.flags	= v["flags"].getUnsigned();
	result.hash		= v["hash"].getUnsigned();
	if (v.contains("name"))
		result.name = v["name"].getString();
	if (v.contains("basic"))
		result.basic = Cast::as<BasicType>(v["basic"].getUnsigned());
	if (v.contains("base"))
		result.base = v["base"].getUnsigned();
	if (v.contains("bytes"))
		result.byteSize = v["bytes"];
	if (v.contains("align"))
		result.alignment = v["align"];
	if (v.contains("meta"))
		result.meta = v["meta"];
	if (v.contains("fields"))
		for (auto& field: v["fields"].getArray())
			if (!field.isUndefined())
				result.fields.pushBack(field.getUnsigned());
	return result;
}
