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
	using Project = Makai::Anima::V2::Toolchain::Compiler::Project;

	static Makai::Data::Value configBase() {
		Makai::Data::Value cfg;
		cfg["help"]		= false;
		cfg["write"]	= false;
		cfg["type"]		= "prog";
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

	static bool isValidProjectNameChar(char const c) {
		return (
			Makai::isIdentifierNameChar(c)
		or	c == '-'
		or	c == '.'
		);
	}

	void write(Makai::String const& what) const override {DEBUGLN(what);}

	void doCreate(Makai::Data::Value const& args) {
		if (args["__args"].size() < 2)
			error("Missing project name!");
		auto const projName = args["__args"][1].getString();
		if (Makai::OS::FS::exists(projName))
			error("Project with this name already exists!");
		if (!projName.validate(isValidProjectNameChar))
			error("Project name must only contain alphanumeric characters, '_', '.' and '-'!");
		Makai::OS::FS::makeDirectory(Makai::StringList::from(projName, projName + "/src"));
		Project project;
		project.name = projName;
		Makai::String langp;
		auto const lang = args.fetch<Makai::String>("lang", "breve");
		if (lang == "breve") {
			Makai::File::saveText(projName + "/src/main.bv", MAINFILE_BV);
			project.language = Project::Language::AV2_TCPL_BREVE;
			project.main = "main.bv";
		} else if (lang == "minima") {
			Makai::File::saveText(projName + "/src/main.min", MAINFILE_MIN);
			project.language = Project::Language::AV2_TCPL_MINIMA;
			project.main = "main.min";
		} else error("Invalid/unsupported project language!");
		auto const type = args.fetch<Makai::String>("type", "program");
		if (type == "prog")		project.type = args.fetch("binary", true) ? Project::Type::AV2_TCPT_BIN_PROGRAM : Project::Type::AV2_TCPT_WEB_PROGRAM;
		else if (type == "lib") project.type = Project::Type::AV2_TCPT_LIBRARY;
		else if (type == "exe")	project.type = Project::Type::AV2_TCPT_EXECUTABLE;
		else error("Invalid/unsupported project type!");
		Makai::File::saveText(projName + "/project.flow", project.serialize().toFLOWString("  "));
		Makai::File::saveText(projName + "/.gitignore", PROJ_GITIGNORE);
	}

	void doBuild(Makai::Data::Value const& args) {
		if (args["__args"].size() < 2)
			error("Missing build target!");
		auto const target = args["__args"][1].getString();
		Project project = Project::deserialize(Makai::File::getFLOW("project.flow"));
		Makai::OS::FS::remove("output");
		Makai::String compiler;
		switch (project.language) {
			case Project::Language::AV2_TCPL_BREVE:		compiler = "brevec";	break;
			case Project::Language::AV2_TCPL_MINIMA:	compiler = "minimac";	break;
		}
		switch (project.type) {
			case Project::Type::AV2_TCPT_LIBRARY: {
				Makai::OS::FS::makeDirectory(
					Makai::StringList::from("output/" + project.name)
				);
				Makai::OS::FS::copy("src/*", "output/" + project.name);
			} break;
			case Project::Type::AV2_TCPT_BIN_PROGRAM:
			case Project::Type::AV2_TCPT_WEB_PROGRAM: {
				if (Makai::OS::FS::isDirectory("lib")) {
					auto cache = Makai::OS::FS::exists("lib/.cache/.cache")
					?	Makai::File::getFLOW("lib/.cache/.cache")
					:	Makai::FLOW::Value::object();
					;
					auto const libs = Makai::OS::FS::foldersIn("lib");
					for (auto& lib: libs) {
						if (cache.contains(lib)) continue;
						cache[lib] = true;
						Makai::OS::launch(
							Makai::OS::FS::sourceLocation() + Makai::OS::FS::asExecutable("/concerto"),
							Makai::OS::FS::currentDirectory() + "/" + lib,
							Makai::StringList::from("build", target)
						);
						auto const outDir = "lib/.cache/" + Makai::OS::FS::childPath(lib);
						Makai::OS::FS::makeDirectory(outDir);
						Makai::OS::FS::copy(lib + "/output/*", outDir);
					}
					Makai::File::saveText("lib/.cache/.cache", cache.toFLOWString("  "));
				}
				Makai::OS::launch(
					Makai::OS::FS::sourceLocation() + "/" + Makai::OS::FS::asExecutable(compiler),
					Makai::OS::FS::currentDirectory() + "/src",
					Makai::StringList::from(
						project.main,
						"-S",
						project.type == Project::Type::AV2_TCPT_BIN_PROGRAM ? "-B" : "",
						"-o",
						"../output/" +  project.name,
						"-s",
						"[" + project.sources.join(" ") + (Makai::OS::FS::isDirectory("lib/.cache") ? " lib/.cache" : "") + "]"
					)
				);
			} break;
			case Project::Type::AV2_TCPT_EXECUTABLE:
				error("This program type is currently unsupported!");
		}
	}

	void doCache(Makai::Data::Value const& args) {
		if (args["__args"].size() < 2)
			error("Missing cache action!");
		auto const verb = args["__args"][1].getString();
		if (verb == "clear") Makai::OS::FS::remove("lib/.cache");
		else error("Invalid cache action [" + verb + "]!");
	}

	void doHelp(Makai::Data::Value const& args) {
		writeLine("Concerto - V" + VER.serialize().get<Makai::String>());
		writeLine("Available commands:");
		writeLine("concerto <action>");
	}

	Task run(Makai::Data::Value const& args) override {
		if (args.fetch("help", false)) {
			doHelp(args);
		} else {
			if (args["__args"].empty())
				co_return doHelp(args);
			auto const verb = args["__args"][0].getString();
			if (verb == "create")		doCreate(args);
			else if (verb == "build")	doBuild(args);
			else error("Invalid action [" + verb + "]!");
		}
		co_return;
	}
};

Makai_bindMain(ConcertoMain)

// TODO: This (again)
