#include "flow.hpp"

#include "../parser/parser.hpp"

Makai::FLOW::Value Makai::FLOW::parse(String const& flow) {
	return Data::parse<Parser::Data::FLOWParser>(flow);
}

Makai::FLOW::Value Makai::FLOW::loadFile(String const& path) {
	return Makai::FLOW::parse(Makai::File::loadText(path));
}

Makai::JSON::Value Makai::FLOW::getFile(String const& path) {
	return Makai::FLOW::parse(Makai::File::getText(path));
}
