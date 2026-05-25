#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

#define doWrite(WHAT) std::cout << WHAT
#define doWriteLine(WHAT) std::cout << WHAT << "\n"

struct ConsoleLib: ALibrary {
	static void write_string(UTF8String str) {
		doWrite(__LINE__);
		doWrite("/");
		doWrite(str.size());
		doWriteLine(" ::: Writing to standard output...");
		doWrite(str);
	}

	static void writeLine_string(UTF8String str) {
		doWrite(__LINE__);
		doWrite("/");
		doWrite(str.size());
		doWriteLine(" ::: Writing to standard output...");
		doWriteLine(str);
	}

	static UTF8String toString(Makai::Data::Value val) {
		if (val.isUndefined()) return "";
		if (val.isString()) return val.getString();
		return val.toFLOWString();
	}

	static void write_any(Makai::Anima::V2::Core::Any what) {
		doWrite(__LINE__);
		doWriteLine(" ::: Writing to standard output...");
		if (what.value)
			write_string(toString(what.value->toDynamicValue()));
	}

	static void writeLine_any(Makai::Anima::V2::Core::Any what) {
		doWrite(__LINE__);
		doWriteLine(" ::: Writing to standard output...");
		if (what.value)
			writeLine_string(toString(what.value->toDynamicValue()));
	}

	void load(Context::Adder const& context) override {
		context.methods.add("av2/console/write_string", 	write_string		);
		context.methods.add("av2/console/write_any",		write_any			);
		context.methods.add("av2/console/writeLine_string",	writeLine_string	);
		context.methods.add("av2/console/writeLine_any",	writeLine_any		);
	}

	String name() const override {return "av2/console";}
};

AV2_Library(ConsoleLib);
