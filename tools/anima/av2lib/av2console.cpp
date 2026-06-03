#include <makai/makai.hpp>

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
		CTL::CPP::Debug::breakpoint();
		auto const b = copy(str).toBytes();
		CTL::CPP::Debug::breakpoint();
		auto const cont = Makai::Data::encode(b, Makai::Data::EncodingType::ET_BASE64);
		doWriteLine("<bytes data='" + cont + "' />");
		doWriteLine("<text>");
		doWrite(str);
		doWriteLine("</text>");
	}

	static void AV2Call writeLine_string(String str) {
		doWrite(__LINE__);
		doWrite("/");
		doWrite(str.size());
		doWriteLine(" ::: Writing to standard output...");
		CTL::CPP::Debug::breakpoint();
		auto const b = copy(str).toBytes();
		CTL::CPP::Debug::breakpoint();
		auto const cont = Makai::Data::encode(b, Makai::Data::EncodingType::ET_BASE64);
		doWriteLine("<bytes data='" + cont + "' />");
		doWriteLine("<text>");
		doWriteLine(str);
		doWriteLine("</text>");
	}

	static String AV2Call toString(Makai::Data::Value val) {
		doWrite(__LINE__);
		if (val.isUndefined()) return "";
		doWrite(__LINE__);
		if (val.isString()) return val.getString();
		doWrite(__LINE__);
		return val.toFLOWString();
	}

	static void AV2Call write_any(Makai::Anima::V2::Core::Any what) {
		doWrite(__LINE__);
		doWriteLine(" ::: Writing to standard output...");
		if (what.value)
			write_string(toString(what.value->toDynamicValue()));
	}

	static void AV2Call writeLine_any(Makai::Anima::V2::Core::Any what) {
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
