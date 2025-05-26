#include "makai/ctl/ctl/container/list.hpp"
#include "makai/ctl/ctl/container/span.hpp"
#define ARCSYS_APPLICATION_

#include <makai/tool/archive/archive.hpp>
#include <makai/data/encdec.hpp>

CTL::String escape(char const c) {
	switch (c) {
		case '\'':	return "\\'";
		case '\"':	return "\\\"";
		case '\?':	return "\\?";
		case '\\':	return "\\\\";
		case '\a':	return "\\a";
		case '\b':	return "\\b";
		case '\f':	return "\\f";
		case '\n':	return "\\n";
		case '\r':	return "\\r";
		case '\t':	return "\\t";
		case '\v':	return "\\v";
		default:	return CTL::toString(c);
	}
}

CTL::Random::SecureGenerator srng;

int main(int argc, char** argv) {
	DEBUGLN("Starting...");
	if (argc < 2)
		DEBUGLN(
			"\n\nHow to use ArcGen:\n\n"
			"arcgen.exe \"YOUR_PASSWORD_HERE\""
		);
	else if (argc >= 2) {
		usize sz = srng.number<usize>(64, 128);
		CTL::String keyfile = CTL::toString(
			"consinit ObfuscatedStaticString<",
			sz
			,"> const passkey = ObfuscatedStaticString<",
			sz
			,">(\""
		);
		CTL::String const passhash = Makai::Tool::Arch::hashPassword(argv[1]);
		DEBUGLN("Password hash size: ", passhash.size());
		keyfile += Makai::Data::encode(
			Makai::BinaryData<>(
				(byte const*)passhash.data(), 
				passhash.size()
			), 
			Makai::Data::EncodingType::ET_BASE64
		);
		keyfile += "\");";
		Makai::File::saveText("key.256.h", keyfile);
		DEBUGLN("Key generated!");
	}
	return 0;
}
