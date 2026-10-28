#include "hmac.hpp"

using namespace Makai;

String Data::hmac(String const& message, String const& key, Data::HashMode const mode) {
	String hc = key;
	while (hc.size() < 64) hc.pushBack('0');
	if (hc.size() > 64) Data::hash(hc, mode);
	String okey = hc, ikey = hc;
	for (auto& c: ikey) okey ^= 0x36;
	for (auto& c: okey) okey ^= 0x5C;
	return Data::hashed(okey + Data::hashed(ikey + message, mode), mode);
}
