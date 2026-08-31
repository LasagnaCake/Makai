#ifndef MAKAILIB_ANIMA_V2_CORE_BINARY_H
#define MAKAILIB_ANIMA_V2_CORE_BINARY_H

#include "module.hpp"

namespace Makai::Anima::V2::Core::BinaryFormat {
	using namespace CTL::Ex::BinaryFormat;

	template <class T> using Builder = Encoder<T>;

	using VersionInfo = CTL::Ex::Binary::Version;

	struct ByteReader: IReadable {
		Bytes<>	input;
		usize	pointer = 0;

		Bytes<> read(usize const count) override {
			MAKAILIB_DEBUGLN_FULL("Size: ", input.size());
			MAKAILIB_DEBUGLN_FULL("Reading ", count, " bytes at index ", pointer, "...");
			if (pointer > input.size())
				return {};
			pointer += count;
			if (pointer > input.size())
				return input.sliced(pointer - count, -1);
			return input.sliced(pointer - count, pointer - 1);
		}

		void go(usize const pos) override {
			pointer = pos < (input.size() - 1) ? pos : input.size() - 1;
		}

		ByteReader(Bytes<> const& input): input(input) {}
	};

	struct ByteWriter: IWritable {
		Bytes<>&	output;
		usize		pointer = 0;

		void write(ConstByteSpan<> const& bytes) override {
			if (pointer < output.size()) {
				if (output.size() < pointer + bytes.size())
					output.reserve(pointer + bytes.size(), 0);
				MX::memmove(output.data() + pointer, bytes.data(), bytes.size());
			}
			else output.appendBack(bytes.begin(), bytes.end());
			pointer += bytes.size();
		}

		void go(usize const pos) override {
			pointer = pos < (output.size() - 1) ? pos : output.size() - 1;
		}

		ByteWriter(Bytes<>& output): output(output) {}
	};

	struct [[CTL_PACKED_STRUCT]] Label: Text {
		uint64 id	= 0;
	};

	struct [[CTL_PACKED_STRUCT]] Record {
		uint64	id		= 0;
	};

	template <class T>
	requires (sizeof(T) == sizeof(uint64))
	struct [[CTL_PACKED_STRUCT]] Symbol: Record {
		Text	name;
		uint64	hash	= 0;
		T		flags	= T();
		Text	meta;
	};

	struct [[CTL_PACKED_STRUCT]] Mapping {
		uint64 src	= 0;
		uint64 dst	= 0;
	};

	struct [[CTL_PACKED_STRUCT]] Method: Symbol<Core::MethodFlags> {
		uint64			returnType	= -1;
		Data<uint64>	argTypes;
		uint64			entry		= 0;
		uint64			size		= 0;
	};

	struct [[CTL_PACKED_STRUCT]] Decl: Symbol<Core::TypeFlags> {
		BasicType		basic		= BasicType::AV2_BT_NOT_A_BASIC_TYPE;
		uint64			base		= -1;
		uint64			byteSize	= 0;
		uint64			alignment	= 0;
		Data<uint64>	fields;
	};

	struct [[CTL_PACKED_STRUCT]] Module {
		HeaderTable<Decl>	types;
		HeaderTable<Method>	methods;
	};

	struct [[CTL_PACKED_STRUCT]] Include {
		uint64				module	= 0;
		HeaderTable<Record>	types;
		HeaderTable<Record>	methods;
	};

	struct [[CTL_PACKED_STRUCT]] External {
		HeaderTable<Include> modules;
	};

	struct [[CTL_PACKED_STRUCT]] Shared {
		StringTable<String> libraries;
		StringTable<String> modules;
		StringTable<String> interops;
	};

	struct [[CTL_PACKED_STRUCT]] ANI {
		HeaderTable<Label>	in;
		StringTable<String>	out;
		Header<Shared>		shared;
	};

	struct [[CTL_PACKED_STRUCT]] FileStructure {
		As<char const[10]>		magic = "AV2::ANPB";
		Core::Module::Type		type;
		VersionInfo				artVersion		= {1};
		VersionInfo				binVersion		= {1};
		VersionInfo				moduleVersion	= {1};
		uint64					flags			= 0;
		uint64					moduleFlags		= 0;
		uint64					entry			= 0;
		uint64					totalTypes		= 0;
		uint64					totalMethods	= 0;
		StringTable<String>		strings;
		Data<uint64>			jumps;
		Data<Instruction>		code;
		Data<uint64>			relocations;
		Header<ANI>				ani;
		Header<Module>			module;
		Header<External>		external;

		bool has(uint64 const bit) const {
			return flags & bit;
		}
	};

	using FileHeader = Header<FileStructure>;

	struct Error {
		String message;
	};

	Result<Core::Module, Error>	fromBytes(Bytes<> const& source);
	Result<Bytes<>, Error>		toBytes(Core::Module const& source, bool const strip = false);
}

#endif
