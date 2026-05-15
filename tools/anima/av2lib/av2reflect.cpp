#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct ReflectLib: ILibrary {
	static bool hasAttribute(Any const& obj, String const& attr) {
		if (!obj.value)					return false;
		if (!obj->getOriginalType())	return false;
		return obj->getOriginalType()->meta.contains(attr);
	}

	static StringList attributes(Any const& obj) {
		if (!obj.value)					return {};
		if (!obj->getOriginalType())	return {};
		return obj->getOriginalType()->meta.keys();
	}

	static Any unpack(Any const& obj, TypeID const& attr) {
		if (!obj.value)					return {};
		if (!obj->getOriginalType())	return {};
	}

	void load(Context::Adder const& context) override {
	}

	String name() const override {return "av2/reflect";}
};

AV2_Library(ReflectLib);
