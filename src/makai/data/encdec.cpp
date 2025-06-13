#include <cppcodec/base64_rfc4648.hpp>
#include <cppcodec/base32_rfc4648.hpp>

#include "encdec.hpp"

using namespace Makai;

using namespace Data;

BinaryData<> Data::decode(String const& data, EncodingType const& encoding) try {
	std::vector<ubyte> result;
	switch (encoding) {
		case EncodingType::ET_BASE32: result = cppcodec::base32_rfc4648::decode<std::vector<ubyte>>(data); break;
		case EncodingType::ET_BASE64: result = cppcodec::base64_rfc4648::decode<std::vector<ubyte>>(data); break;
		default: throw Error::InvalidValue("Invalid encoding type!", CTL_CPP_PRETTY_SOURCE);
	}
	return BinaryData<>(&*result.begin(), &*result.end());
} catch (cppcodec::parse_error const& e) {
	throw Error::FailedAction(
		"Failed at decoding byte data!",
		e.what(),
		CTL_CPP_PRETTY_SOURCE
	);
}

String Data::encode(BinaryData<> const& data, EncodingType const& encoding) try {
	switch (encoding) {
		case EncodingType::ET_BASE32: return cppcodec::base32_rfc4648::encode<std::string>(data);
		case EncodingType::ET_BASE64: return cppcodec::base64_rfc4648::encode<std::string>(data);
		default: throw Error::InvalidValue("Invalid encoding type!", CTL_CPP_PRETTY_SOURCE);
	}
} catch (cppcodec::parse_error const& e) {
	throw Error::FailedAction(
		"Failed at decoding byte data!",
		e.what(),
		CTL_CPP_PRETTY_SOURCE
	);
}

String Data::toString(EncodingType const& type) {
	switch (type) {
		case EncodingType::ET_BASE32: return "base32";
		case EncodingType::ET_BASE64: return "base64";
		default: throw Error::InvalidValue("Invalid encoding type!", CTL_CPP_PRETTY_SOURCE);
	}
}

EncodingType Data::fromString(String const& type) {
	if (type == "base32") return EncodingType::ET_BASE32;
	if (type == "base64") return EncodingType::ET_BASE64;
	throw Error::InvalidValue("Invalid encoding type of '" + type + "'!", CTL_CPP_PRETTY_SOURCE);
}
