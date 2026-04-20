#include "module.hpp"
#include "../../../../tool/archive/archive.hpp"

using namespace Makai;
using namespace Makai::Anima::V2::Core;

static void deserializeV1(Module& mod, Makai::Data::Value const& v) {
	mod.strings =
		v.fetch(
			"strings",
			Makai::Data::Value::ArrayType()
		).toList<String>(
			[] (auto& e) {
				return e.getString();
			}
		)
	;
	auto const code		= Makai::Tool::Arch::decompress(v["code"].get<Makai::Data::Value::ByteListType>());
	auto const jumps	= Makai::Tool::Arch::decompress(v["jumps"].get<Makai::Data::Value::ByteListType>());
	mod.code		= decltype(mod.code){ref<Instruction>(code.data()), ref<Instruction>(code.data()) + (code.size() / sizeof(Instruction))};
	mod.jumpTable	= decltype(mod.jumpTable){ref<uint64>(jumps.data()), ref<uint64>(jumps.data()) + (jumps.size() / sizeof(uint64))};
	mod.sym = Module::Symbols::deserialize(v["sym"]);
	mod.detail = Module::Detail::deserialize(v["detail"]);
	if (v.contains("ani"))
		*mod.ani = Module::ANI::deserialize(v["ani"]);
	else mod.ani.unbind();
	mod.type = v["type"].get<Module::Type>(Module::Type::AV2_CMT_LIBRARY);
}

Module Module::deserialize(Makai::Data::Value const& v) {
	Module mod;
	if (v.contains("art"))
		mod.art		= v["art"];
	else mod.art	= Module::Version{1};
	if (v.contains("version"))
		mod.version	= v["version"];
	else mod.art = ART_VER;
	switch (mod.art.major) {
		case 2: deserializeV1(mod, v); break;
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
	result["shared"]	= result.object();
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
			result.types.pushBack(sym);
	if (v.contains("methods"))
		for (auto& sym: v["methods"].getArray())
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
	result.id = v["id"];
	if (v.contains("name"))
		result.name = v["name"].getString();
	result.retType = v["return"];
	result.argTypes = v["args"].getArray().toList<uint64>();
	result.out = v["out"];
	result.shared = v["shared"];
	result.entrypoint = v["entry"];
	result.size = v["size"];
	result.meta = v["meta"];
	return result;
}

Makai::Data::Value Module::Declaration::serialize() const {
	auto result = Data::Value::object();
	result["id"] = id;
	if (name.size())
		result["name"] = name.toString();
	result["flags"] = flags;
	if (basic)
		result["basic"] = *basic;
	if (base)
		result["base"] = *base;
	result["bytes"] = byteSize;
	result["align"] = alignment;
	result["fields"] = fields.toList<Data::Value>();
	result["meta"] = meta;
	return result;
}


Module::Declaration Module::Declaration::deserialize(Data::Value const& v) {
	Module::Declaration result;
	result.id = v["id"];
	if (v.contains("name"))
		result.name = v["name"].getString();
	result.flags = v["flags"].getUnsigned();
	if (v.contains("basic"))
		result.basic = Cast::as<BasicType>(v["basic"].getUnsigned());
	if (v.contains("base"))
		result.base = v["base"].getUnsigned();
	result.byteSize = v["bytes"];
	result.alignment = v["align"];
	result.meta = v["meta"];
	return result;
}
