#include "makai/lang/anima/v2/toolchain/assembler/minima.hpp"
#include <makai/makai.hpp>

using namespace Makai::Anima::V2::Toolchain;

constexpr auto const VER = Makai::Data::Version{1};

static Makai::Data::Value configBase() {
	Makai::Data::Value cfg;
	cfg["help"]		= false;
	cfg["output"]	= "out.anp";
	cfg["link"]		= cfg.array();
	return cfg;
}

static void translationBase(Makai::CLI::Parser::Translation& tl) {
	tl["H"]	= "help";
	tl["o"]	= "output";
	tl["l"]	= "link";
	tl["c"]	= "code";
	tl["P"]	= "pack";
	tl["S"]	= "strip";
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
		auto const outPath = Makai::OS::FS::currentDirectory() + "/output/" + outName;
		auto const code = Assembler::Minima::assemble(file);
		DEBUGLN("Done!");
		Makai::File::saveText(
			outPath + ".anp",
			code.serialize().toFLOWString(cfg.fetch("strip", false) ? null : "  ")
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
