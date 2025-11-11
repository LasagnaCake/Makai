#ifndef MAKAILIB_FILE_XML_H
#define MAKAILIB_FILE_XML_H

#include "../compat/ctl.hpp"
#include "json.hpp"

/// @brief XML-related facilities.
namespace Makai::XML {
	/// @brief Converts an XML string to a value.
	/// @param xml String to convert.
	/// @return String as value.
	/// @throw Error::FailedAction at XML and JSON parsing failures.
	/// @note String is converted to a JSON string, then to a value.
	///		Tags are converted to JSON objects, where its accessor key is the tag name.
	///		
	///		All tag attributes are prefixed with an `@` in the resulting JSON, and contained within the tag object.
	///		
	///		Rogue text in a tag is located in the `.content` property.
	///		
	///		If there is only one tag of a given type, it results in a single object.
	///		If there is more than one tag of a given type, it results in an array of objects.
	JSON::Value			toValue(String const& xml);
	inline JSON::Value	toJSON(String const& xml)			{return toValue(xml);		}

	/// @brief Converts a value to an XML string.
	/// @param value Value to convert.
	/// @return String as XML value.
	/// @throw Error::FailedAction at XML and JSON parsing failures.
	/// @note Value is converted to a JSON string, then to an XML one.
	///		Does not support the rogue text `.content` property!
	String			fromValue(JSON::Value const& value);
	inline String	fromJSON(JSON::Value const& value)		{return fromValue(value);	}

	
	inline JSON::Value	fromXML(String const& xml)			{return toJSON(xml);		}
	inline String		toXML(JSON::Value const& json)		{return fromJSON(json);		}
}

namespace MkXML = Makai::XML;

#endif