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
	/// @note
	///		Tags are converted to JSON objects, where its accessor key is the tag name.
	///		
	///		All tag attributes are prefixed with an `@` in the resulting JSON, and contained within the tag object.
	///		
	///		Rogue text in a tag is located in the `.content` property.
	///		
	///		If there is only one tag of a given type, it results in a single object.
	///		If there is more than one tag of a given type, it results in an array of objects.
	JSON::JSONValue toJSON(String const& xml);
}

namespace MkXML = Makai::XML;

#endif