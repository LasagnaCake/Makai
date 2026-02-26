#include <makai/makai.hpp>
#include <makai/main.hpp>

#include "base.cc"

constexpr auto const VER = Makai::Data::Version{1};

struct ARTE: Makai::Anima::V2::Runtime::Engine {
	bool httpRequestsEnabled = false;

	void onPrint(Makai::Data::Value const& value) override {
		#ifndef ARTE_NO_CLI
		if (value.isString())
			std::cout << value.getString();
		else std::cout << value.toFLOWString();
		#endif
	}

	Makai::Data::Value onHTTPRequest(
		Makai::String const& url,
		Makai::String const& action,
		Makai::Data::Value const& value
	) override {
		if (httpRequestsEnabled)
			return Engine::onHTTPRequest(url, action, value);
		Makai::Data::Value result = result.object();
		result["status"]	= 2;
		result["content"]	= "Program is forbidden from making HTTP requests";
		result["time"]		= 0;
		result["header"]	= "HTTP requests are not enabled!";
		result["source"]	= url;
		return result;
	}
};

struct ARTEMain: Makai::AMain {
	static Makai::Data::Value configBase() {
		Makai::Data::Value cfg;
		cfg["help"]		= false;
		cfg["net"]		= true;
		return cfg;
	}

	static void translationBase(Makai::CLI::Parser::Translation& tl) {
		tl["H"]		= "help";
		tl["N"]		= "net";
		tl["Net"]	= "net";
	}

	ARTEMain(Makai::CLI::Parser& cli): AMain(cli) {
		translationBase(cli.tl);
		baseArgs = configBase();
		showDialogOnError = false;
	}

	void write(Makai::String const& what) const override {DEBUG(what);}

	void run(Makai::Data::Value const& args) override {
		ARTE engine;
		engine.httpRequestsEnabled = args.fetch("net", false);
		if (args.fetch("help", false)) {
			DEBUGLN("Anima RunTime - V" + VER.serialize().get<Makai::String>());
			DEBUGLN("Available commands:");
			DEBUGLN("art <program> [-N]");
		} else {
			engine.load(Makai::File::getFLOW(args["__args"][0].getString() + ".anp"));
			engine.execute();
			while (engine.process()) {};
			engine.error().then([&] (auto const& e) {
				writeLine("!!! ERROR !!!");
				writeLine("At bytecode offset ", e.location);
				writeLine("Message: [", e.location, "]");
				return e;
			});
		}
	}
};

Makai_bindMain(ARTEMain);
