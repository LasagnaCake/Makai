#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct StringManipLib: ILibrary {

	static String findFirst(String const& str, String const& expr) {
		return Regex::findFirst(str, expr).match;
	}

	static ssize firstIndexOf(String const& str, String const& expr) {
		return Regex::findFirst(str, expr).position;
	}

	void load(Context::TypeAdder const& types, Context::MethodAdder const& methods) {
		methods.add("av2/string/replace", Regex::replace);
		methods.add("av2/string/contains", Regex::contains);
		methods.add("av2/string/matches", Regex::matches);
		methods.add("av2/string/count", Regex::count);
		methods.add("av2/string/findFirst", findFirst);
		methods.add("av2/string/firstIndexOf", firstIndexOf);
	}
};

AV2_Library(StringManipLib);
