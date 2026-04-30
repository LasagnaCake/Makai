#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct StringLib: ILibrary {
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

	void load(Context::TypeAdder const& types, Context::MethodAdder const& methods) {
		methods.add("av2/string/replace", replace);
		methods.add("av2/string/contains", contains);
		methods.add("av2/string/matches", matches);
		methods.add("av2/string/count", count);
		methods.add("av2/string/findFirst", findFirst);
		methods.add("av2/string/firstIndexOf", firstIndexOf);
		methods.add("av2/string/find", find);
	}
};

AV2_Library(StringLib);
