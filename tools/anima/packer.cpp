#include <makai/makai.hpp>

constexpr auto const VER = Makai::Data::Version{1};

static Makai::Data::Value configBase() {
	Makai::Data::Value cfg;
	cfg["help"]		= false;
	cfg["output"]	= "${name}.pack";
	cfg["pass"]		= "password";
	return cfg;
}

static void translationBase(Makai::CLI::Parser::Translation& tl) {
	tl["help"]	= "H";
	tl["h"]		= "H";
	tl["o"]		= "output";
	tl["p"]		= "pass";
}

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

namespace Command {
	static void doHelpMessage() {
		DEBUGLN("Anima Packer - V" + VER.serialize().get<Makai::String>());
	}

	static void doPack(Makai::Data::Value const& cfg) {
		DEBUGLN("Packing archive...");
		if (cfg["__args"].size() < 2)
			throw Makai::Error::NonexistentValue("Missing folder name!");
		auto const packName = Makai::Regex::replace(
			cfg["output"],
			"${name}",
			cfg["__args"][1]
				.get<Makai::String>()
				.replace('\\', '/')
				.splitAtLast('/')
				.back()
		);
		Makai::Tool::Arch::pack(cfg["output"], cfg["__args"][0], cfg["password"]);
		DEBUGLN("Done!");
	}

	static void doUnpack(Makai::Data::Value const& cfg) {
		DEBUGLN("Unpacking archive...");
		if (cfg["__args"].size() < 2)
			throw Makai::Error::NonexistentValue("Missing archive name!");
		auto const packName = Makai::Regex::replace(
			cfg["output"],
			"${name}(\\.pack)?",
			Makai::OS::FS::fileName(cfg["__args"][1], true)
		);
		Makai::Tool::Arch::unpack(cfg["__args"][0], cfg["output"], cfg["password"]);
		DEBUGLN("Done!");
	}

	static void doKeygen(Makai::Data::Value const& cfg) {
		DEBUGLN("Generating keyfile...");
		usize sz = srng.number<usize>(32, 64);
		CTL::String const pkid = CTL::toString("PASSKEY_ID", srng.integer<usize>(), "EX");
		CTL::String keyfile = "";
		keyfile += CTL::toString(
			"#ifndef ", pkid, "_H\n",
			"#define ", pkid, "_H\n",
			"#include <makai/makai.hpp>\n"
		); 
		keyfile += CTL::toString(
			"constinit static CTL::Ex::ObfuscatedStaticString<",
			sz
			,"> const PASS_KEY = CTL::Ex::ObfuscatedStaticString<",
			sz
			,">(\""
		);
		CTL::String const passhash = Makai::Tool::Arch::hashPassword(cfg["password"]);
		DEBUGLN("Password hash size: ", passhash.size());
		for (char const c: passhash) {
			std::stringstream stream;
			stream << std::hex << (unsigned int)(unsigned char)(c);
			std::string code = stream.str();
			keyfile += CTL::String("\\x") + (code.size()<2?"0":"") + CTL::String(code);
		}
		keyfile += "\");\n";
		keyfile += "#endif\n";
		Makai::File::saveText((cfg["__args"].size() < 2) ? "key.256.h" : cfg["__args"][1], keyfile);
		DEBUGLN("Key generated!");
	}
}

int main(int argc, char** argv) try {
	DEBUGLN("Initializing...");
	Makai::CLI::Parser cli(argc, ref<cstring>(argv));
	translationBase(cli.tl);
	auto cfg = cli.parse(configBase());
	if (cfg["help"])
		Command::doHelpMessage();
	else {
		if (cfg["__args"].empty())
			throw Makai::Error::NonexistentValue("Missing command!");
		auto const command = cfg["__args"][0].get<Makai::String>();
		if		(command == "pack"		)	Command::doPack(cfg);
		else if	(command == "unpack"	)	Command::doUnpack(cfg);
		else if	(command == "keygen"	)	Command::doKeygen(cfg);
		return 0;
	}
} catch (Makai::Error::Generic const& e) {
	DEBUGLN(e.report());
	return -1;
} catch (Makai::Exception const& e) {
	DEBUGLN(e.what());
	return -1;
}