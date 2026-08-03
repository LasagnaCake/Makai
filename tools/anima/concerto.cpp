#define MAKAILIB_MAIN_NO_POPUPS

#include <makai/makai.hpp>
#include <makai/main.hpp>
#include "base.cc"

using namespace Makai::Anima::V2;

using namespace Toolchain;

constexpr auto const VER = Makai::Data::Version{1};

constinit auto const METAPASS = Makai::obfuscate("Moriarty and the Unnamed Catharsis ~ Microcosm Genesis of Ars Poetica");

constinit auto const PACKAGEKEY = Makai::obfuscate("Binary Interloper of Esoteric Dreams ~ In Another Angelic Devil");

constexpr auto const METAINFO = R"###(
{
	key		**{{key}}
	source	"concerto.animart.dev"
	version	**{{version}}
}
)###";

constexpr auto const MAINFILE_BV = R"###(
using import core

@Main
main :: () {
    // Main code goes here...
    IO.writeLine("Hello, World!")
}
)###";

constexpr auto const MAINFILE_MIN = R"###()###";

constexpr auto const PROJ_GITIGNORE = R"###(
output/*
lib/*
)###";

struct ConcertoMain: Makai::AMain {
	static Makai::Data::Value configBase() {
		Makai::Data::Value cfg;
		cfg["help"]		= false;
		cfg["write"]	= false;
		cfg["type"]		= "program";
		cfg["lang"]		= "breve";
		cfg["bin"]		= true;
		return cfg;
	}

	static void translationBase(Makai::CLI::Parser::Translation& tl) {
		tl["H"]	= "help";
		tl["W"]	= "write";
		tl["B"]	= "bin";
	}

	ConcertoMain(Makai::CLI::Parser& cli): AMain(cli) {
		translationBase(cli.tl);
		baseArgs = configBase();
		showDialogOnError = false;
	}


	void write(Makai::String const& what) const override {DEBUGLN(what);}

	void doCreate(Makai::Data::Value const& args) {
		using Project = Makai::Anima::V2::Toolchain::Compiler::Project;
		if (args["__args"].size() < 2)
			throw Makai::Error::NonexistentValue("Missing project name!");
		auto const projName = args["__args"][1].getString();
		if (Makai::OS::FS::exists(projName))
			throw Makai::Error::InvalidValue("Project with this name already exists!");
		Makai::OS::FS::createDirectory({projName, projName + "/src"});
		Project project;
		project.name = projName;
		Makai::String langp;
		auto const lang = args.fetch("lang", "breve");
		if (lang == "breve") {
			Makai::File::saveText(projName + "/src/main.bv", MAINFILE_BV);
			project.language = Project::Language::AV2_TCPL_BREVE;
		} else if (lang == "minima") {
			Makai::File::saveText(projName + "/src/main.min", MAINFILE_MIN);
			project.language = Project::Language::AV2_TCPL_MINIMA;
		} else throw Makai::Error::InvalidValue("Invalid/unsupported project language!");
		projfile = Makai::Regex::replace(projfile, R"(\*\*\{\{langp\}\})", langp);
		auto const type = args.fetch("type", "program");
		if (type == "program")			project.type = args.fetch("binary", true) ? Project::Type::AV2_TCPT_BIN_PROGRAM : Project::Type::AV2_TCPT_WEB_PROGRAM;
		else if (type == "library") 	project.type = Project::Type::AV2_TCPT_LIBRARY;
		else if (type == "executable")	project.type = Project::Type::AV2_TCPT_EXECUTABLE;
		else throw Makai::Error::InvalidValue("Invalid/unsupported project type!");
		Makai::File::saveText(projName + "/project.flow", project.serialize().toFLOWString("  "));
		Makai::File::saveText(projName + "/.gitignore", PROJ_GITIGNORE);
	}

	void doBuild(Makai::Data::Value const& args) {

	}

	void run(Makai::Data::Value const& args) override {
		if (args.fetch("help", false)) {
			writeLine("Concerto - V" + VER.serialize().get<Makai::String>());
			writeLine("Available commands:");
			writeLine("concerto <action>");
		} else {
			if (args["__args"].empty())
				throw Makai::Error::NonexistentValue("No file given!");
			auto const verb = args["__args"][0].getString();
			if (verb == "create")	doCreate(args);
			if (verb == "build")	doBuild(args);
		}
	}
};

Makai_bindMain(ConcertoMain)

// TODO: This (again)
