#include "makai/ctl/ctl/algorithm/strconv.hpp"
#include "makai/ctl/ctl/container/lists/list.hpp"
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
		usize sz = srng.number<usize>(32, 64);
		CTL::String const pkid = CTL::toString("PASSKEY_ID", srng.integer(), "EX");
		CTL::String keyfile = "";
		keyfile += CTL::toString(
			"#ifndef ", pkid, "_H\n",
			"#define ", pkid, "_H\n",
			"#include <makai/makai.hpp>\n"
		); 
		keyfile += CTL::toString(
			"constinit static Makai::Ex::ObfuscatedStaticString<",
			sz
			,"> const passkey = Makai::Ex::ObfuscatedStaticString<",
			sz
			,">(\""
		);
		CTL::String const passhash = Makai::Tool::Arch::hashPassword(argv[1]);
		DEBUGLN("Password hash size: ", passhash.size());
		for (char const c: passhash) {
			std::stringstream stream;
			stream << std::hex << (unsigned int)(unsigned char)(c);
			std::string code = stream.str();
			keyfile += CTL::String("\\x") + (code.size()<2?"0":"") + CTL::String(code);
		}
		keyfile += "\");\n";
		keyfile += "#endif\n";
		Makai::File::saveText("key.256.h", keyfile);
		DEBUGLN("Key generated!");
	}
	return 0;
}
