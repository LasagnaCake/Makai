#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct ToStringLib: ALibrary {
	template <class T>
	static UTF8String toString(T val) {
		if constexpr (Makai::Type::Equal<T, Any>) {
			if (!val.value) return "";
			auto const vv = val.value->toDynamicValue();
			if (vv.isString()) return vv.getString();
			else return vv.toFLOWString();
		}
		else if constexpr (Makai::Type::Equal<T, UTF8Char>)
			return Makai::UTF8String(val);
		else if constexpr (Makai::Type::Equal<T, Vector4>)
			return Makai::Data::Value(val).toFLOWString();
		else return Makai::toString(val);
	}

	void load(Context::Adder const& context) override {
		context.methods.add("av2/tostring_any", toString<Any>);
		context.methods.add("av2/tostring_int8", toString<int8>);
		context.methods.add("av2/tostring_int16", toString<int16>);
		context.methods.add("av2/tostring_int32", toString<int32>);
		context.methods.add("av2/tostring_int64", toString<int64>);
		context.methods.add("av2/tostring_uint8", toString<uint8>);
		context.methods.add("av2/tostring_uint16", toString<uint16>);
		context.methods.add("av2/tostring_uint32", toString<uint32>);
		context.methods.add("av2/tostring_uint64", toString<uint64>);
		context.methods.add("av2/tostring_float32", toString<float32>);
		context.methods.add("av2/tostring_float64", toString<float64>);
		context.methods.add("av2/tostring_float128", toString<float128>);
		context.methods.add("av2/tostring_char", toString<UTF8Char>);
		context.methods.add("av2/tostring_vector", toString<Vector4>);
	}

	String name() const override {return "av2/tostring";}
};

AV2_Library(ToStringLib);
