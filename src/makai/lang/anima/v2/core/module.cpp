#include "module.hpp"

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
	auto const code		= v["code"].get<Makai::Data::Value::ByteListType>();
	auto const jumps	= v["jumps"].get<Makai::Data::Value::ByteListType>();
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
	if (v.contains("version"))
		mod.art = v["version"];
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
	out["jumps"]	= jumpTable.toBytes();
	out["code"]		= code.toBytes();
	out["version"]	= art;
	if (type == Module::Type::AV2_CMT_LIBRARY || forceSymbolsToBeKept) {
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
