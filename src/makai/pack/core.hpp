#ifndef MAKAILIB_PACK_CORE_H
#define MAKAILIB_PACK_CORE_H

#include "../compat/ctl.hpp"

namespace Makai::Pack {
	/// @brief Encryption method.
	enum class Encryption: u16 {
		PE_NO_ENCRYPTION,
		PE_AES256
	};

	/// @brief Compression method.
	enum class Compression: u16 {
		PC_NO_COMPRESSION,
		PC_ZIP
	};

	/// @brief Encrypts data.
	/// @param data Data to encrypt.
	/// @param password Password to use. Can be a hashed string. By default, it is empty.
	/// @param method Encryption method. By default, it is `Encryption::PE_AES256`.
	/// @param block IV block. By default, it is set to all zeroes.
	/// @return Encrypted data.
	Nullable<Bytes<>> encrypt(
		ConstByteSpan<> const&	data,
		String const&			block,
		String const&			key		= "",
		Encryption const		method	= Encryption::PE_AES256
	);

	/// @brief Decrypts data.
	/// @param data Data to decrypt.
	/// @param password Password to use. Can be a hashed string. By default, it is empty.
	/// @param method Decryption method. By default, it is `Encryption::PE_AES256`.
	/// @param block IV block. By default, it is set to all zeroes.
	/// @return Decrypted data.
	Nullable<Bytes<>> decrypt(
		ConstByteSpan<> const&	data,
		String const&			block,
		String const&			key		= "",
		Encryption const		method	= Encryption::PE_AES256
	);

	/// @brief Compresses data.
	/// @param data Data to compress.
	/// @param method Decompression method. By default, it is `Compression::PC_ZIP`.
	/// @param level Compression level. By default, it is `9`.
	/// @return Compressed data.
	Nullable<Bytes<>> compress(
		ConstByteSpan<> const&	data,
		Compression const		method	= Compression::PC_ZIP,
		uint8 const				level	= 9
	);

	/// @brief Decompresses data.
	/// @param data Data to Decompress.
	/// @param method Decompression method. By default, it is `Compression::PC_ZIP`.
	/// @param level Decompression level. By default, it is `9`.
	/// @return Decompressed data.
	Nullable<Bytes<>> decompress(
		ConstByteSpan<> const&	data,
		Compression const		method	= Compression::PC_ZIP,
		uint8 const				level	= 9
	);

	Nullable<Bytes<>> pack(
		ConstByteSpan<> const&	data,
		String const&			block,
		String const&			key		= "",
		Encryption const		enc		= Encryption::PE_AES256,
		Compression const		comp	= Compression::PC_ZIP,
		uint8 const				level	= 9
	);

	Nullable<Bytes<>> unpack(
		ConstByteSpan<> const&	data,
		String const&			block,
		String const&			key		= "",
		Encryption const		enc		= Encryption::PE_AES256,
		Compression const		comp	= Compression::PC_ZIP,
		uint8 const				level	= 9
	);

	/// @brief Gets the hash for some data.
	/// @param data Data to get hash for.
	/// @return Hash of data.
	u64 hashOf(ConstByteSpan<> const& data);
}

#endif
