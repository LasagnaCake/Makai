#ifndef MAKAILIB_ANIMA_V2_CORE_MODULE_H
#define MAKAILIB_ANIMA_V2_CORE_MODULE_H

#include "instruction.hpp"
#include "makai/ctl/ctl/container/id/ssuid.hpp"
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

		struct External {
			struct Reference {
				Nullable<uint64>	module = null;
				uint64				id;
			};
			using References = List<Reference>;
			References types, methods;
		};

		struct Method {
			String			name;
			uint64			retType;
			List<uint64>	argTypes;
			bool			out = false;
			uint64			entrypoint;
			uint64			id;
			uint64			size;
		};

		struct Declaration {
			uint64						id;
			uint64						flags		= 0;
			Nullable<Core::BasicType>	basic		= Core::BasicType::AV2_BT_VOID;
			Nullable<uint64>			base		= null;
			uint64						byteSize	= 0;
			uint64						alignment	= 1;
			List<uint64>				fields;
			List<uint64>				casts;
			Nullable<uint64>			ns;
		};

		struct Info {
			List<Method>		methods;
			List<Declaration>	types;
			List<Namespace>		namespaces;
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
		Bytecode				code;
		List<uint64>			jumpTable;
		NativeInterface			ani;
		Namespace				base;
		External::References	methods;
		External::References	types;
		External::References	namespaces;
		Info					info;
		External				external;
		StringList				requiredModules;
	};
}

#endif
