#include <makai/makai.hpp>
#include "base.cc"

using namespace Makai::Anima::V2;
using namespace Toolchain::Compiler::Breve;

using namespace Toolchain;

constexpr auto const VER = Makai::Data::Version{1};

static Makai::Data::Value configBase() {
	Makai::Data::Value cfg;
	cfg["help"]		= false;
	cfg["strip"]	= false;
	cfg["output"]	= "**{{name}}";
	cfg["src"]		= cfg.array();
	cfg["level"]	= "full";
	return cfg;
}

static void translationBase(Makai::CLI::Parser::Translation& tl) {
	tl["H"]	= "help";
	tl["l"]	= "level";
	tl["o"]	= "output";
	tl["s"]	= "src";
	tl["S"]	= "strip";
}

static void doHelpMessage() {
	DEBUGLN("Breve Compiler - V" + VER.serialize().get<Makai::String>());
	DEBUGLN("Usage:");
	DEBUGLN(R"(    brevec <file> [--output <name>] [-l <compilation-level>] [--src "[<source-dirs> ...]"] [-X])");
}

int main(int argc, char** argv) try {
	if (CTL::CPP::Debug::hasDebugger() or (CTL_TARGET_OS == CTL_OS_UNIX))
		CTL::CPP::Debug::Traceable::trap = true;
	Makai::CLI::Parser cli(argc, argv);
	translationBase(cli.tl);
	auto cfg = cli.parse(configBase());
	Transformer::Import::importer = [dirs = cfg["src"].getArray().toList<Makai::String>()] (auto const path) -> File {
		if (Makai::OS::FS::exists(path + ".bv"))
			return parseFile(path + ".bv", Makai::File::getText(path + ".bv"));
		auto const brevecDir = Makai::OS::FS::sourceLocation() + "/anima/breve/lib";
		if (Makai::OS::FS::exists(brevecDir + "/" + path + ".bv"))
			return parseFile(brevecDir + "/" + path + ".bv", Makai::File::getText(brevecDir + "/" + path + ".bv"));
		for (auto& dir: dirs)
			if (Makai::OS::FS::exists(dir + "/" + path + ".bv"))
				return parseFile(dir + "/" + path + ".bv", Makai::File::getText(dir + "/" + path + ".bv"));
		throw Makai::Error::FailedAction("Failed to find module '" + path + "'");
	};
	if (cfg["help"])
		doHelpMessage();
	else {
		DEBUGLN("Here!");
		if (cfg["__args"].empty())
			throw Makai::Error::NonexistentValue("No file given!");
		auto const file = Makai::OS::FS::standardize(cfg["__args"][0].get<Makai::String>(), Makai::OS::FS::PathSeparator::PS_POSIX);
		DEBUGLN("Compiling file \"", file, "\"...");
		auto const level = cfg.fetch<Makai::String>("level", "full");
		auto const outName = Makai::Regex::replace(
			cfg["output"].getString(),
			R"(\*\*\{\{name\}\})",
			file
				.splitAtLast('/').back()
				.splitAtLast('.').front()
		);
		auto const outPath = Makai::OS::FS::currentDirectory() + "/output/" + outName;
		DEBUGLN("Level: ", level);
		if (level == "parse-tree" || level == "parse") {
			Makai::File::saveText(
				outPath + ".bpt",
				compile(outName, Makai::File::getText(file), CompilationLevel::AV2_TCB_CCL_PARSE_TREE).toFLOWString()
			);
		} else if (level == "intermediate" || level == "ir") {
			Makai::File::saveText(
				outPath + ".bir",
				compile(outName, Makai::File::getText(file), CompilationLevel::AV2_TCB_CCL_INTERMEDIATE).toFLOWString()
			);
		} else if (level == "minima" || level == "min") {
			Makai::File::saveText(
				outPath + ".min",
				compile(outName, Makai::File::getText(file), CompilationLevel::AV2_TCB_CCL_MINIMA).getString()
			);
		} else {
			Makai::File::saveText(
				outPath + ".anp",
				Makai::Regex::replace(
					compile(outName, Makai::File::getText(file), CompilationLevel::AV2_TCB_CCL_FULL, cfg["strip"]).toFLOWString(),
					"\n+", " "
				)
			);
		}
	}
	return 0;
} catch (Makai::Error::Generic const& e) {
	DEBUGLN(e.report());
	return -1;
} catch (Makai::Exception const& e) {
	DEBUGLN(e.what());
	return -1;
}
