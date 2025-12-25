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
	}
	return 0;
}
