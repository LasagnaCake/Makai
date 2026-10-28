#include "hmac.hpp"

using namespace Makai;

String Makai::Data::hmac(String const& message, String const& key, Makai::Data::HashMode const mode) {
	String hc = key;
	while (hc.size() < 64) hc.pushBack(0);
	if (hc.size() > 64) Makai::Data::hash(hc, mode);
	String okey = hc, ikey = hc;
	for (auto& c: ikey) c ^= 0x36;
	for (auto& c: okey) c ^= 0x5C;
	return Makai::Data::hashed(okey + Makai::Data::hashed(ikey + message, mode), mode);
}

String Makai::Data::HMAC::get(String const& message) const {
	return Makai::Data::hmac(message, key, mode);
}
