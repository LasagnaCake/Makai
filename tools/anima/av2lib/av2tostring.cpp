#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct ToStringLib: ILibrary {
	void open() {

	}

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

	void load(Context::MethodAdder const& context) {
		context.add("av2/tostring_any", toString<Any>);
		context.add("av2/tostring_int8", toString<int8>);
		context.add("av2/tostring_int16", toString<int16>);
		context.add("av2/tostring_int32", toString<int32>);
		context.add("av2/tostring_int64", toString<int64>);
		context.add("av2/tostring_uint8", toString<uint8>);
		context.add("av2/tostring_uint16", toString<uint16>);
		context.add("av2/tostring_uint32", toString<uint32>);
		context.add("av2/tostring_uint64", toString<uint64>);
		context.add("av2/tostring_float32", toString<float32>);
		context.add("av2/tostring_float64", toString<float64>);
		context.add("av2/tostring_float128", toString<float128>);
		context.add("av2/tostring_char", toString<UTF8Char>);
		context.add("av2/tostring_vector", toString<Vector4>);
	}

	void unload(Context::MethodRemover const& context) {
	}

	void close() {

	}
};

AV2_Library(ToStringLib);
