#include <makai/makai.hpp>
#include "base.cc"

constexpr auto const VER = Makai::Data::Version{1};

static Makai::Data::Value configBase() {
	Makai::Data::Value cfg;
	cfg["help"]		= false;
	cfg["output"]	= "${name}.pack";
	cfg["root"]		= "";
	cfg["pass"]		= "";
	cfg["type"]		= "";
	cfg["format"]	= "base64";
	cfg["gen"]		= 4;
	return cfg;
}

static void translationBase(Makai::CLI::Parser::Translation& tl) {
	tl["H"]	= "help";
	tl["o"]	= "output";
	tl["p"]	= "pass";
	tl["t"]	= "type";
	tl["r"]	= "root";
	tl["f"]	= "format";
	tl["g"]	= "gen";
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

	static void packFolder(Makai::Pack::Archive& pa, Makai::String const& folder, Makai::String const& source, Makai::String const& root) {
		auto const files = Makai::OS::FS::walkFiles(folder);
		for (auto const& f: files) {
			auto const basepath = f.sliced(source.size()+1);
			auto const destPath = root.size() ? Makai::OS::FS::concatenate(root, basepath) : basepath;
			DEBUGLN("- ", f, " -> ", destPath);
			if (auto const e = pa.tryAdd(destPath, {.data = Makai::File::loadBinary(f)})) {
				switch (e.value()) {
					case Makai::Pack::Archive::AddError::PAE_NOT_OPEN:	throw Makai::Error::FailedAction("");
					case Makai::Pack::Archive::AddError::PAE_READ_ONLY:	throw Makai::Error::FailedAction("");
				}
			}
		}
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
			DEBUGLN("Folder: ", source);
			packFolder(pa, source, source, cfg["root"].getString());
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
		usize sz = srng.number<usize>(64, 128);
		CTL::String passhash = Makai::Data::hashed(cfg["pass"].getString(), Makai::Data::HashMode::HM_SHA3_512);
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
			Makai::File::saveText(((cfg["__args"].size() < 2) ? "key" : cfg["__args"][1].getString()) + ".256.h", keyfile);
		} else {
			bool encoded = false;
			if ((encoded = cfg["format"].getString() == "base64"))
				passhash = Makai::Data::encode(passhash.toBytes(), Makai::Data::EncodingType::ET_BASE64);
			else if ((encoded = cfg["format"].getString() == "base32"))
				passhash = Makai::Data::encode(passhash.toBytes(), Makai::Data::EncodingType::ET_BASE32);
			auto const kt = cfg["type"].getString();
			auto const fname = ((cfg["__args"].size() < 2) ? "key" : cfg["__args"][1].getString()) + "." + cfg["type"].getString();
			if (kt == "txt")
				Makai::File::saveText(fname, passhash);
			else {
				Makai::Data::Value keyfile = keyfile.object();
				keyfile["key"] = encoded ? passhash : Makai::Data::encode(passhash.toBytes(), Makai::Data::EncodingType::ET_BASE64);
				keyfile["format"] = encoded ? cfg["format"].getString() : "base64";
				if (kt == "json")
					Makai::File::saveText(fname, keyfile.toJSONString("  "));
				else if (kt == "flow")
					Makai::File::saveText(fname, keyfile.toFLOWString("  "));
				else DEBUGLN(keyfile["format"].getString(), ":", keyfile["key"].getString());
			};
		}
	}

	static CTL::Bytes<> generateSalt() {
		CTL::List<u64> salt;
		salt.reserve(64, 0);
		for (auto& b: salt)
			b = srng.number<u64>();
		return salt.toBytes();
	}

	static void doSalt(Makai::Data::Value const& cfg) {
		CTL::Bytes<> salt = cfg["pass"].getString().toBytes();
		auto const rounds = cfg["gen"].getUnsigned();
		for (usize i = 0; i < rounds; ++i)
			salt = Makai::Data::hashed(
				CTL::String::fromBytes(salt)
			+	Makai::Data::encode(
					generateSalt(),
					Makai::Data::EncodingType::ET_BASE64
				),
				Makai::Data::HashMode::HM_SHA3_512
			).toBytes();
		DEBUGLN(
			"base64:",
			Makai::Data::encode(
				salt,
				Makai::Data::EncodingType::ET_BASE64
			)
		);
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
		else if	(command == "salt"		)	Command::doSalt(cfg);
		else return -1;
		return 0;
	}
} catch (Makai::Error::Generic const& e) {
	DEBUGLN(e.report());
	return -1;
} catch (Makai::Exception const& e) {
	DEBUGLN(e.what());
	return -1;
}
