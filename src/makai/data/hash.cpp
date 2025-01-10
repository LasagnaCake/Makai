#include <cryptopp/sha3.h>

#include "hash.hpp"

using namespace Makai;

using Makai::Data::HashMode;

template<class T>
BinaryData<> hashData(BinaryData<> const& data) {
	BinaryData<> result;
	T hasher;
	hasher.Update((const byte*)data.data(), data.size());
	result.resize(hasher.DigestSize());
	hasher.Final((byte*)result.data());
	return result;
}

template<class T>
String hashString(String const& str) {
	String result;
	T hasher;
	hasher.Update((const byte*)str.data(), str.size());
	result.resize(hasher.DigestSize());
	hasher.Final((byte*)result.data());
	return result;
}

BinaryData<> Makai::Data::hashed(BinaryData<> const& data, HashMode const mode) {
	switch (mode) {
		case HashMode::HM_SHA3_224: return hashData<CryptoPP::SHA3_224>(data);
		case HashMode::HM_SHA3_256: return hashData<CryptoPP::SHA3_256>(data);
		case HashMode::HM_SHA3_384: return hashData<CryptoPP::SHA3_384>(data);
		case HashMode::HM_SHA3_512: return hashData<CryptoPP::SHA3_512>(data);
	};
	return data;
}

void Makai::Data::hash(BinaryData<>& data, HashMode const mode) {
	data = Makai::Data::hashed(data, mode);
}


String Makai::Data::hashed(String const& str, HashMode const mode) {
	switch (mode) {
		case HashMode::HM_SHA3_224: return hashString<CryptoPP::SHA3_224>(str);
		case HashMode::HM_SHA3_256: return hashString<CryptoPP::SHA3_256>(str);
		case HashMode::HM_SHA3_384: return hashString<CryptoPP::SHA3_384>(str);
		case HashMode::HM_SHA3_512: return hashString<CryptoPP::SHA3_512>(str);
	};
	return str;
}

void Makai::Data::hash(String& str, HashMode const mode) {
	str = Makai::Data::hashed(str, mode);
}