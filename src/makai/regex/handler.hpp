#ifndef MAKAILIB_REGEX_HANDLER_H
#define MAKAILIB_REGEX_HANDLER_H

#include "../compat/ctl.hpp"

namespace Makai::Regex {
	struct PCRE2Handler: IHandler {
		PCRE2Handler();

		virtual ~PCRE2Handler();

		String replace(String const& str, String const& expr, String const& fmt) override;

		bool contains(String const& str, String const& expr) override;

		bool matches(String const& str, String const& expr) override;

		usize count(String const& str, String const& expr) override;

		List<Match> find(String const& str, String const& expr) override;

		Nullable<Match> findFirst(String const& str, String const& expr) override;

	private:
		//static PCRE2Handler& handler;
	};
}

#endif
