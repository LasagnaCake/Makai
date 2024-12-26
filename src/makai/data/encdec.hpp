#ifndef MAKAILIB_DATA_ENCDEC_H
#define MAKAILIB_DATA_ENCDEC_H

#include "../compat/ctl.hpp"

/// @brief Data manipulation facilities.
namespace Makai::Data {
	/// @brief Binary data encoding type.
	enum class EncodingType {
		ET_UNKNOWN = -1,
		ET_BASE32,
		ET_BASE64
	};

	/// @brief Decodes the given data in a given encoding scheme.
	/// @param data Data to encode.
	/// @param type Encoding sceme used.
	/// @return Decoded data.
	BinaryData<> decode(String const& data, EncodingType const& type);

	/// @brief Encodes the given data in a given encoding scheme.
	/// @param data Data to encode.
	/// @param type Encoding scheme to use.
	/// @return Encoded data.
	String encode(BinaryData<> const& data, EncodingType const& type);

	/// @brief Returns the name for a given encoding.
	/// @param type Encoding to get name for.
	/// @return Name of encoding scheme.
	String			toString(EncodingType const& type);
	/// @brief Returns the encoding for a given name.
	/// @param type Name to get encoding for.
	/// @return Encoding scheme.
	EncodingType	fromString(String const& type);
}

#endif // MAKAILIB_DATA_ENCDEC_H
