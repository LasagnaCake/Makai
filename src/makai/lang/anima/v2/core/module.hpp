#ifndef MAKAILIB_ANIMA_V2_CORE_MODULE_H
#define MAKAILIB_ANIMA_V2_CORE_MODULE_H

#include "instruction.hpp"
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

		using Refs = List<Symbol>;

		struct Method {
			uint64			id;
			UTF8String		name;
			uint64			hash;
			uint64			retType;
			List<uint64>	argTypes;
			bool			out = false;
			bool			shared = false;
			bool			optional = false;
			uint64			entrypoint;
			uint64			size;
			Data::Value		meta;

			Data::Value serialize() const;
			static Method deserialize(Data::Value const& v);
		};

		struct Declaration {
			uint64						id;
			UTF8String					name;
			uint64						hash;
			uint64						flags		= 0;
			Nullable<Core::BasicType>	basic		= Core::BasicType::AV2_BT_VOID;
			Nullable<uint64>			base		= null;
			uint64						byteSize	= 0;
			uint64						alignment	= 0;
			List<uint64>				fields;
			MethodTable					vtable;
			Data::Value					meta;

			Data::Value serialize() const;
			static Declaration deserialize(Data::Value const& v);
		};

		struct Symbols {
			Refs	methods;
			Refs	types;

			Data::Value serialize() const;
			static Symbols deserialize(Data::Value const& v);
		};

		struct Detail {
			List<Declaration>	types;
			List<Method>		methods;

			Data::Value serialize() const;
			static Detail deserialize(Data::Value const& v);
		};

		struct ANI {
			Label		in;
			StringList	out;

			struct Shared {
				StringList	libraries;
				StringList	modules;
				StringList	interops;

				Data::Value serialize() const;
				static Shared deserialize(Data::Value const& v);
			} shared;

			Data::Value serialize() const;
			static ANI deserialize(Data::Value const& v);
		};

		Data::Value serialize(bool forceSymbolsToBeKept = false) const;

		static Module deserialize(Data::Value const& v);

		Type			type		= Type::AV2_CMT_CLI_EXE;
		String			name;
		Version			art			= ART_VER;
		Version			version		= {0, 0, 1};
		StringList		strings;
		Bytecode		code;
		List<uint64>	jumpTable;
		Detail			detail;
		Symbols			sym;
		Instance<ANI>	ani			= new ANI();
		uint64			entry		= -1;
		List<uint64>	relocations;
	};
}

#endif
