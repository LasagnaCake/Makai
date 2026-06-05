#include <makai/makai.hpp>
#include <makai/main.hpp>

#include "base.cc"

#define doWrite(WHAT) std::cout << WHAT
#define doWriteLine(WHAT) std::cout << WHAT << "\n"

constexpr auto const VER = Makai::Data::Version{1};

DEFINE_ERROR_TYPE_EX(EngineError, FailedAction);

struct ARTE: Makai::Anima::V2::Runtime::Engine {
	bool cliEnabled = false;

	static void AV2Call write_string(Makai::String str) {
		Makai::CPP::Debug::breakpoint();
		doWrite(str);
	}

	static void AV2Call writeLine_string(Makai::String str) {
		Makai::CPP::Debug::breakpoint();
		doWriteLine(str);
	}

	static Makai::String AV2Call toString(Makai::Data::Value val) {
		if (val.isUndefined()) return "";
		if (val.isString()) return val.getString();
		return val.toFLOWString();
	}

	static void AV2Call write_any(Makai::Anima::V2::Core::Any what) {
		if (what.value)
			write_string(toString(what.value->toDynamicValue()));
	}

	static void AV2Call writeLine_any(Makai::Anima::V2::Core::Any what) {
		if (what.value)
			writeLine_string(toString(what.value->toDynamicValue()));
	}

	ARTE(
		bool const allowDynlibs	= false,
		bool const cliEnabled	= false
	): Engine(Config{allowDynlibs}), cliEnabled(cliEnabled) {
	}

	void onLoad() override {
		if (!config.allowDynamicLibraries && cliEnabled) {
			context.art.addExternalMethod("av2/console/write_string", 		write_string		);
			context.art.addExternalMethod("av2/console/write_any",			write_any			);
			context.art.addExternalMethod("av2/console/writeLine_string",	writeLine_string	);
			context.art.addExternalMethod("av2/console/writeLine_any",		writeLine_any		);
		}
	}
};

struct ARTEMain: Makai::AMain {
	static Makai::Data::Value configBase() {
		Makai::Data::Value cfg;
		cfg["help"]				= false;
		cfg["cli"]				= false;
		cfg["allow-dynlibs"]	= false;
		return cfg;
	}

	static void translationBase(Makai::CLI::Parser::Translation& tl) {
		tl["H"]		= "help";
		tl["C"]		= "cli";
		tl["DL"]	= "allow-dynlibs";
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
			writeLine("art <program> [-C] [-DL]");
		} else {
			ARTE engine{
				args["allow-dynlibs"].getBoolean(),
				args["cli"].getBoolean()
			};
			engine.load(Makai::File::getFLOW(args["__args"][0].getString() + ".anp"));
			engine.execute();
			while (engine.process()) {
				DEBUGLN("Frame!");
			}
			engine.error().then(handleError);
		}
	}
};

Makai_bindMain(ARTEMain);
