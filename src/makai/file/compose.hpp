#ifndef MAKAILIB_FILE_COMPOSE_H
#define MAKAILIB_FILE_COMPOSE_H

#include "get.hpp"

namespace Makai::File {
	/// @brief Include directive macros.
	namespace Directive {
		/// @brief C-style (C & C++) include directives.
		inline static String const C_CPP	= R"=((?:#include ["<]).*?(?:[">]))=";
		/// @brief Anima Version 1 include directives.
		inline static String const ANIMA_V1	= R"=((?:/append ["]).*?(?:["]))=";
	}

	/// @brief Composes a file.
	/// @param source File source.
	/// @param directive Include directive to parse for. Must be a regex match. By default, it is `Directive::C_CPP`.
	/// @note Behaves like `#include` in C/C++ - simply appends the file's contents at the given directive's position.
	String compose(String const& source, String const& directive = Directive::C_CPP);
}

#endif // MAKAILIB_FILE_H
