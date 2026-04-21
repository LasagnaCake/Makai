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

	template <class T>
	static Makai::UTF8String toString(T const& val) {
		if constexpr (Makai::Type::Equal<T, Makai::Anima::V2::Core::Any>)
			return Makai::UTF8String();
		else if constexpr (Makai::Type::Equal<T, Makai::UTF8Char>)
			return Makai::UTF8String(val);
		else return Makai::toString(val);
	}

	static void writeAny(Makai::Anima::V2::Core::Any const& what) {
		write(toString(what));
	}

	static void writeAnyLine(Makai::Anima::V2::Core::Any const& what) {
		writeLine(toString(what));
	}
}

struct ARTE: Makai::Anima::V2::Runtime::Engine {
	ARTE() {
		context.art.addExternalMethod("art/core/io/write_string", ART::write);
		context.art.addExternalMethod("art/core/io/write_any", ART::writeAny);
		context.art.addExternalMethod("art/core/io/writeLine_string", ART::writeLine);
		context.art.addExternalMethod("art/core/io/writeLine_any", ART::writeAnyLine);
		context.art.addExternalMethod("art/core/conv/toString_string", ART::toString<Makai::UTF8String>);
		context.art.addExternalMethod("art/core/conv/toString_int8", ART::toString<int8>);
		context.art.addExternalMethod("art/core/conv/toString_int16", ART::toString<int16>);
		context.art.addExternalMethod("art/core/conv/toString_int32", ART::toString<int32>);
		context.art.addExternalMethod("art/core/conv/toString_int64", ART::toString<int64>);
		context.art.addExternalMethod("art/core/conv/toString_uint8", ART::toString<uint8>);
		context.art.addExternalMethod("art/core/conv/toString_uint16", ART::toString<uint16>);
		context.art.addExternalMethod("art/core/conv/toString_uint32", ART::toString<uint32>);
		context.art.addExternalMethod("art/core/conv/toString_uint64", ART::toString<uint64>);
		context.art.addExternalMethod("art/core/conv/toString_float32", ART::toString<float32>);
		context.art.addExternalMethod("art/core/conv/toString_float64", ART::toString<float64>);
		context.art.addExternalMethod("art/core/conv/toString_float128", ART::toString<float128>);
		context.art.addExternalMethod("art/core/conv/toString_bool", ART::toString<bool>);
		context.art.addExternalMethod("art/core/conv/toString_char", ART::toString<Makai::UTF8Char>);
		context.art.addExternalMethod("art/core/conv/toString_any", ART::toString<Makai::Anima::V2::Core::Any>);
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
