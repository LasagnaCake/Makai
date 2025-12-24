#ifndef MAKAILIB_ANIMA_V2_RUNTIME_PROGRAM_H
#define MAKAILIB_ANIMA_V2_RUNTIME_PROGRAM_H

#include "../instruction.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Program {
		struct Labels {
			Dictionary<usize>	globals;
			Dictionary<usize>	jumps;
		};
		Data::Value::ArrayType	types;
		Data::Value::ArrayType	constants;
		List<Instruction>		code;
		List<uint64>			jumpTable;
		Labels					labels;

		constexpr Data::Value serialize() {
			Data::Value out;
			out["types"]		= types;
			out["constants"]	= constants;
			out["jumps"]		= jumpTable.toBytes();
			out["code"]			= code.toBytes();
			out["labels"]		= out.object();
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
			if (v.contains("types") && v["types"].isArray())
				prog.types		= v["types"].get<Data::Value::ArrayType>();
			if (v.contains("constants") && v["constants"].isArray())
				prog.constants	= v["constants"].get<Data::Value::ArrayType>();
			return prog;
		}
	};
}

#endif
