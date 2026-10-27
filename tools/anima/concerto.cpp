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
main :: () -> void {
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
	struct Path {
		static inline Makai::String const globalConfig = Makai::OS::FS::sourceLocation() + "/config/concerto/global.flow";
		static inline Makai::String const temp = Makai::OS::FS::sourceLocation() + "/tmp";
	};

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

	void doHelp(Makai::Data::Value const& args) {
		if (args["__args"].size() < 2 or args["__args"][0].getString() != "help") {
			writeLine("Concerto - V" + VER.serialize().get<Makai::String>());
			writeLine("Available commands:");
			writeLine("concerto create    Creates a concerto project");
			writeLine("concerto add       Adds a library to the project");
			writeLine("concerto build     Builds a concerto project");
			writeLine("concerto cache     Manage the build cache");
			writeLine("concerto app       Manage installed concerto apps");
			writeLine("concerto source    Adds a library source");
			writeLine("concerto hub       Adds a hub to the library lookup registry");
			writeLine("concerto init      Initializes a concerto project");
			writeLine("concerto run       Runs a concerto project");
			writeLine("concerto help      Displays information for a specific command");
		} else {
			auto const verb = args["__args"][1].getString();
			writeLine("\nCommand:");
			if (verb == "create") {
				writeLine("concerto create <project> [--lang (breve|minima)] [--type (prog|lib|exe)] [--bin (true|false)]");
				writeLine("");
				writeLine("Creates a concerto project with the name <project>, where:");
				writeLine("  --lang: Target language");
				writeLine("  --type: Project type");
				writeLine("  --bin: Whether to generate a binary or a web-safe file (for [--type prog] only)")
			} else if (verb == "build") {
				writeLine("concerto build <target>");
				writeLine("");
				writeLine("Builds the current project for the given build <target>");
			} else if (verb == "add") {
				writeLine("concerto add <lib>");
				writeLine("");
				writeLine("Adds a library <lib> to the current project");
			} else if (verb == "cache") {
				writeLine("concerto cache (clean|update)");
				writeLine("");
				writeLine("Actions:");
				writeLine("clean    Clears the build cache");
				writeLine("update   Ensures the build cache is up-to-date");
			} else if (verb == "app") {
				writeLine("concerto app (add|update|run) ");
				writeLine("");
				writeLine("Actions:");
				writeLine("add <app> [-G]                   Installs a concerto app [-G: globally]");
				writeLine("update                           Updates all installed apps");
				writeLine("run <app> [--args \"<args>\"]    Runs an installed app [--args: with the given arguments]");
			} else if (verb == "source") {
				writeLine("concerto source <source> [-G]");
				writeLine("");
				writeLine("Adds a library source <source> [-G: globally]");
			} else if (verb == "hub") {
				writeLine("concerto hub <repo>");
				writeLine("");
				writeLine("Adds a library hub <repo> globally");
			} else if (verb == "init") {
				writeLine("concerto init");
				writeLine("");
				writeLine("Initialises the current project for building");
			} else if (verb == "run") {
				writeLine("concerto run <target>");
				writeLine("");
				writeLine("Runs the current project for the given build <target>");
			} else if (verb == "help") {
				writeLine("concerto help [<command>]");
				writeLine("");
				writeLine("Displays help for a given command");
			}
		}
	}

	void doCreate(Makai::Data::Value const& args) {
		if (args["__args"].size() < 2)
			error("Missing project name!");
		auto const projName = args["__args"][1].getString();
		if (Makai::OS::FS::exists(projName))
			error("Folder/Project with this name already exists!");
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
				} else updateLibraries();
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

	void updateLibraries() {
		Makai::OS::FS::remove("lib");
		Makai::OS::launch(
			Makai::OS::FS::sourceLocation() + "/" + Makai::OS::FS::asExecutable("concerto"),
			Makai::OS::FS::currentDirectory(),
			Makai::StringList::from("init")
		);
	}

	void doCache(Makai::Data::Value const& args) {
		if (args["__args"].size() < 2)
			error("Missing cache action!");
		auto const verb = args["__args"][1].getString();
		if (verb == "clean") Makai::OS::FS::remove("lib/.cache");
		if (verb == "update") updateLibraries();
		else error("Invalid cache action [" + verb + "]!");
	}

	void addApp(Makai::String const& path) {

	}

	void updateApps() {

	}

	void doApp(Makai::Data::Value const& args) {
		if (args["__args"].size() < 2)
			error("Missing app action!");
		auto const verb = args["__args"][1].getString();
		if (verb == "add") {
			if (args["__args"].size() < 3)
				error("Missing app source/name!");
			addApp(args["__args"][2].getString());
		}
		if (verb == "update") updateApps();
		else error("Invalid cache action [" + verb + "]!");
	}

	void addSource(Makai::String const& path, bool const global = false) {
		Makai::Data::Value package;
		if (Makai::Regex::contains(path, R"#(^https?:\/\/)#"))
			package = Makai::FLOW::parse(Makai::Net::File::fetchText(path + "/package.flow"));
		else if (Makai::Regex::contains(path, R"#(^cla:\/\/)#")) {
			Makai::Tool::Arch::FileArchive arch;
			arch.open(
				Makai::Tool::Arch::FileArchive::Source::create<Makai::InputByteFileStream>(
					Makai::Regex::replace(path, R"#(^cla:\/\/)#", "")
				),
				Makai::Tool::Arch::hashPassword(PACKAGEKEY)
			);
			package = arch.getTextFile("project.flow");
		} else
			package = Makai::File::getText(path + "/project.flow");
		if (global) {
			auto const gloc = Makai::OS::FS::sourceLocation() + "/config/concerto/global.flow";
			Makai::Data::Value globals;
			if (Makai::OS::FS::exists(gloc))
				globals = Makai::File::getFLOW(gloc);
			globals[package["name"].getString()] = path;
			Makai::File::saveText(gloc, globals.toFLOWString("  "));
		} else {
			Project project = Project::deserialize(Makai::File::getFLOW("project.flow"));
			project.libraries[package["name"].getString()] = {path};
			Makai::File::saveText("project.flow", project.serialize().toFLOWString("  "));
			installLibrary(package["name"].getString(), path);
		}
	}

	void doSource(Makai::Data::Value const& args) {
		if (args["__args"].size() < 2)
			error("Missing source path!");
		auto const src = args["__args"][1].getString();
		Makai::Data::Value package;
		addSource(src, args.fetch("global", false));
	}

	void addRepoSources(Makai::StringList const& sources) {
		for (auto const& source: sources)
			addSource(source, true);
	}

	static Makai::StringList toStringList(Makai::Data::Value const& value) {
		Makai::StringList out;
		out.resize(value.size());
		for (usize i = 0; i < value.size(); ++i)
			if (value[i].isString())
				out.pushBack(value[i].getString());
		return out;
	}

	void addRepo(Makai::String const& path) {
		//error(Makai::Net::File::fetchText(path + "/repo.flow"));
		auto const repo = Makai::FLOW::parse(Makai::Net::File::fetchText(path + "/repo.flow"));
		if (repo.contains("sources"))
			addRepoSources(toStringList(repo["sources"]));
		if (repo.contains("repos")) {
			auto const subRepos = toStringList(repo["repos"]);
			for (auto const& path: subRepos)
				addRepo(path);
		}
	}

	void doHub(Makai::Data::Value const& args) {
		if (args["__args"].size() < 2)
			error("Missing package repository path!");
		addRepo(args["__args"][1].getString());
	}

	void doAdd(Makai::Data::Value const& args) {
		if (args["__args"].size() < 2)
			error("Missing package name!");
		auto const gloc = Makai::OS::FS::sourceLocation() + "/config/concerto/global.flow";
		auto const globals = Makai::File::getFLOW(gloc);
		auto const name = args["__args"][1].getString();
		if (!globals.contains(name))
			error("Package [" + name + "] does not exist!");
		auto const src = globals[args["__args"][1].getString()].getString();
		Project project = Project::deserialize(Makai::File::getFLOW("project.flow"));
		project.libraries[name] = {src};
		Makai::File::saveText("project.flow", project.serialize().toFLOWString("  "));
	}

	void installLibrary(Makai::String const& name, Makai::String const& source) {
		auto const outPath = "lib/" + Makai::Regex::replace(name, "[^A-Za-z0-9_]", "_");
		if (Makai::OS::FS::isDirectory(outPath)) return;
		Makai::OS::FS::makeDirectory(outPath);
		if (Makai::Regex::contains(source, R"#(^https:\/\/)#")) {
			auto const pz = Makai::Net::File::fetchBinary(source + "/package.cla");
			Makai::Tool::Arch::FileArchive arch;
			arch.open(
				Makai::Tool::Arch::FileArchive::Source::create<Makai::InputMemoryStream>(
					Makai::span(pz)
				),
				Makai::Tool::Arch::hashPassword(PACKAGEKEY)
			);
			arch.unpackTo(outPath);
		}
		else if (Makai::Regex::contains(source, R"#(^cla:\/\/)#")) {
			Makai::Tool::Arch::FileArchive arch;
			arch.open(
				Makai::Tool::Arch::FileArchive::Source::create<Makai::InputByteFileStream>(
					Makai::Regex::replace(source,R"#(^cla:\/\/)#","")
				),
				Makai::Tool::Arch::hashPassword(PACKAGEKEY)
			);
			arch.unpackTo(outPath);
		} else Makai::OS::FS::copy(source, outPath);
	}

	void doInit(Makai::Data::Value const& args) {
		Makai::OS::FS::remove("lib");
		Project project = Project::deserialize(Makai::File::getFLOW("project.flow"));
		for (auto const& [name, lib]: project.libraries)
			installLibrary(name, lib.source);
	}

	void doExport(Makai::Data::Value const& args) {
		Makai::OS::FS::copy("src", "export/src");
		Makai::OS::FS::copy("project.flow", "export");
		Makai::Tool::Arch::pack("output/package.cla", "export", PACKAGEKEY);
	}

	void run(Makai::Data::Value const& args) override {
		if (args.fetch("help", false)) {
			doHelp(args);
		} else {
			if (args["__args"].empty())
				return doHelp(args);
			auto const verb = args["__args"][0].getString();
			if (verb == "create")		doCreate(args);
			else if (verb == "build")	doBuild(args);
			else if (verb == "add")		doAdd(args);
			else if (verb == "source")	doSource(args);
			else if (verb == "hub")		doHub(args);
			else if (verb == "cache")	doCache(args);
			else if (verb == "init")	doInit(args);
			else if (verb == "export")	doExport(args);
			else if (verb == "help")	doHelp(args);
			else error("Invalid action [" + verb + "]!");
		}
	}
};

Makai_bindMain(ConcertoMain)

// TODO: This (again)
