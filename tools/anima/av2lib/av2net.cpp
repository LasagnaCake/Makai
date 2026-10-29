#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct NetLib: ALibrary {
	static Net::HTTP::Response fetch(String const& url, u32 const type, String const& data, String const& ssl) {
		return Makai::Net::HTTP::fetch({Makai::Net::HTTP::Request::Type{type}, data, ssl});
	}

	struct ResponseHandler {
		static u32 status(Net::HTTP::Response const& self) {
			return enumcast(self.status);
		}

		static double time(Net::HTTP::Response const& self) {
			return self.time;
		}

		static String source(Net::HTTP::Response const& self) {
			return self.source;
		}

		static String header(Net::HTTP::Response const& self) {
			return self.header;
		}

		static String content(Net::HTTP::Response const& self) {
			return self.content;
		}
	};

	void load(Context::Adder const& context) override {
		context.types.add<Net::HTTP::Response>();
		context.methods.add("av2/net/fetch", fetch);
		context.methods.add("av2/net/Response.status", ResponseHandler::status);
		context.methods.add("av2/net/Response.source", ResponseHandler::source);
		context.methods.add("av2/net/Response.header", ResponseHandler::header);
		context.methods.add("av2/net/Response.content", ResponseHandler::content);
		context.methods.add("av2/net/fetchText",	Net::File::fetchText);
		context.methods.add("av2/net/fetchBinary",	Net::File::fetchText);
	}

	String name() const override {return "av2/net";}
};

AV2_Library(NetLib);
