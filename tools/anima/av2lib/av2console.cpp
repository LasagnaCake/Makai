#include <makai/makai.hpp>
#include <iostream>

using namespace Makai;
using namespace Anima::V2::Core;

#define doWrite(WHAT) std::cout << WHAT
#define doWriteLine(WHAT) std::cout << WHAT << "\n"

struct ConsoleLib: ALibrary {
	static void AV2Call write_string(String str) {
		doWrite(__LINE__);
		doWrite("/");
		doWrite(str.size());
		doWriteLine(" ::: Writing to standard output...");
		doWrite(str);
	}

	static void AV2Call writeLine_string(String str) {
		doWrite(__LINE__);
		doWrite("/");
		doWrite(str.size());
		doWriteLine(" ::: Writing to standard output...");
		doWriteLine(str);
	}

	static String AV2Call toString(Makai::Data::Value val) {
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

	void load(Context::Adder const& context) override {
		context.methods.add("av2/console/write_string", 	write_string		);
		context.methods.add("av2/console/write_any",		write_any			);
		context.methods.add("av2/console/writeLine_string",	writeLine_string	);
		context.methods.add("av2/console/writeLine_any",	writeLine_any		);
	}

	String name() const override {return "av2/console";}
};

AV2_Library(ConsoleLib);
