#define MAKAILIB_MAIN_NO_POPUPS
#include <makai/makai.hpp>
#include <makai/main.hpp>
#include <iostream>
#include "base.cc"

using namespace Makai;

constexpr auto const VER = Makai::Data::Version{1};

enum class FileType {
	FT_FLOW,
	FT_JSON,
	FT_XML
};

static FileType fromString(String const& str) {
	if (str == "flow")	return FileType::FT_FLOW;
	if (str == "json")	return FileType::FT_JSON;
	if (str == "xml")	return FileType::FT_XML;
	throw Makai::Error::InvalidValue("Invalid file type '" + str + "'!");
}

struct Data2DataMain: AMain {
	static Makai::Data::Value configBase() {
		Makai::Data::Value cfg;
		cfg["help"]	= false;
		cfg["from"]	= "flow";
		cfg["to"]	= "json";
		return cfg;
	}

	static void translationBase(Makai::CLI::Parser::Translation& tl) {
		tl["H"]		= "help";
		tl["f"]		= "file";
		tl["o"]		= "out";
		tl["in"]	= "from";
	}

	Data2DataMain(Makai::CLI::Parser& cli): AMain(cli) {
		translationBase(cli.tl);
		baseArgs = configBase();
		showDialogOnError = false;
	}

	static String convert(String const& src, FileType const from, FileType const to) {
		if (from == to) return src;
		switch (from) {
			case FileType::FT_JSON: {
				switch (to) {
					case FileType::FT_JSON: return src;
					case FileType::FT_FLOW: return JSON::parse(src).toFLOWString();
					case FileType::FT_XML: return XML::toXML(JSON::parse(src));
				}
			}
			case FileType::FT_FLOW: {
				switch (to) {
					case FileType::FT_JSON: return FLOW::parse(src).toJSONString();
					case FileType::FT_FLOW: return src;
					case FileType::FT_XML: return XML::toXML(FLOW::parse(src));
				}
			}
			case FileType::FT_XML: {
				switch (to) {
					case FileType::FT_JSON: return XML::fromXML(src).toJSONString();
					case FileType::FT_FLOW: return XML::fromXML(src).toFLOWString();
					case FileType::FT_XML: return src;
				}
			}
		}
		throw Makai::Error::InvalidValue("Failed to parse file!");
	}

	void run(Makai::Data::Value const& args) override {
		if (args.fetch("help", false)) {
			writeLine("Data2Data - V" + VER.serialize().get<Makai::String>());
			writeLine("Available commands:");
			writeLine("d2d (<source> OR -f <source-path>) [-o <out-path>] [-in <src-type>] [-to dst-type]");
		} else {
			String parse;
			if (args.contains("file")) {
				parse = Makai::File::getText(args["file"].getString());
			} else if (args["__args"].size() < 1)
				throw Error::FailedAction("Expected file content to follow 'd2d'!");
			else parse = args["__args"][0].getString();
			auto const srcType = fromString(args["from"].getString());
			auto const destType = fromString(args["to"].getString());
			if (srcType != destType)
				parse = convert(parse, srcType, destType);
			if (args.contains("out"))
				Makai::File::saveText(args["out"].getString(), parse);
			else std::cout << parse;
		}
	}
};

Makai_bindMain(Data2DataMain)
