#ifndef MAKAILIB_NET_URL_H
#define MAKAILIB_NET_URL_H

#include "../compat/ctl.hpp"

namespace Makai::Net::URI {
	constexpr String normalize(String const& str) {
		return Regex::replace(str, R"(\\\\)", "/")
			.split('/')
			.filter([] (String const& part) {
				return (
					part.isNullOrSpaces()
				or	part == "."
				or	part == ".."
				);
			})
			.join('/')
		;
	}
	constexpr String decode(String const& str) {
		bool first = true;
		return
			str
				.split('%')
				.transform([&first] (String const& str) {
					return first
						? (first = false, str)
						: str
							.sliced(2)
							.insert(toInt8(str.sliced(0, 1), 16), 0)
					;
				})
				.join("")
		;
	}
}

#endif
