#include <makai/makai.hpp>
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

static Makai::Data::Value const fetchSources(Makai::String const& path) {
	Makai::Data::Value db;
	if (Makai::OS::FS::exists(path)) {
		Makai::OS::FS::FileTree tree(path);
		for (auto const& f: tree.tree.getAllFiles())
			db.append(Makai::File::getFLOW(f));
	}
	return db;
}

static Makai::Data::Value const projectDatabase() {
	auto const path			= Makai::OS::FS::sourceLocation() + "sources";
	auto const localPath	= "proj/sources";
	return Makai::Data::Value::merge(fetchSources(path), fetchSources(localPath));
}

static void resolveSource(Compiler::Project& project, Makai::String const& name, Makai::String const& version) {
	static auto projdb = projectDatabase();
	if (projdb.contains(name))
		project.modules.pushBack({projdb[name], version});
}

static Makai::Data::Value configBase() {
	Makai::Data::Value cfg;
	cfg["help"]		= false;
	cfg["output"]	= "${name}";
	cfg["asm"]		= false;
	cfg["type"]		= "program";
	cfg["lang"]		= "breve";
	cfg["ver"]		= "latest";
	cfg["mode"]		= "cli";
	return cfg;
}

static void translationBase(Makai::CLI::Parser::Translation& tl) {
	tl["H"]		= "help";
	tl["Asm"]	= "asm";
	tl["Min"]	= "asm";
	tl["M"]		= "asm";
	tl["o"]		= "output";
	tl["t"]		= "type";
	tl["l"]		= "lang";
	tl["v"]		= "ver";
	tl["m"]		= "mode";
}

static Compiler::Project::File::Type getFileType(Makai::String const& name) {
	if (name == "breve" || name == "bv")	return Compiler::Project::File::Type::AV2_TC_PFT_BREVE;
	if (name == "minima" || name == "min")	return Compiler::Project::File::Type::AV2_TC_PFT_MINIMA;
	return Compiler::Project::File::Type::AV2_TC_PFT_BREVE;
}

static Makai::String getFileExtension(Compiler::Project::File::Type const& type) {
	switch (type) {
		using enum Compiler::Project::File::Type;
		case (AV2_TC_PFT_MINIMA):	return "min";
		case (AV2_TC_PFT_BREVE):	return "bv";
	}
	return "";
}

namespace Command {
	namespace Source {
		static void doAdd(Makai::Data::Value& cfg) {

		}

		static void doRemove(Makai::Data::Value& cfg) {

		}

		static void doList(Makai::Data::Value& cfg) {

		}
	}

	static void doInitialize(Makai::Data::Value& cfg) {
		auto const instPath = Makai::OS::FS::sourceLocation();
		Makai::File::saveText(
			instPath + "/config/meta.flow",
			Makai::Regex::replace(
				Makai::Regex::replace(
					METAINFO,
					R"(\*\*\{\{version\}\})",
					VER.serialize().toFLOWString()
				),
				R"(\*\*\{\{key\}\})",
				"\"" + Makai::Data::encode(
					Makai::Tool::Arch::hashPassword(
						METAPASS.deobfuscated()
					).toBytes(),
					Makai::Data::EncodingType::ET_BASE64
				) + "\""
			)
		);
	}

	static void doHelpMessage() {
		DEBUGLN("Anima Concerto - V" + VER.serialize().get<Makai::String>());
		DEBUGLN("Available commands:");
		DEBUGLN("build <target> [-Asm,-Min] [--output <name>]");
		DEBUGLN("create <name> [--type <type>] [--lang <lang>] [--mode <mode>]");
		DEBUGLN("refresh");
		DEBUGLN("add <module> [--ver <version>]");
		DEBUGLN("source <action> [<url>] [-G]");
		DEBUGLN("remove <module>");
		DEBUGLN("update");
		DEBUGLN("init");
	}

	static void doSource(Makai::Data::Value& cfg) {
		if (cfg["__args"].size() < 2)
			throw Makai::Error::NonexistentValue("Missing target!");
		// TODO: Package sources
		auto const command = cfg["__args"][1].getString();
		if (command == "add")			Source::doAdd(cfg);
		else if (command == "remove")	Source::doRemove(cfg);
		else if (command == "list")		Source::doList(cfg);
	}

