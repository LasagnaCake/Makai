#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct StringLib: ALibrary {
	static String replace(String str, String expr, String fmt) {
		return Regex::replace(str, expr, fmt);
	}

	static bool contains(String str, String expr) {
		return Regex::contains(str, expr);
	}

	static bool matches(String str, String expr) {
		return Regex::matches(str, expr);
	}

	static usize count(String str, String expr) {
		return Regex::count(str, expr);
	}

	static String findFirst(String str, String expr) {
		return Regex::findFirst(str, expr).match;
	}

	static ssize firstIndexOf(String str, String expr) {
		return Regex::findFirst(str, expr).position;
	}

	static StringList find(String str, String expr) {
		return Regex::find(str, expr).toList<String>([] (auto const& e) {return e.match;});
	}

	void load(Context::Adder const& context) override {
		context.methods.add("av2/string/replace", replace);
		context.methods.add("av2/string/contains", contains);
		context.methods.add("av2/string/matches", matches);
		context.methods.add("av2/string/count", count);
		context.methods.add("av2/string/findFirst", findFirst);
		context.methods.add("av2/string/firstIndexOf", firstIndexOf);
		context.methods.add("av2/string/find", find);
	}

	String name() const override {return "av2/string";}
};

AV2_Library(StringLib);
