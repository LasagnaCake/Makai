#ifndef MAKAILIB_FILE_XML_H
#define MAKAILIB_FILE_XML_H

#include "../compat/ctl.hpp"
#include "json.hpp"

/// @brief XML-related facilities.
namespace Makai::XML {
	/// @brief Converts an XML string to a JSON value.
	/// @param xml String to convert.
	/// @return String as JSON value.
	/// @throw Error::FailedAction at XML and JSON parsing failures.
	JSON::JSONValue toJSON(String const& xml);
}

namespace MkXML = Makai::XML;

#endif