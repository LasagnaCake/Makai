#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct ReflectLib: ILibrary {
	static bool hasAttribute(Any const& obj, String const& attr) {
		if (!obj.value)		return false;
		if (!obj->origin)	return false;
		return obj->origin->meta.contains(attr);
	}

	static StringList attributes(Any const& obj) {
		if (!obj.value)		return {};
		if (!obj->origin)	return {};
		return obj->origin->meta.keys();
	}

	void load(Context::Adder const& context) override {
	}

	String name() const override {return "av2/reflect";}
};

AV2_Library(ReflectLib);
