#include "totp.hpp"

#include "hmac.hpp"

using namespace Makai;

String Makai::Data::totp(String const& secret, i64 const time, u64 const interval, usize const digits, Makai::Data::HashMode const mode) {
	u64 ctime = Cast::bit<u64>(time / interval);
	String msg;
	msg.resize(8, '\0');
	for (auto& c: Range::reverse(msg))
		c = (ctime >>= 8) & 0xFF;
	auto const tok = Makai::Data::hmac(msg, secret, mode);
	u64 num = String::toNumber<u64>(tok.sliced(tok.back() & 0x0f), 32) & 0x7FFFFFFF;
	return toString(num).resize(digits, '0');
}

String Makai::Data::TOTP::get() const {
	return Makai::Data::totp(secret, OS::Time::Clock::sinceEpoch<Makai::OS::Time::Seconds>(), interval, digits, mode);
}
