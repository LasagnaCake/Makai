#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct StringLib: ALibrary {
	static String AV2Call replace(String const& str, String const& expr, String const& fmt) {
		return Makai::Regex::replace(str, expr, fmt);
	}

	static bool AV2Call contains(String const& str, String const& expr) {
		return Makai::Regex::contains(str, expr);
	}

	static bool AV2Call matches(String const& str, String const& expr) {
		return Makai::Regex::matches(str, expr);
	}

	static usize AV2Call count(String const& str, String const& expr) {
		return Makai::Regex::count(str, expr);
	}

	static String AV2Call findFirst(String const& str, String const& expr) {
		return Makai::Regex::findFirst(str, expr).match;
	}

	static ssize AV2Call firstIndexOf(String const& str, String const& expr) {
		return Makai::Regex::findFirst(str, expr).position;
	}

	static StringList AV2Call find(String const& str, String const& expr) {
		return Makai::Regex::find(str, expr).toList<String>([] (auto const& e) {return e.match;});
	}

	void load(Context::Adder const& context) override {
		context.methods.add("av2/string/replace",		replace			);
		context.methods.add("av2/string/contains",		contains		);
		context.methods.add("av2/string/matches",		matches			);
		context.methods.add("av2/string/count",			count			);
		context.methods.add("av2/string/findFirst",		findFirst		);
		context.methods.add("av2/string/firstIndexOf",	firstIndexOf	);
		context.methods.add("av2/string/find",			find			);
	}

	String name() const override {return "av2/string";}
};

AV2_Library(StringLib);
