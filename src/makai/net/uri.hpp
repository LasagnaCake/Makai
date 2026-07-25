#ifndef MAKAILIB_NET_URL_H
#define MAKAILIB_NET_URL_H

#include "../compat/ctl.hpp"

namespace Makai::Net::URI {
	inline String normalize(String const& str) {
		return (
			Regex::replace(str, R"(\\\\)", "/")
			.split('/')
			| [] (StringList const& parts) {
				StringList out;
				for (auto& part: parts) {
					if (
						part.isNullOrSpaces()
					or	part == "."
					) continue;
					if (part == "..") {
						if (out.size())
							out.popBack();
						continue;
					}
					out.pushBack(part);
				}
				return out;
			}
			).join('/')
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

	constexpr String encode(String const& str) {
		String out;
		for (auto& c: str)
			out += "%" + Format::pad(
				String::fromNumber<int8>(c, 16, false),
				'0',
				2,
				Format::Justify::CFJ_LEFT
			);
		return out;
	}
}

#endif
