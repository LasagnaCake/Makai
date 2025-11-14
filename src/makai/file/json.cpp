#include "json.hpp"

#include "../parser/parser.hpp"

Makai::JSON::Value Makai::JSON::parse(String const& json) {
	return Data::parse<Parser::Data::JSONParser>(json);
}

Makai::JSON::Value Makai::JSON::loadFile(String const& path) {
	return Makai::JSON::parse(Makai::File::loadText(path));
}

Makai::JSON::Value Makai::JSON::getFile(String const& path) {
	return Makai::JSON::parse(Makai::File::getText(path));
}
