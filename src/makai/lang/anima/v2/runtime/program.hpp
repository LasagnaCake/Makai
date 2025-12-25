#ifndef MAKAILIB_ANIMA_V2_RUNTIME_PROGRAM_H
#define MAKAILIB_ANIMA_V2_RUNTIME_PROGRAM_H

#include "../instruction.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Program {
		using Version = Data::Version;

		constexpr static Version const LANG_VER = {2};

		struct Labels {
			Dictionary<usize>	globals;
			Dictionary<usize>	jumps;
		};

		Version					language;
		Data::Value::ArrayType	types;
		Data::Value::ArrayType	constants;
		List<Instruction>		code;
		List<uint64>			jumpTable;
		Labels					labels;

		constexpr Data::Value serialize() const {
			Data::Value out;
			out["types"]		= types;
			out["constants"]	= constants;
			out["jumps"]		= jumpTable.toBytes();
			out["code"]			= code.toBytes();
			out["labels"]		= out.object();
			out["version"]		= language;
			auto& outLabels = out["labels"];
			outLabels["jumps"]		=
			outLabels["globals"]	= out.object();
			auto& outJumps		= outLabels["jumps"];
			auto& outGlobals	= outLabels["globals"];
			for (auto& [name, id]: labels.globals)
				outGlobals[name] = id;
			for (auto& [name, id]: labels.jumps)
				outJumps[name] = id;
			return out;
		}

		constexpr static Program deserialize(Data::Value const& v) {
			Program prog;
			if (v.contains("version"))
				prog.language = v["version"];
			else prog.language = {2};
			switch (prog.language.major) {
				case 2: deserializeV2(prog, v); break;
				default: break;
			}
			return prog;
		}

		constexpr static void deserializeV2(Program& prog, Data::Value const& v) {
			if (v.contains("types") && v["types"].isArray())
				prog.types		= v["types"].get<Data::Value::ArrayType>();
			if (v.contains("constants") && v["constants"].isArray())
				prog.constants	= v["constants"].get<Data::Value::ArrayType>();
			auto const code		= v["code"].get<Data::Value::ByteListType>();
			auto const jumps	= v["jumps"].get<Data::Value::ByteListType>();
			prog.code		= decltype(prog.code){ref<Instruction>(code.data()), ref<Instruction>(code.data()) + (code.size() / sizeof(Instruction))};
			prog.jumpTable	= decltype(prog.jumpTable){ref<uint64>(jumps.data()), ref<uint64>(jumps.data()) + (jumps.size() / sizeof(uint64))};
			auto const jumpLabels	= v["labels"]["jumps"];
			auto const globalLabels	= v["labels"]["globals"];
			for (auto [label, id]: jumpLabels.items())
				prog.labels.jumps[label]	= id;
			for (auto [label, id]: globalLabels.items())
				prog.labels.globals[label]	= id;
		}
	};
}

#endif
