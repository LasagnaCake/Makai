#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct NetLib: ALibrary {
	using Response = Proxy<Net::HTTP::Response>;

	static Proxy<Net::HTTP::Response> fetch(String const& url, u32 const type, String const& data, String const& ssl) {
		return Makai::Net::HTTP::fetch({Makai::Net::HTTP::Request::Type{type}, data, ssl});
	}

	void load(Context::Adder const& context) override {
		context.types.add<Response>();
	}

	String name() const override {return "av2/net";}
};

AV2_Library(NetLib);
