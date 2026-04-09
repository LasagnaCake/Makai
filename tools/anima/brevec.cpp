#include <makai/makai.hpp>
#include "base.cc"
#include "makai/lang/anima/v2/toolchain/compiler/breve/transformer.hpp"

using namespace Makai::Anima::V2;

using namespace Toolchain;

constexpr auto const VER = Makai::Data::Version{1};

static Makai::Data::Value configBase() {
	Makai::Data::Value cfg;
	cfg["help"]		= false;
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
}

static void doHelpMessage() {
	DEBUGLN("Breve Compiler - V" + VER.serialize().get<Makai::String>());
	DEBUGLN("Usage:");
	DEBUGLN(R"(    brevec <file> [--output <name>] [-I] [--src "[<source-dirs> ...]"])");
}

int main(int argc, char** argv) try {
	//if (CTL::CPP::Debug::hasDebugger())
		CTL::CPP::Debug::Traceable::trap = true;
	Makai::CLI::Parser cli(argc, argv);
	translationBase(cli.tl);
	auto cfg = cli.parse(configBase());
	if (cfg["help"])
		doHelpMessage();
	else {
		DEBUGLN("Here!");
		if (cfg["__args"].empty())
			throw Makai::Error::NonexistentValue("No file given!");
		auto const file = Makai::OS::FS::standardize(cfg["__args"][0].get<Makai::String>(), Makai::OS::FS::PathSeparator::PS_POSIX);
		DEBUGLN("Compiling file \"", file, "\"...");
		Compiler::Breve::Compiler::Context ctx;
		Compiler::Breve::Parser parser(ctx);
		Makai::Lexer::CStyle::TokenStream stream;
		stream.open(Makai::File::getText(file));
		DEBUGLN("This part");
		Makai::List<Assembler::BaseContext::Axiom> ax;
		while (stream.next())
			ax.pushBack({stream.current(), true, file});
		DEBUGLN("That part");
		ctx.put(ax).pad();
		DEBUGLN("Blablabla");
		auto const level = cfg["level"].getString("full");
		if (level == "parse-tree" || level == "parse") {
			DEBUGLN("Doing parse tree...");
			auto const i = parser.parse();
			Makai::File::saveText(
				Makai::OS::FS::currentDirectory() + "/output/" + Makai::Regex::replace(
					cfg["output"].getString(),
					R"(\*\*\{\{name\}\})",
					file
						.splitAtLast('/').back()
						.splitAtLast('.').front()
				) + ".bpt",
				i->serialize().toFLOWString("  ")
			);
		} else if (level == "intermediate" || level == "ir") {
			Compiler::Breve::Transformer::ATransformer::Context ctx;
			Compiler::Breve::Transformer::TheEntireProgram tf;
			tf.transform(ctx, parser.parse());
			Makai::File::saveText(
				Makai::OS::FS::currentDirectory() + "/output/" + Makai::Regex::replace(
					cfg["output"].getString(),
					R"(\*\*\{\{name\}\})",
					file
						.splitAtLast('/').back()
						.splitAtLast('.').front()
				) + ".bir",
				ctx.serialize().toFLOWString("  ")
			);
		} else if (level == "minima" || level == "min") {
			Compiler::Breve::Transformer::ATransformer::Context ctx;
			Compiler::Breve::Transformer::TheEntireProgram tf;
			Compiler::Breve::Composer comp{.inter = ctx};
			tf.transform(ctx, parser.parse());
			Makai::File::saveText(
				Makai::OS::FS::currentDirectory() + "/output/" + Makai::Regex::replace(
					cfg["output"].getString(),
					R"(\*\*\{\{name\}\})",
					file
						.splitAtLast('/').back()
						.splitAtLast('.').front()
				) + ".min",
				comp.toMinima()
			);
		} else {
			// TODO: This
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
