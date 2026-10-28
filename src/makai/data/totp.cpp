#include "totp.hpp"

#include "hmac.hpp"

using namespace Makai;

String Data::totp(String const& secret, usize const interval, usize const digits) {
	u64 ctime = OS::Time::time() / interval;
	String msg;
	msg.resize(8, '\0');
	for (auto& c: Range::reverse(msg))
		c = (ctime >>= 8) & 0xFF;
	auto const tok = Data::hmac(msg, secret, Data::HashMode::HM_SHA1)
	u64 num = String::toNumber<u64>(hc.sliced(hc.back() & 0x0f), 32) & 0x7FFFFFFF;
	return toString(num).resize(digits, '0');
}
