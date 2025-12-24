#include <makai/makai.hpp>

static Makai::Data::Value configBase() {
	Makai::Data::Value cfg;
	cfg["help"]		= false;
	cfg["output"]	= "out.anp";
	return cfg;
}

static void translationBase(Makai::CLI::Parser::Translation& tl) {
	tl["H"] = "help";
	tl["o"] = "output";
}

int main(int argc, char** argv) {
	Makai::CLI::Parser cli(argc, ref<cstring>(argv));
	translationBase(cli.tl);
	auto cfg = cli.parse(configBase());
	return 0;
}
