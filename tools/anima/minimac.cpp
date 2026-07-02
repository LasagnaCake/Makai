#include <makai/makai.hpp>

using namespace Makai::Anima::V2::Toolchain;

constexpr auto const VER = Makai::Data::Version{1};

static Makai::Data::Value configBase() {
	Makai::Data::Value cfg;
	cfg["help"]		= false;
	cfg["output"]	= "output/out.anp";
	cfg["link"]		= cfg.array();
	cfg["write"]	= false;
	return cfg;
}

static void translationBase(Makai::CLI::Parser::Translation& tl) {
	tl["H"]	= "help";
	tl["o"]	= "output";
	tl["l"]	= "link";
	tl["c"]	= "code";
	tl["S"]	= "strip";
	tl["P"]	= "pretty";
	tl["W"]	= "write";
}

static void doHelpMessage() {
	DEBUGLN("Minima Compiler - V" + VER.serialize().get<Makai::String>());
	DEBUGLN("Usage:");
	DEBUGLN(R"(minimac (<file> OR -c <code>) [--output <name>] [--link "[<modules> ...]"])");
	DEBUGLN("init");
}

int main(int argc, char** argv) try {
	if (CTL::CPP::Debug::hasDebugger())
		CTL::CPP::Debug::Traceable::trap = true;
	Makai::CLI::Parser cli(argc, argv);
	translationBase(cli.tl);
	auto cfg = cli.parse(configBase());
	if (cfg["help"])
		doHelpMessage();
	else {
		DEBUGLN("Assembilg minima program...");
		if (cfg["__args"].empty() && !cfg.contains("code"))
			throw Makai::Error::NonexistentValue("No file given!");
		auto const file = cfg.contains("code") ? cfg["code"].getString() : Makai::File::getText(cfg["__args"][0].getString());
		auto const outName = Makai::Regex::replace(
			cfg["output"].getString(),
			R"(\*\*\{\{name\}\})",
			file
				.splitAtLast('/').back()
				.splitAtLast('.').front()
		);
		auto const outPath = Makai::OS::FS::currentDirectory() + "/" + outName;
		auto const code = Assembler::Minima::assemble(outName, file);
		DEBUGLN("Done!");
		Makai::Data::Value::Padding pad;
		if (cfg.fetch("pretty", false))
			pad = Makai::String("  ");
		Makai::File::saveText(
			outPath	 + ".anp",
			code.serialize().toFLOWString(pad)
		);
	}
	return 0;
} catch (Makai::Error::Generic const& e) {
	DEBUGLN(e.report());
	return -1;
} catch (Makai::Exception const& e) {
	DEBUGLN(e.what());
	return -1;
}
