#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

#define doWrite(WHAT) std::cout << WHAT
#define doWriteLine(WHAT) std::cout << WHAT << "\n"

struct ConsoleLib: ILibrary {
	static void write_string(UTF8String str) {
		doWrite(str);
	}

	static void writeLine_string(UTF8String str) {
		doWriteLine(str);
	}

	static UTF8String toString(Makai::Data::Value val) {
	}

	static void write_any(Makai::Anima::V2::Core::Any what) {
		if (what.value)
			write_string(toString(what.value->toDynamicValue()));
	}

	static void writeLine_any(Makai::Anima::V2::Core::Any what) {
		if (what.value)
			writeLine_string(toString(what.value->toDynamicValue()));
	}

	void load(Context::MethodAdder const& context) {
		context.add("av2/console/write_string", write_string);
		context.add("av2/console/write_any", write_any);
		context.add("av2/console/writeLine_string", writeLine_string);
		context.add("av2/console/writeLine_any", writeLine_any);
	}
};

AV2_Library(ConsoleLib);
