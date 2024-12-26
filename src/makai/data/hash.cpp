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

BinaryData<> Makai::Data::hashed(BinaryData<> const& data, Makai::Data::HashMode const& mode) {
	switch (mode) {
		case HashMode::HM_SHA3_224: return hashData<CryptoPP::SHA3_224>(data);
		case HashMode::HM_SHA3_256: return hashData<CryptoPP::SHA3_256>(data);
		case HashMode::HM_SHA3_384: return hashData<CryptoPP::SHA3_384>(data);
		case HashMode::HM_SHA3_512: return hashData<CryptoPP::SHA3_512>(data);
	};
	return data;
}

void Makai::Data::hash(BinaryData<>& data, Makai::Data::HashMode const& mode) {
	data = Makai::Data::hashed(data, mode);
}
