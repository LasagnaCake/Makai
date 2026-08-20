#ifndef MAKAILIB_REGEX_ENGINE_H
#define MAKAILIB_REGEX_ENGINE_H

#include "../compat/ctl.hpp"

namespace Makai::Regex {
	struct Engine {
		struct Flags {
			bool ignoreCase = false;
			bool lineByLine = false;
		};

		template <class T>
		struct Match;

		template <> struct Match<String>: CTL::Regex::Match {};

		template <Type::OneOf<UTF8String, UTF32String> T>
		struct Match<T> {
			/// @brief Match position.
			ssize	position;
			/// @brief Match contents.
			T		match;
		};

		template <class T>
		using Matches = List<Match<T>>;

		struct Impl;

		Engine();

		Engine(String const& regex, Flags const& flags);
		Engine(UTF8String const& regex, Flags const& flags);
		Engine(UTF32String const& regex, Flags const& flags);

		Engine(Engine const&);
		Engine(Engine&&);

		Engine& operator=(Engine const&);
		Engine& operator=(Engine&&);

		Matches<String>			matchIn(String const& str, bool const fullMatch = false) const;
		Matches<UTF8String>		matchIn(UTF8String const& str, bool const fullMatch = false) const;
		Matches<UTF32String>	matchIn(UTF32String const& str, bool const fullMatch = false) const;

		String					replaceIn(String const& str, String const& rep) const;
		UTF8String				replaceIn(UTF8String const& str, UTF8String const& rep) const;
		UTF32String				replaceIn(UTF32String const& str, UTF32String const& rep) const;

	private:
		AtomicCell<Impl> impl;
	};
}

#endif
