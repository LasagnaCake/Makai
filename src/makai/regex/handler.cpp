#include "handler.hpp"

using namespace Makai;
using namespace Regex;

String PCRE2Handler::replace(String const& str, String const& expr, String const& fmt) {
	Engine engine{expr, {}};
	return engine.replaceIn(str, fmt);
}

usize PCRE2Handler::count(String const& str, String const& expr) {
	Engine engine{expr, {}};
	return engine.matchIn(str).size();
}

bool PCRE2Handler::contains(String const& str, String const& expr) {
	Engine engine{expr, {}};
	return engine.matchIn(str).size();
}

bool PCRE2Handler::matches(String const& str, String const& expr) {
	Engine engine{expr, {}};
	return engine.matchIn(str, true).size();
}

bool PCRE2Handler::find(String const& str, String const& expr) {
	Engine engine{expr, {}};
	return engine.matchIn(str).toList<CTL::Regex::Match>();
}

bool PCRE2Handler::findFirst(String const& str, String const& expr) {
	Engine engine{expr, {}};
	auto const m = engine.matchIn(str);
	return m.size() ? m.front() : null;
}
