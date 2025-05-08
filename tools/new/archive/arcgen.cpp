#define ARCSYS_APPLICATION_

#include <makai/tool/archive/archive.hpp>

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
		CTL::String keyfile = CTL::toString(
			"consinit ObfuscatedStaticString<",
			sz
			,"> const passkey = ObfuscatedStaticString<",
			sz
			,">(\""
		);
		for (char& c: Makai::Tool::Arch::hashPassword(argv[1])) {
			std::stringstream stream;
			stream << std::hex << (unsigned int)(unsigned char)(c);
			std::string code = stream.str();
			keyfile += CTL::String("\\x") + (code.size()<2?"0":"") + CTL::String(code);
		}
		keyfile += "\");";
		Makai::File::saveText("key.256.h", keyfile);
		DEBUGLN("Key generated!");
	}
	return 0;
}