	static void doBuild(Makai::Data::Value& cfg) {
		if (cfg["__args"].size() < 2)
			throw Makai::Error::NonexistentValue("Missing target!");
		DEBUGLN("Building project...");
		Compiler::Project proj;
		Compiler::Breve::Compiler::Context ctx;
		proj = proj.deserialize(Makai::File::getFLOW("project.flow"));
		Compiler::Breve::Parser parser(ctx);
		Makai::Lexer::CStyle::TokenStream stream;
		if (proj.main.path.empty())
			proj.main.source = Makai::File::getText(proj.main.path);
		stream.open(proj.main.source);
		Makai::List<Assembler::BaseContext::Axiom> ax;
		while (stream.next())
			ax.pushBack({stream.current(), true, stream.tokenText(), stream.position(), proj.main.path});
		ctx.append(ax.reverse());
		DEBUGLN("Done!");
	}

	static void doCreate(Makai::Data::Value& cfg) {
		if (cfg["__args"].size() < 2)
			throw Makai::Error::NonexistentValue("Missing project name!");
		DEBUGLN("Creating project...");
		auto proj = Compiler::Project();
		{
			auto const pt = cfg["type"].get<Makai::String>();
			if		(pt == "executable" || pt == "exe"	) proj.type = decltype(proj.type)::AV2_TC_PT_EXECUTABLE;
			else if	(pt == "program" || pt == "prg"		) proj.type = decltype(proj.type)::AV2_TC_PT_PROGRAM;
			else if	(pt == "module" || pt == "mod"		) proj.type = decltype(proj.type)::AV2_TC_PT_MODULE;
		}
		{
			auto const pt = cfg["mode"].get<Makai::String>();
			if		(pt == "console" || pt == "cli"	) proj.mode = decltype(proj.mode)::AV2_TC_PM_CONSOLE;
			else if	(pt == "window" || pt == "win"	) proj.mode = decltype(proj.mode)::AV2_TC_PM_WINDOW;
			else if	(pt == "worker" || pt == "bg"	) proj.mode = decltype(proj.mode)::AV2_TC_PM_WORKER;
		}
		proj.name = cfg["__args"][1].get<Makai::String>();
		if (Makai::OS::FS::exists(proj.name))
			throw Makai::Error::FailedAction("Project '"+proj.name+"' already exists in this folder!");
		if (proj.type == decltype(proj.type)::AV2_TC_PT_EXECUTABLE)
			throw Makai::Error::FailedAction("Standalone executable projects are curently unimplemented, sorry :/");
		Makai::OS::FS::makeDirectory(proj.name);
		proj.package = {.patch = 1};
		proj.main.type = getFileType(cfg["lang"]);
		proj.main.path = "src/main." + getFileExtension(proj.main.type);
		proj.sources.pushBack("src");
		if (proj.type != decltype(proj.type)::AV2_TC_PT_MODULE)
			Makai::File::saveText(proj.name + "/" + proj.main.path, "import core.all;\n\nmain {\n\t// Main code goes here...\n\tcore.IO.writeLine(\"Hello, world!\");\n}");
		else Makai::File::saveText(proj.name + "/all.bv", "// Full imports goes here...");
		Makai::File::saveText(proj.name + "/project.flow", proj.serialize().toFLOWString("\t"));
		Makai::File::saveText(proj.name + "/.gitignore", "output/\nmodule/\ncache.flow\n*.anp");
		DEBUGLN("Done!");
	}

	static void doRefresh(Makai::Data::Value& cfg) {
		DEBUGLN("Refreshing project...");
		Makai::OS::FS::remove("cache.flow", "module");
		Compiler::Project proj;
		Assembler::Context ctx;
		proj = proj.deserialize(Makai::File::getFLOW("project.flow"));
		Compiler::downloadProjectModules(ctx, proj);
		DEBUGLN("Done!");
	}

	static void doAdd(Makai::Data::Value& cfg) {
		if (cfg["__args"].size() < 2)
			throw Makai::Error::NonexistentValue("Missing project name!");
		DEBUGLN("Adding module...");
		Compiler::Project proj = proj.deserialize(Makai::File::getFLOW("project.flow"));
		Assembler::Context ctx;
		auto cache = Makai::FLOW::Value::object();
		if (Makai::OS::FS::exists("cache.flow"))
			cache = Makai::File::getFLOW("cache.flow");
		else cache["modules"] = Makai::FLOW::Value::array();
		Compiler::fetchModule(ctx, proj, {cfg["__args"][1], cfg["ver"]}, ".", cache);
		Makai::File::saveText("cache.flow", cache.toFLOWString("\t"));
		DEBUGLN("Done!");
	}

