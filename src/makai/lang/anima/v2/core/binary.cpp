#include "binary.hpp"

using namespace Makai;
using namespace Makai::Anima::V2;
using namespace Makai::Anima::V2::Core::BinaryFormat;

namespace BF = Makai::Anima::V2::Core::BinaryFormat;

Result<Core::Module, BF::Error> BF::fromBytes(Bytes<> const& source) {
	ByteReader reader(source);
	Core::Module out;
	if (auto const fheader = FileHeader::build(reader)) {
		auto const file = fheader.value();
		if (String(file.data.magic) != "AV2::ANPB")
			return Error{"File is not an ART-compatible binary!"};
		out.entry = file.data.entry;
		if (auto const code = file.data.code.fromBytes(reader))
			out.code = code.value();
		else return Error{"Failed to get code!"};
		if (auto const jumps = file.data.jumps.fromBytes(reader))
			out.jumpTable = jumps.value();
		else return Error{"Failed to get jump table!"};
		if (auto const strings = file.data.strings.fromBytes(reader))
			out.strings = strings.value();
		else return Error{"Failed to get string table!"};
		if (auto const relocations = file.data.relocations.fromBytes(reader))
			out.relocations = relocations.value();
		else return Error{"Failed to get string table!"};
		if (auto const symbols = file.data.module.build(reader)) {

		} else return Error{"Failed to get module symbol data!"};
	}
	return out;
}

Result<Bytes<>, BF::Error> BF::toBytes(Core::Module const& source) {

}
