#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct SystemLib: ALibrary {

	AV2Call
	static void dload(Context& context, String const& libpath) {
		context.openLibrary(libpath);
		context.loadLibraries();
	}

	AV2Call
	static Any dcall(Context& context, String const& fname, List<Any> const& args) {
		Any out;
		context
			.callNative(fname, args.toList<Object::Storage>([] (auto const& e) {return e.value;}))
			.then([&] (auto const& v) {out.value = v;})
		;
		return out;
	}

	AV2Call
	static void dinvoke(Context& context, String const& fname, List<Any> const& args) {
		context.callNative(fname, args.toList<Object::Storage>([] (auto const& e) {return e.value;}));
	}

	void load(Context::Adder const& context) override {
		context.methods.add("av2/system/dynamic/load",		dload	);
		context.methods.add("av2/system/dynamic/call",		dcall	);
		context.methods.add("av2/system/dynamic/invoke",	dinvoke	);
	}

	String name() const override {return "av2/system";}
};

AV2_Library(SystemLib);
