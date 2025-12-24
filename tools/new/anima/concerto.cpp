#include <makai/makai.hpp>

static Makai::Data::Value configBase() {
	Makai::Data::Value cfg;
	cfg["help"] = false;
	return cfg;
}

static void translationBase(Makai::CLI::Parser::Translation& tl) {
	tl["H"] = "help";
}

int main(int argc, char** argv) {
	Makai::CLI::Parser cli(argc, ref<cstring>(argv));
	auto cfg = cli.parse(configBase());
	translationBase(cli.tl);
	return 0;
}
