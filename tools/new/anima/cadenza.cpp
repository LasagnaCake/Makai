#include "makai/lang/anima/v2/toolchain/compiler/core.hpp"
#include <makai/makai.hpp>

using namespace Makai::Anima::V2;

using namespace Toolchain;

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

int main(int argc, char** argv) {
	Makai::CLI::Parser cli(argc, ref<cstring>(argv));
	translationBase(cli.tl);
	auto cfg = cli.parse(configBase());
	if (cfg["help"]) {
	} else {
		auto const command = cfg["__args"][0].get<Makai::String>();
		if (command == "build") {
			Compiler::Project proj;
			Assembler::Context ctx;
			proj = proj.deserialize(Makai::File::getFLOW("project.flow"));
			if (proj.main.source.empty() && proj.main.path.size())
				proj.main.source = Makai::File::getText(proj.main.path);
			Compiler::buildProject(ctx, proj, cfg["ir"]);
			auto const outName = Makai::Regex::replace(cfg["output"], "${name}", proj.name);
			Makai::OS::FS::makeDirectory(Makai::String("output"));
			if (cfg["ir"])
				Makai::File::saveText("output/" + outName + ".min", ctx.compose());
			else Makai::File::saveText("output/" + outName + ".anp", ctx.program.serialize().toFLOWString());
		} else if (command == "create") {
			Makai::Data::Value projBase = Compiler::Project();
			projBase["type"] = cfg["type"];
			auto proj = Compiler::Project::deserialize(projBase);
			proj.name = cfg["name"].get<Makai::String>();
			Makai::OS::FS::makeDirectory(proj.name);
			proj.main.type = getFileType(cfg["lang"]);
			proj.main.path = proj.name + "/src/main." + getFileExtension(proj.main.type);
			proj.sources.pushBack("src");
			Makai::File::saveText(proj.main.path, "import core;\n\nmain {\n\t// Main code goes here...\n}");
			Makai::File::saveText(proj.name + "project.flow", proj.serialize().toFLOWString());
		}
	}
	return 0;
}
