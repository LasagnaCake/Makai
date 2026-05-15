#include "makai/ctl/ctlex/data/value.hpp"
#include "makai/lang/anima/v2/core/type.hpp"
#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

namespace ARTMeta = Anima::V2::Core::Meta;

using Path = Makai::Data::Value::Path;

struct ReflectLib: ALibrary {
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

	static Any unpackImpl(Database<Definition>& types, Any const& obj, TypeID const& attr) {
		static auto const hashPath = Path{"::meta/hash"}.compiled();
		static auto const fieldMapPath = Path{"::meta/map"}.compiled();
		if (!obj.value)					return {};
		if (!obj->getOriginalType())	return {};
		auto const t = obj->getOriginalType();
		AsNonConst<decltype(t)> metaType;
		{
			auto const ts = types.queryByNameHash(attr.id);
			if (ts.empty()) return {};
			metaType = ts.front();
		}
		Makai::Data::Value meta;
		for (auto const& [name, vMeta]: t->meta) {
			if (meta.contains(hashPath) && meta[hashPath].getUnsigned() == attr.id) {
				meta = vMeta;
				break;
			}
		}
		if (meta.isUndefined()) return {};
		auto const fields = meta[fieldMapPath]
			.getArray()
			.filter([] (auto const& e) {return e.isString();})
			.toList<Makai::String>([] (auto const& e) {return e.getString();})
		;
		Any out;
		out.value = Object::create(types.byNameHash(Makai::hash("any")).front());
		for (auto const& [field, index]: Range::expand(fields)) {
			auto const value = meta[field];
			switch (value.type()) {
				using enum Makai::Data::Value::Kind;
				case DVK_STRING:	out.value->setAtIndex(index, Object::create(value.getString(), types.byNameHash(Makai::hash("string")).front()));
				case DVK_SIGNED:	out.value->setAtIndex(index, Object::create(value.getSigned(), types.byNameHash(Makai::hash("int64")).front()));
				case DVK_UNSIGNED:	out.value->setAtIndex(index, Object::create(value.getUnsigned(), types.byNameHash(Makai::hash("uint64")).front()));
				case DVK_BOOLEAN:	out.value->setAtIndex(index, Object::create(value.getBoolean(), types.byNameHash(Makai::hash("bool")).front()));
				case DVK_REAL:		out.value->setAtIndex(index, Object::create(value.getBytes(), types.byNameHash(Makai::hash("float64")).front()));
				default: break;
			}
		};
		return out;
	}

	void load(Context::Adder const& context) override {
		context.methods.add("av2/reflect/unpack", wrapUnpack());
	}

	String name() const override {return "av2/reflect";}
};

AV2_Library(ReflectLib);
