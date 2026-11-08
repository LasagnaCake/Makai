#include <cryptopp/aes.h>
#include <cryptopp/zlib.h>
#include <cryptopp/modes.h>
#include <cryptopp/sha3.h>
#include <cryptopp/randpool.h>

#include "core.hpp"

#include "../data/encdec.hpp"
#include "../data/hash.hpp"

using namespace CryptoPP;


using namespace Makai;

using namespace Pack;

u64 Makai::Pack::hashOf(ConstByteSpan<> const& data) {
	return CTL::hash(data);
}


template<class T>
static Nullable<Bytes<>> cbcTransform(
	ConstByteSpan<> const&	data,
	String const&			password	= "",
	String					block		= {}
) try {
	std::string result;
	T tf;
	String pass = password;
	pass.resize(tf.MaxKeyLength(), ' ');
	block.resize(16, 0);
	tf.SetKeyWithIV((uint8*)pass.data(), pass.size(), (ref<byte>)block.data(), 16);
	StringSource ss(
		data.data(),
		data.size(),
		true,
		new StreamTransformationFilter(
			tf,
			new StringSink(result)
		)
	);
	return Bytes<>((ref<byte>)result.data(), result.size());
} catch (std::exception const& e) {
	return null;
} catch (CTL::Exception const& e) {
	return null;
}

template<Makai::Type::Equal<Deflator> T>
static T* getFlator(std::string& result, uint8 const level) {
	return new T(new StringSink(result), level);
}

template<Makai::Type::Equal<Inflator> T>
static T* getFlator(std::string& result, uint8 const level) {
	return new T(new StringSink(result));
}

template<class T>
static Nullable<Bytes<>> flate(
	ConstByteSpan<> const&	data,
	Compression const&		method	= Compression::PC_ZIP,
	uint8 const				level	= 9
) try {
	std::string result;
	switch (method) {
	case Compression::PC_NO_COMPRESSION: return Bytes<>(data);
	case Compression::PC_ZIP: {
			StringSource ss(
				data.data(),
				data.size(),
				true,
				getFlator<T>(result, Makai::Math::clamp<uint8>(level, 0, 9))
			);
		}
	}
	return Bytes<>((ref<byte>)result.data(), result.size());
} catch (std::exception const& e) {
	return null;
} catch (CTL::Exception const& e) {
	return null;
}

template<typename T>
static Nullable<Bytes<>> cbcEncrypt(
	ConstByteSpan<> const&	data,
	String const&			password	= "",
	String const&			block		= {}
) {
	return cbcTransform<typename CBC_Mode<T>::Encryption>(data, password, block);
}

template<typename T>
static Nullable<Bytes<>> cbcDecrypt(
	ConstByteSpan<> const&	data,
	String const&			password	= "",
	String const&			block		= {}
) {
	return cbcTransform<typename CBC_Mode<T>::Decryption>(data, password, block);
}

Nullable<Bytes<>> Makai::Pack::encrypt(
	ConstByteSpan<> const&	data,
	String const&			block,
	String const&			password,
	Encryption const		method
) {
	switch (method) {
		default: return null;
		case Encryption::PE_NO_ENCRYPTION:	return Bytes<>(data);
		case Encryption::PE_AES256:	return cbcEncrypt<AES>(data, password, block);
	}
	return null;
}

Nullable<Bytes<>> Makai::Pack::decrypt(
	ConstByteSpan<> const&	data,
	String const&			block,
	String const&			password,
	Encryption const		method
) {
	switch (method) {
		default: return null;
		case Encryption::PE_NO_ENCRYPTION:	return Bytes<>(data);
		case Encryption::PE_AES256:	return cbcDecrypt<AES>(data, password, block);
	}
	return null;
}

Nullable<Bytes<>> Makai::Pack::compress(
	ConstByteSpan<> const&	data,
	Compression const		method,
	uint8 const				level
) {
	return flate<Inflator>(data, method, level);
}

Nullable<Bytes<>> Makai::Pack::decompress(
	ConstByteSpan<> const&	data,
	Compression const		method,
	uint8 const				level
) {
	return flate<Deflator>(data, method, level);
}


Nullable<Bytes<>> Makai::Pack::pack(
	ConstByteSpan<> const&	data,
	String const&			block,
	String const&			key,
	Encryption const		enc,
	Compression const		comp,
	uint8 const				level
) {
	auto const cc = compress(data, comp, level);
	if (!cc) return null;
	return encrypt(data, block, key, enc);
}

Nullable<Bytes<>> Makai::Pack::unpack(
	ConstByteSpan<> const&	data,
	String const&			block,
	String const&			key,
	Encryption const		enc,
	Compression const		comp,
	uint8 const				level
) {
	auto const cc = encrypt(data, block, key, enc);
	if (!cc) return null;
	return decompress(data, comp, level);
}
