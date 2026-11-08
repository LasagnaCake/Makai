#include <makai/makai.hpp>
#include "base.cc"

constexpr auto const VER = Makai::Data::Version{1};

static Makai::Data::Value configBase() {
	Makai::Data::Value cfg;
	cfg["help"]		= false;
	cfg["output"]	= "${name}.pack";
	cfg["pass"]		= "password";
	cfg["type"]		= "base64";
	return cfg;
}

static void translationBase(Makai::CLI::Parser::Translation& tl) {
	tl["H"]	= "help";
	tl["o"]	= "output";
	tl["p"]	= "pass";
	tl["t"]	= "type";
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
		DEBUGLN("Available commands:");
		DEBUGLN("pack <folder> [--password <password>] [--output <archive>]");
		DEBUGLN("unpack <archive> [--password <password>] [--output <folder>]");
		DEBUGLN("keygen <file> [--password <password>] [--type <type>]");
	}

	static void doPack(Makai::Data::Value const& cfg) {
		DEBUGLN("Packing archive...");
		if (cfg["__args"].size() < 2)
			throw Makai::Error::NonexistentValue("Missing folder name!");
		auto const packName = Makai::Regex::replace(
			cfg["output"],
			"\\$\\{name\\}",
			cfg["__args"][1]
				.get<Makai::String>()
				.replace('\\', '/')
				.splitAtLast('/')
				.back()
		);
		auto const source = cfg["__args"][1].getString();
		auto const packRoot = packName.replaced('\\', '/').splitAtLast('/').back();
		Makai::Bytes<> outFile;
		{
			Makai::Pack::Archive pa;
			pa.open(
				Makai::Data::hashed(cfg["pass"].getString(), Makai::Data::HashMode::HM_SHA3_256),
				Makai::Pack::Archive::Destination::create<Makai::Pack::BF::Writer>(outFile)
			);
			for (auto const& f: Makai::OS::FS::walkFiles(source)) {
				auto const destPath = f.sliced(source.size());
				if (auto const e = pa.tryAdd(destPath, {.data = Makai::File::loadBinary(f)})) {
					// TODO: Error handling
				}
			}
			pa.close();
		}
		Makai::File::saveBinary(packName, outFile);
		DEBUGLN("Done!");
	}

	static void doUnpack(Makai::Data::Value const& cfg) {
		DEBUGLN("Unpacking archive...");
		if (cfg["__args"].size() < 2)
			throw Makai::Error::NonexistentValue("Missing archive name!");
		auto const outFolder = Makai::Regex::replace(
			cfg["output"],
			"\\$\\{name\\}(\\.pack)?",
			Makai::OS::FS::fileName(cfg["__args"][1], true)
		);
		{
			Makai::Pack::Archive pa;
			pa.open(
				Makai::Data::hashed(cfg["pass"].getString(), Makai::Data::HashMode::HM_SHA3_256),
				Makai::Pack::Archive::Source::create<Makai::InputFileStream<Makai::Bytes<>>>(cfg["__args"][1].getString())
			);
			pa.files().then([&] (auto const& files) {
				for (auto const& f: files) {
					pa.tryGet(f).then([&] (auto const& fe) {
						Makai::File::saveBinary(Makai::OS::FS::concatenate(outFolder, f), fe.data);
					}).onError([] (auto const& e) {
						// TODO: Error handling
					});
				}
			});
			pa.close();
		}
		DEBUGLN("Done!");
	}

	static void doKeygen(Makai::Data::Value const& cfg) {
		DEBUGLN("Generating keyfile...");
		usize sz = srng.number<usize>(32, 64);
		CTL::String const passhash = Makai::Data::hashed(cfg["pass"].getString(), Makai::Data::HashMode::HM_SHA3_256);
		DEBUGLN("Password hash size: ", passhash.size());
		if (cfg["type"].getString() == "embed") {
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
			for (char const c: passhash) {
				std::stringstream stream;
				stream << std::hex << (unsigned int)(unsigned char)(c);
				std::string code = stream.str();
				keyfile += CTL::String("\\x") + (code.size()<2?"0":"") + CTL::String(code);
			}
			keyfile += "\");\n";
			keyfile += "#endif\n";
			Makai::File::saveText((cfg["__args"].size() < 2) ? "key.256.h" : cfg["__args"][1], keyfile);
		} else if (cfg["type"].getString() == "base64") {
			Makai::File::saveText((cfg["__args"].size() < 2) ? "key.txt" : cfg["__args"][1], Makai::Data::encode(passhash.toBytes(), Makai::Data::EncodingType::ET_BASE64));
		}
		DEBUGLN("Key generated!");
	}
}

int main(int argc, char** argv) try {
	if (CTL::CPP::Debug::hasDebugger())
		CTL::CPP::Debug::Traceable::trap = true;
	Makai::CLI::Parser cli(argc, argv);
	translationBase(cli.tl);
	auto cfg = cli.parse(configBase());
	if (cfg["help"] or cfg["__args"].empty())
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
