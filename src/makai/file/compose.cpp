#include "compose.hpp"

using namespace Makai::File;

Makai::String Makai::File::compose(Makai::String const& source, Makai::String const& directive) {
	Makai::String result = source;
	auto includes = Makai::Regex::find(source, directive);
	if (includes.empty()) return result;
	for (auto include: includes)
		result = Makai::Regex::replace(
			result,
			directive,
			compose(Makai::File::getText(include.match), directive)
		);
	return result;
}
