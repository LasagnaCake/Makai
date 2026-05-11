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

struct ConcertoMain: Makai::AMain {
	static Makai::Data::Value configBase() {
		Makai::Data::Value cfg;
		cfg["help"]	= false;
		return cfg;
	}

	static void translationBase(Makai::CLI::Parser::Translation& tl) {
		tl["H"]	= "help";
	}

	ConcertoMain(Makai::CLI::Parser& cli): AMain(cli) {
		translationBase(cli.tl);
		baseArgs = configBase();
		showDialogOnError = false;
	}


	void write(Makai::String const& what) const override {DEBUGLN(what);}

	void run(Makai::Data::Value const& args) override {
		if (args.fetch("help", false)) {
			writeLine("Concerto - V" + VER.serialize().get<Makai::String>());
			writeLine("Available commands:");
			writeLine("concerto <action>");
		} else {
		}
	}
};

Makai_bindMain(ConcertoMain)

// TODO: This (again)
