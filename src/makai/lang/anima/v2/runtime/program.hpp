#ifndef MAKAILIB_ANIMA_V2_RUNTIME_PROGRAM_H
#define MAKAILIB_ANIMA_V2_RUNTIME_PROGRAM_H

#include "../instruction.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Program {
		using Version = Data::Version;

		constexpr static Version const LANG_VER = {2};

		using Label = Dictionary<usize>;

		struct Labels {
			Label	globals;
			Label	jumps;

			constexpr Data::Value serialize() const {
				auto out = Data::Value::object();
				out["jumps"]	=
				out["globals"]	= out.object();
				auto& outJumps		= out["jumps"];
				auto& outGlobals	= out["globals"];
				for (auto& [name, id]: globals)
					outGlobals[name] = id;
				for (auto& [name, id]: jumps)
					outJumps[name] = id;
				return out;
			}

			static Labels deserialize(Data::Value const& v) {
				Labels labels;
				if (v.contains("jumps")) {
					auto const jumpLabels	= v["jumps"];
					for (auto [label, id]: jumpLabels.items())
						labels.jumps[label]		= id;
				}
				if (v.contains("globals")) {
					auto const globalLabels	= v["globals"];
					for (auto [label, id]: globalLabels.items())
						labels.globals[label]	= id;
				}
				return labels;
			}
		};

		struct NativeInterface {
			Label						in;
			StringList					out;
			struct SharedLibrary {
				String				path;
				Dictionary<String>	functions;
			};
			Dictionary<SharedLibrary>	shared;

			constexpr Data::Value serialize() const {
				auto result = Data::Value::object();
				result["shared"]	= result.object();
				result["in"]		= result.object();
				result["out"]		= result.array();
				auto& signals		= result["in"];
				auto& externs		= result["out"];
				auto& sharedLibs	= result["shared"];
				for (auto& [name, id]: in)
					signals[name] = id;
				for (auto& name: out)
					externs[externs.size()] = name;
				for (auto& [lib, funcs]: shared)
					sharedLibs[lib] = funcs.keys().toList<Data::Value>();
				return result;
			}

			static NativeInterface deserialize(Data::Value const& v) {
				NativeInterface ani;
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
				if (v.contains("shared")) {
					auto const sharedLibs	= v["shared"];
					for (auto [lib, funcs]: sharedLibs.items())
						for (auto const& f: funcs.getArray().toList<String>())
							ani.shared[lib][f] = true;
				}
				return ani;
			}
		};

		Version					language	= LANG_VER;
		Data::Value::ArrayType	types;
		Data::Value::ArrayType	constants;
		List<Instruction>		code;
		List<uint64>			jumpTable;
		Labels					labels;
		NativeInterface			ani;

		constexpr Data::Value serialize(bool const keepLabels = true) const {
			Data::Value out;
			out["types"]		= types;
			out["constants"]	= constants;
			out["jumps"]		= jumpTable.toBytes();
			out["code"]			= code.toBytes();
			out["version"]		= language;
			if (keepLabels) {
				out["labels"]	= out.object();
				auto& outLabels = out["labels"];
				outLabels["jumps"]		= out.object();
				outLabels["globals"]	= out.object();
				auto& outJumps		= outLabels["jumps"];
				auto& outGlobals	= outLabels["globals"];
				for (auto& [name, id]: labels.globals)
					outGlobals[name] = id;
				for (auto& [name, id]: labels.jumps)
					outJumps[name] = id;
			}
			out["ani"]			= ani;
			return out;
		}

		static Program deserialize(Data::Value const& v) {
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

		static void deserializeV2(Program& prog, Data::Value const& v) {
			if (v.contains("types") && v["types"].isArray())
				prog.types		= v["types"].get<Data::Value::ArrayType>();
			if (v.contains("constants") && v["constants"].isArray())
				prog.constants	= v["constants"].get<Data::Value::ArrayType>();
			auto const code		= v["code"].get<Data::Value::ByteListType>();
			auto const jumps	= v["jumps"].get<Data::Value::ByteListType>();
			prog.code		= decltype(prog.code){ref<Instruction>(code.data()), ref<Instruction>(code.data()) + (code.size() / sizeof(Instruction))};
			prog.jumpTable	= decltype(prog.jumpTable){ref<uint64>(jumps.data()), ref<uint64>(jumps.data()) + (jumps.size() / sizeof(uint64))};
			if (v.contains("labels"))
				prog.labels = Labels::deserialize(v["labels"]);
			if (v.contains("ani"))
				prog.labels = Labels::deserialize(v["ani"]);
		}
	};
}

#endif
