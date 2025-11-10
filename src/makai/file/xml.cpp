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
#include <json2xml.hpp>

#pragma GCC diagnostic pop

Makai::JSON::Value Makai::XML::toValue(Makai::String const& xml) {
	Makai::String json;
	try {
		json = xml2json(xml.cstr());
	} catch (std::exception const& e) {
		throw Error::FailedAction(
			"Failed at converting XML to JSON!",
			e.what(),
			CTL_CPP_PRETTY_SOURCE
		);
	}
	return Makai::JSON::parse(json);
}


Makai::String Makai::XML::fromValue(Makai::JSON::Value const& xml) {
	try {
		ert::JsonSaxConsumer consumer(4, XML2JSON_ATTRIB_PREFIX[0]);
		bool success = nlohmann::json::sax_parse(xml.toJSONString().cstr(), &consumer);
		if (!success)
			throw Error::FailedAction(
				"Failed at converting JSON to XML!",
				CTL_CPP_PRETTY_SOURCE
			);
		return consumer.getXmlString();
	} catch (std::exception const& e) {
		throw Error::FailedAction(
			"Failed at converting XML file!",
			e.what(),
			CTL_CPP_PRETTY_SOURCE
		);
	}
}