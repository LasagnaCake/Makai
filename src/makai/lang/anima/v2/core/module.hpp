#ifndef MAKAILIB_ANIMA_V2_CORE_MODULE_H
#define MAKAILIB_ANIMA_V2_CORE_MODULE_H

#include "instruction.hpp"
#include "type.hpp"

namespace Makai::Anima::V2::Core {
	struct Module {
		using Version = Data::Version;

		enum class Type {
			AV2_CMT_LIBRARY,
			AV2_CMT_CLI_EXE,
			AV2_CMT_EXE,
		};

		constexpr static Version const ART_VER = {1};

		using Label = Dictionary<usize>;

		struct Namespace {
			struct Entry {
				uint64	refID;
				String	name;
			};
			uint64		id;
			String		name;
			List<Entry>	methods;
			List<Entry>	types;
			List<Entry>	namespaces;
		};

		struct Method {
			uint64			id;
			String			name;
			uint64			retType;
			List<uint64>	argTypes;
			bool			out = false;
			String			entrypoint;
			uint64			size;
		};

		struct Declaration {
			StringList					aliases;
			uint64						flags		= 0;
			Nullable<Core::BasicType>	basic		= Core::BasicType::AV2_BT_VOID;
			Nullable<uint64>			base		= null;
			uint64						byteSize	= 0;
			uint64						alignment	= 1;
			List<uint64>				fields;
			Dictionary<uint64>			operators;
			Dictionary<uint64>			casts;
			Instance<Namespace>			ns;
		};

		struct ToRemapLater {

		};

		struct Labels {
			Label	globals;
			Label	jumps;
			Data::Value serialize() const;
			static Labels deserialize(Data::Value const& v);
		};

		struct NativeInterface {
			Label				in;
			StringList			out;
			Dictionary<String>	shared;

			Data::Value serialize() const;
			static NativeInterface deserialize(Data::Value const& v);
		};

		Data::Value serialize(bool forceSymbolsToBeKept = false) const;

		static Module deserialize(Data::Value const& v);

		Type					type;
		Version					art			= ART_VER;
		StringList				strings;
		List<Core::Instruction>	code;
		List<uint64>			jumpTable;
		Labels					labels;
		NativeInterface			ani;
		Namespace				base;
		List<Method>			methods;
		List<Declaration>		types;
		List<Namespace>			namespaces;
		Dictionary<List<usize>>	jumpsToMap;
	};
}

#endif
