#include "encdec.hpp"

using namespace Makai;

using namespace Makai::Data;

BinaryData<> Makai::Data::decode(String const& data, EncodingType const& encoding) {
	std::vector<ubyte> result;
	switch (encoding) {
		case EncodingType::ET_BASE32: return Convert::fromBase<CTL::Convert::Base::CB_BASE32>(data); break;
		case EncodingType::ET_BASE64: return Convert::fromBase<CTL::Convert::Base::CB_BASE64>(data); break;
		default: throw Error::InvalidValue("Invalid encoding type!", CTL_CPP_PRETTY_SOURCE);
	}
	return BinaryData<>(result.data(), result.data() + result.size());
}

String Makai::Data::encode(BinaryData<> const& data, EncodingType const& encoding) {
	switch (encoding) {
		case EncodingType::ET_BASE32: return Convert::toBase<CTL::Convert::Base::CB_BASE32>(data);
		case EncodingType::ET_BASE64: return Convert::toBase<CTL::Convert::Base::CB_BASE64>(data);
		default: throw Error::InvalidValue("Invalid encoding type!", CTL_CPP_PRETTY_SOURCE);
	}
}

String Makai::Data::toString(EncodingType const& type) {
	switch (type) {
		case EncodingType::ET_BASE32: return "base32";
		case EncodingType::ET_BASE64: return "base64";
		default: throw Error::InvalidValue("Invalid encoding type!", CTL_CPP_PRETTY_SOURCE);
	}
}

EncodingType Makai::Data::fromString(String const& type) {
	if (type == "base32") return EncodingType::ET_BASE32;
	if (type == "base64") return EncodingType::ET_BASE64;
	throw Error::InvalidValue("Invalid encoding type of '" + type + "'!", CTL_CPP_PRETTY_SOURCE);
}
