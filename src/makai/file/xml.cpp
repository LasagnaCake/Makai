#include "xml.hpp"
#include "json.hpp"

#include <exception>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
// SHUT UUUUUUUUUUUUUUP
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Woverflow"

#define XML2JSON_CONTENT_PROP		".content"
#define XML2JSON_ATTRIB_PREFIX		"@" 
#define XML2JSON_ACCEPTS_NUMBERS	true
#include <xml2json/xml2json.hpp>

#pragma GCC diagnostic pop

Makai::JSON::JSONValue Makai::XML::toJSON(Makai::String const& xml) {
	Makai::String json;
	try {
		json = xml2json(xml.cstr());
	} catch (std::exception const& e) {
		throw Error::FailedAction(
			"Failed at converting XML file!",
			e.what(),
			CTL_CPP_PRETTY_SOURCE
		);
	}
	return Makai::JSON::parse(json);
}