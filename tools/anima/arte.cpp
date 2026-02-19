#include <makai/makai.hpp>
#include <makai/main.hpp>

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

main(args) {
	ARTE engine;
	engine.httpRequestsEnabled = args["http"].get<bool>(false);
	engine.load(Makai::File::getFLOW(args["__args"][0].getString() + ".anp"));
}
