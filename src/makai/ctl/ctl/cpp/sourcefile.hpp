#ifndef CTL_CPP_SOURCEFILE_H
#define CTL_CPP_SOURCEFILE_H

#include "../namespace.hpp"

#include "../container/string.hpp"
#include "../algorithm/strconv.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Facilites pertaining to C++ source files.
namespace CPP {
	/// @brief Source file information.
	struct SourceFile {
		/// @brief Constructs the information structure. No values are required to be passed.
		/// @param function Current function.
		/// @param line Current file.
		/// @param file Current line.
		constexpr SourceFile(
			String const&	function	= __builtin_FUNCTION(),
			int const		line		= __builtin_LINE(),
			String const&	file		= __builtin_FILE()
		): function(function), file(file), line(line) {}

		/// @brief Returns the file number as a string. If line is 0 or less, returns `"unspecified"`.
		/// @return File number as string.	
		String lineName() const {
			if (line > 1) return toString(line);
			return "unspecified";
		}

		/// @brief Current function.
		String const	function;
		/// @brief Current file.
		String const	file;
		/// @brief Current line.
		int const		line;
	};
}

CTL_NAMESPACE_END

/// @brief Basic source information.
#define CTL_CPP_BASIC_SOURCE (::CTL::CPP::SourceFile())
/// @brief Source information with pretty function name.
#define CTL_CPP_PRETTY_SOURCE (::CTL::CPP::SourceFile(__PRETTY_FUNCTION__))
/// @brief Unknown source.
#define CTL_CPP_UNKNOWN_SOURCE (::CTL::CPP::SourceFile("unknown", -1))

/// @brief Default source.
#define CTL_CPP_DEFAULT_SOURCE (CTL_CPP_BASIC_SOURCE)

#endif