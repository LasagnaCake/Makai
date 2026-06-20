#define MAKAILIB_MAIN_NO_POPUPS
#include <makai/makai.hpp>
#include <makai/main.hpp>
#include <iostream>
#include "base.cc"

using namespace Makai;

constexpr auto const VER = Makai::Data::Version{1};

struct FLOW2JSONMain: AMain {
	static Makai::Data::Value configBase() {
		Makai::Data::Value cfg;
		cfg["help"]	= false;
		return cfg;
	}

	static void translationBase(Makai::CLI::Parser::Translation& tl) {
		tl["H"]	= "help";
		tl["f"]	= "file";
		tl["o"]	= "out";
	}

	FLOW2JSONMain(Makai::CLI::Parser& cli): AMain(cli) {
		translationBase(cli.tl);
		baseArgs = configBase();
		showDialogOnError = false;
	}

	void run(Makai::Data::Value const& args) override {
		if (args.fetch("help", false)) {
			writeLine("FLOW2JSON - V" + VER.serialize().get<Makai::String>());
			writeLine("Available commands:");
			writeLine("flow2json (<source> OR -f <source-path>) [-o <out-path>]");
		} else {
			String parse;
			if (args.contains("file")) {
				parse = Makai::File::getText(args["file"].getString());
			} else if (args["__args"].size() < 1)
				throw Error::FailedAction("Expected file content to follow 'flow2json'!");
			else parse = args["__args"][0].getString();
			parse = Makai::FLOW::parse(parse).toJSONString();
			if (args.contains("out"))
				Makai::File::saveText(args["out"].getString(), parse);
			else std::cout << parse;
		}
	}
};

Makai_bindMain(FLOW2JSONMain)