	static void doRemove(Makai::Data::Value& cfg) {
		DEBUGLN("Removing module...");
		auto proj = Makai::File::getFLOW("project.flow");
		proj["modules"][cfg["name"].get<Makai::String>()] = proj.undefined();
		Makai::File::saveText("project.flow", proj.toFLOWString("\t"));
		proj = Makai::File::getFLOW("cache.flow");
		proj["modules"][cfg["name"].get<Makai::String>()] = proj.undefined();
		Makai::File::saveText("cache.flow", proj.toFLOWString("\t"));
		DEBUGLN("Done!");
	}

	static void doUpdate(Makai::Data::Value& cfg) {
		auto const instPath = Makai::OS::FS::sourceLocation();
		if (cfg.fetch("mode").getString() == "clean") {
			Makai::Thread::wait<>(1000);
			DEBUGLN("Cleaning up...");
			Makai::OS::FS::remove(instPath + "/tmp");
			DEBUGLN("Done!");
		} else if (cfg.fetch("mode").getString() == "unpack") {
			Makai::Thread::wait<>(1000);
			DEBUGLN("Installing...");
			auto const upPath = cfg.fetch("__args")[1].getString();
			Makai::OS::FS::copy(upPath, instPath);
			Makai::OS::launchAsync(
				upPath
			+	"concerto"
			#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
			+	".exe"
			#endif
				,"",
				Makai::StringList::from("update", "--mode", "clean")
			);
			DEBUGLN("Done!");
		} else {
			DEBUGLN("Updating...");
			auto const meta = Makai::File::getFLOW(instPath + "/config/meta.flow");
			auto const update = Makai::FLOW::parse(Makai::Net::File::fetchText(meta.fetch("source").getString("")));
			auto content = update.fetch("content").getBytes();
			Makai::MemoryBuffer buf{Makai::Cast::rewrite<ref<char>>(content.data()), content.size()};
			Makai::Tool::Arch::FileArchive fa{
				buf,
				Makai::Tool::Arch::hashPassword(
					update.fetch("key").getString() + "/"
				+	update.fetch("version").getString() + "/"
				+	meta.fetch("key").getString()
				)
			};
			fa.unpackTo(instPath + "/tmp/" + update.fetch("version").getString().replace(':', '-'));
			Makai::OS::launchAsync(
				instPath
			+	"/tmp/"
			+	update.fetch("version").getString().replace(':', '-')
			+	"concerto"
			#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
			+	".exe"
			#endif
			,
			"",
			Makai::StringList::from("update", instPath, "--mode", "unpack")
			);
		}
		DEBUGLN("Done!");
	}
}

int main(int argc, char** argv) try {
	if (CTL::CPP::Debug::hasDebugger())
		CTL::CPP::Debug::Traceable::trap = true;
	Makai::CLI::Parser cli(argc, argv);
	translationBase(cli.tl);
	auto cfg = cli.parse(configBase());
	if (cfg["help"])
		Command::doHelpMessage();
	else {
		if (cfg["__args"].empty())
			throw Makai::Error::NonexistentValue("Missing command!");
		auto const command = cfg["__args"][0].get<Makai::String>();
		if		(command == "build"		)	Command::doBuild(cfg);
		else if	(command == "create"	)	Command::doCreate(cfg);
		else if	(command == "refresh"	)	Command::doRefresh(cfg);
		else if	(command == "add"		)	Command::doAdd(cfg);
		else if	(command == "remove"	)	Command::doRemove(cfg);
		else if	(command == "source"	)	Command::doSource(cfg);
		else if	(command == "update"	)	Command::doUpdate(cfg);
		else if	(command == "init"		)	Command::doInitialize(cfg);
		else throw Makai::Error::InvalidValue("Invalid command [" + command + "]!");
	}
	return 0;
} catch (Makai::Error::Generic const& e) {
	DEBUGLN(e.report());
	return -1;
} catch (Makai::Exception const& e) {
	DEBUGLN(e.what());
	return -1;
}
