#include <makai/makai.hpp>

using namespace Makai::Anima::V2;

using namespace Toolchain;

auto const projectDatabase() {
	auto const path = Makai::OS::FS::sourceLocation() + "sources/db.flow";
	Makai::Data::Value db;
	if (Makai::OS::FS::exists(path))
		db = Makai::File::getFLOW(path);
	if (!Makai::OS::FS::exists(path))
		Makai::File::saveText(path, db.toFLOWString("\t"));
	return db;
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
	cfg["ir"]		= false;
	cfg["name"]		= "project";
	cfg["type"]		= "program";
	cfg["lang"]		= "breve";
	return cfg;
}

static void translationBase(Makai::CLI::Parser::Translation& tl) {
	tl["H"] = "help";
	tl["I"] = "ir";
	tl["o"] = "output";
	tl["n"] = "name";
	tl["t"] = "type";
	tl["l"] = "lang";
}

static Compiler::Project::File::Type getFileType(Makai::String const& name) {
	if (name == "breve" || name == "bv")	return Compiler::Project::File::Type::AV2_TC_PFT_BREVE;
	if (name == "minima" || name == "min")	return Compiler::Project::File::Type::AV2_TC_PFT_MINIMA;
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
	static void doBuild(Makai::Data::Value& cfg) {
		DEBUGLN("Building project...");
		Compiler::Project proj;
		Assembler::Context ctx;
		proj = proj.deserialize(Makai::File::getFLOW("project.flow"));
		if (proj.type == decltype(proj.type)::AV2_TC_PT_MODULE)
			return;
		if (proj.main.source.empty() && proj.main.path.size())
			proj.main.source = Makai::File::getText(proj.main.path);
		Compiler::buildProject(ctx, proj, cfg["ir"]);
		auto const outName = Makai::Regex::replace(cfg["output"], "${name}", proj.name);
		Makai::OS::FS::makeDirectory(Makai::String("output"));
		if (cfg["ir"])
			Makai::File::saveText("output/" + outName + ".min", ctx.compose());
		else Makai::File::saveText("output/" + outName + ".anp", ctx.program.serialize().toFLOWString("\t"));
		DEBUGLN("Done!");
	}

	static void doCreate(Makai::Data::Value& cfg) {
		DEBUGLN("Creating project...");
		Makai::Data::Value projBase = Compiler::Project();
		projBase["type"] = cfg["type"];
		auto proj = Compiler::Project::deserialize(projBase);
		proj.name = cfg["name"].get<Makai::String>();
		if (Makai::OS::FS::exists(proj.name))
			throw Makai::Error::FailedAction("Project '"+proj.name+"' already exists in this folder!");
		if (proj.type == decltype(proj.type)::AV2_TC_PT_EXECUTABLE)
			throw Makai::Error::FailedAction("Standalone executable projects are curently unimplemented, sorry :/");
		Makai::OS::FS::makeDirectory(proj.name);
		proj.package = {0, 0, 1};
		proj.main.type = getFileType(cfg["lang"]);
		proj.main.path = "src/main." + getFileExtension(proj.main.type);
		proj.sources.pushBack("src");
		proj.sources.pushBack(Makai::OS::FS::sourceLocation() + "/breve/lib");
		if (proj.type != decltype(proj.type)::AV2_TC_PT_MODULE)
			Makai::File::saveText(proj.name + "/" + proj.main.path, "import core.all;\n\nmain {\n\t// Main code goes here...\n}");
		else Makai::File::saveText(proj.name + "/all.bv", "// Full imports goes here...");
		Makai::File::saveText(proj.name + "/project.flow", proj.serialize().toFLOWString("\t"));
		Makai::File::saveText(proj.name + "/.gitignore", "output/\nmodule/\ncache.flow\n*.anp");
		DEBUGLN("Done!");
	}

	static void doRefresh(Makai::Data::Value& cfg) {
		DEBUGLN("Refreshing project...");
		Makai::OS::FS::remove("cache.flow", "modules");
		Compiler::Project proj;
		Assembler::Context ctx;
		proj = proj.deserialize(Makai::File::getFLOW("project.flow"));
		Compiler::downloadProjectModules(ctx, proj);
		DEBUGLN("Done!");
	}
}

int main(int argc, char** argv) try {
	Compiler::setModuleSourceResolver(resolveSource);
	Makai::CLI::Parser cli(argc, ref<cstring>(argv));
	translationBase(cli.tl);
	auto cfg = cli.parse(configBase());
	if (cfg["help"]) {
	} else {
		if (cfg["__args"].empty())
			throw Makai::Error::NonexistentValue("Missing command!");
		auto const command = cfg["__args"][0].get<Makai::String>();
		if (command == "build")			Command::doBuild(cfg);
		else if (command == "create")	Command::doCreate(cfg);
		else if (command == "refresh")	Command::doRefresh(cfg);
	}
	return 0;
} catch (Makai::Error::Generic const& e) {
	DEBUGLN(e.report());
	return -1;
} catch (Makai::Exception const& e) {
	DEBUGLN(e.what());
	return -1;
}
