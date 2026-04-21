#include <makai/makai.hpp>
#include <makai/main.hpp>

#include "base.cc"

constexpr auto const VER = Makai::Data::Version{1};

namespace ART {
	static void write(Makai::UTF8String const& str) {
		std::cout << str;
	}

	static void writeLine(Makai::UTF8String const& str) {
		std::cout << str << "\n";
	}
}

struct ARTE: Makai::Anima::V2::Runtime::Engine {
	ARTE() {
		context.art.addExternalMethod("art/core/io/write", ART::write);
		context.art.addExternalMethod("art/core/io/writeLine", ART::writeLine);
	}
};

struct ARTEMain: Makai::AMain {
	static Makai::Data::Value configBase() {
		Makai::Data::Value cfg;
		cfg["help"]		= false;
		return cfg;
	}

	static void translationBase(Makai::CLI::Parser::Translation& tl) {
		tl["H"]		= "help";
	}

	ARTEMain(Makai::CLI::Parser& cli): AMain(cli) {
		translationBase(cli.tl);
		baseArgs = configBase();
		showDialogOnError = false;
	}

	void write(Makai::String const& what) const override {DEBUG(what);}

	void run(Makai::Data::Value const& args) override {
		ARTE engine;
		if (args.fetch("help", false)) {
			writeLine("Anima RunTime - V" + VER.serialize().get<Makai::String>());
			writeLine("Available commands:");
			writeLine("art <program> [-N]");
		} else {
			engine.load(Makai::File::getFLOW(args["__args"][0].getString() + ".anp"));
			engine.execute();
			while (engine.process()) {
				writeLine("Frame!");
			};
			engine.error().then([&] (auto const& e) {
				writeLine("!!! ERROR !!!");
				writeLine("At bytecode offset ", e.location);
				writeLine("At instruction ", Makai::Anima::V2::Core::Instruction::asString(e.instruction.name));
				writeLine("Message: [", e.message, "]");
				return e;
			});
		}
	}
};

Makai_bindMain(ARTEMain);
