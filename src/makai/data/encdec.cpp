#include "encdec.hpp"

using namespace Makai;

using namespace Makai::Data;

BinaryData<> Makai::Data::decode(String const& data, EncodingType const& encoding) {
	std::vector<ubyte> result;
	switch (encoding) {
		case EncodingType::ET_BASE32: return Convert::fromBase<CTL::Convert::Base::CB_BASE32>(data);
		case EncodingType::ET_BASE64: return Convert::fromBase<CTL::Convert::Base::CB_BASE64>(data);
		case EncodingType::ET_BASE2: return Convert::fromBase<CTL::Convert::Base::CB_BASE2>(data);
		case EncodingType::ET_BASE4: return Convert::fromBase<CTL::Convert::Base::CB_BASE4>(data);
		case EncodingType::ET_BASE8: return Convert::fromBase<CTL::Convert::Base::CB_BASE8>(data);
		case EncodingType::ET_BASE16: return Convert::fromBase<CTL::Convert::Base::CB_BASE16>(data);
		default: throw Error::InvalidValue("Invalid encoding type!", CTL_CPP_PRETTY_SOURCE);
	}
	return BinaryData<>(result.data(), result.data() + result.size());
}

String Makai::Data::encode(BinaryData<> const& data, EncodingType const& encoding) {
	switch (encoding) {
		case EncodingType::ET_BASE32: return Convert::toBase<CTL::Convert::Base::CB_BASE32>(data);
		case EncodingType::ET_BASE64: return Convert::toBase<CTL::Convert::Base::CB_BASE64>(data);
		case EncodingType::ET_BASE2: return Convert::toBase<CTL::Convert::Base::CB_BASE2>(data);
		case EncodingType::ET_BASE4: return Convert::toBase<CTL::Convert::Base::CB_BASE4>(data);
		case EncodingType::ET_BASE8: return Convert::toBase<CTL::Convert::Base::CB_BASE8>(data);
		case EncodingType::ET_BASE16: return Convert::toBase<CTL::Convert::Base::CB_BASE16>(data);
		default: throw Error::InvalidValue("Invalid encoding type!", CTL_CPP_PRETTY_SOURCE);
	}
}

String Makai::Data::toString(EncodingType const& type) {
	switch (type) {
		case EncodingType::ET_BASE32: return "base32";
		case EncodingType::ET_BASE64: return "base64";
		case EncodingType::ET_BASE2: return "base2";
		case EncodingType::ET_BASE4: return "base4";
		case EncodingType::ET_BASE8: return "base8";
		case EncodingType::ET_BASE16: return "base16";
		default: throw Error::InvalidValue("Invalid encoding type!", CTL_CPP_PRETTY_SOURCE);
	}
}

EncodingType Makai::Data::fromString(String const& type) {
	if (type == "base32") return EncodingType::ET_BASE32;
	if (type == "base64") return EncodingType::ET_BASE64;
	if (type == "base2") return EncodingType::ET_BASE2;
	if (type == "base4") return EncodingType::ET_BASE4;
	if (type == "base8") return EncodingType::ET_BASE8;
	if (type == "base16") return EncodingType::ET_BASE16;
	throw Error::InvalidValue("Invalid encoding type of '" + type + "'!", CTL_CPP_PRETTY_SOURCE);
}
