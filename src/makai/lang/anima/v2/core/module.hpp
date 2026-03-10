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
			AV2_CMT_SHARED_MODULE,
			AV2_CMT_CLI_EXE,
			AV2_CMT_EXE,
		};

		constexpr static Version const ART_VER = {1};

		using Label = Dictionary<usize>;

		struct Ref {
			Nullable<uint64>	module = null;
			uint64				id;
		};
		using Refs = List<Ref>;

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

		struct Symbol {
			Refs	methods;
			Refs	types;

			Data::Value serialize() const;
			static Symbol deserialize(Data::Value const& v);
		};

		struct Detail {
			List<Declaration>	types;

			Data::Value serialize() const;
			static Detail deserialize(Data::Value const& v);
		};

		struct Meta {
			List<Method>	methods;

			Data::Value serialize() const;
			static Meta deserialize(Data::Value const& v);
		};

		struct ANI {
			Label		in;
			StringList	out;

			Data::Value serialize() const;
			static ANI deserialize(Data::Value const& v);
		};

		Data::Value serialize(bool forceSymbolsToBeKept = false) const;

		static Module deserialize(Data::Value const& v);

		Type				type;
		Version				art			= ART_VER;
		StringList			strings;
		Bytecode			code;
		List<uint64>		jumpTable;
		Detail				detail;
		Symbol				sym;
		Instance<Meta>		meta		= new Meta();
		Instance<ANI>		ani			= new ANI();
		StringList			shared;
	};
}

#endif
