#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct SystemLib: ALibrary {

	static void AV2Call dload(Context& context, String const& libpath) {
		context.openLibrary(libpath);
		context.loadLibraries();
	}

	static Any AV2Call dcall(Context& context, String const& fname, List<Any> const& args) {
		Any out;
		context
			.invokeExternalMethod(fname, args)
			.then([&] (auto const& v) {out.value = v})
		;
		return out;
	}

	static void AV2Call dinvoke(Context& context, String const& fname, List<Any> const& args) {
		context.invokeExternalMethod(fname, args);
	}

	void load(Context::Adder const& context) override {
		context.methods.add("av2/system/dynamic/load",		dload	);
		context.methods.add("av2/system/dynamic/call",		dcall	);
		context.methods.add("av2/system/dynamic/invoke",	dinvoke	);
	}

	String name() const override {return "av2/system";}
};

AV2_Library(SystemLib);
