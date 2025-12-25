#include <makai/makai.hpp>

static Makai::Data::Value configBase() {
	Makai::Data::Value cfg;
	cfg["help"]		= false;
	cfg["output"]	= "${name}";
	cfg["ir"]		= false;
	return cfg;
}

static void translationBase(Makai::CLI::Parser::Translation& tl) {
	tl["H"] = "help";
	tl["I"] = "ir";
	tl["o"] = "output";
}

int main(int argc, char** argv) {
	Makai::CLI::Parser cli(argc, ref<cstring>(argv));
	translationBase(cli.tl);
	auto cfg = cli.parse(configBase());
	if (cfg["help"]) {

	} else {
		Makai::Anima::V2::Toolchain::Compiler::Project proj;
		Makai::Anima::V2::Toolchain::Assembler::Context ctx;
		proj = proj.deserialize(Makai::File::getFLOW("project.flow"));
		Makai::Anima::V2::Toolchain::Compiler::buildProject(ctx, proj, cfg["ir"]);
		auto const outName = Makai::Regex::replace(cfg["output"], "${name}", proj.name);
		if (cfg["ir"]) {
			Makai::File::saveText(outName + ".min", ctx.compose());
		} else {
			Makai::File::saveText(outName + ".anp", ctx.program.serialize().toFLOWString());
		}
	}
	return 0;
}
