#include <makai/makai.hpp>
#include <makai/main.hpp>

#include "base.cc"

#define doWrite(WHAT) std::cout << WHAT
#define doWriteLine(WHAT) std::cout << WHAT << "\n"

constexpr auto const VER = Makai::Data::Version{1};

DEFINE_ERROR_TYPE_EX(EngineError, FailedAction);

struct ARTE: Makai::Anima::V2::Runtime::Engine {
	bool cliEnabled = false;

	ARTE() {
	}
};

struct ARTEMain: Makai::AMain {
	static Makai::Data::Value configBase() {
		Makai::Data::Value cfg;
		cfg["help"]		= false;
		cfg["cli"]		= false;
		return cfg;
	}

	static void translationBase(Makai::CLI::Parser::Translation& tl) {
		tl["H"]		= "help";
		tl["C"]		= "cli";
	}

	ARTEMain(Makai::CLI::Parser& cli): AMain(cli) {
		translationBase(cli.tl);
		baseArgs = configBase();
		showDialogOnError = false;
	}

	void write(Makai::String const& what) const override {doWrite(what);}

	static ARTE::Error handleError(ARTE::Error const& e) {
		throw EngineError(Makai::toString(
			"!!! ERROR !!!",
			"\n", "At bytecode offset ", e.location,
			"\n", "At instruction ", Makai::Anima::V2::Core::Instruction::asString(e.instruction.name),
			"\n", "Message: [", e.message, "]"
		));
	}

	void run(Makai::Data::Value const& args) override {
		if (args.fetch("help", false)) {
			writeLine("Anima RunTime - V" + VER.serialize().get<Makai::String>());
			writeLine("Available commands:");
			writeLine("art <program> [-C]");
		} else {
			ARTE engine;
			engine.cliEnabled = args["cli"];
			{
				auto const f = Makai::Anima::V2::Core::Module::deserialize(Makai::File::getFLOW(args["__args"][0].getString() + ".anp"));
				engine.load(f);
			}
			engine.execute();
			while (engine.process()) {
				DEBUGLN("Frame!");
			}
			engine.error().then(handleError);
		}
	}
};

Makai_bindMain(ARTEMain);
