#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct ToStringLib: ILibrary {
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

	void load(Context::TypeAdder const& types, Context::MethodAdder const& methods) {
		methods.add("av2/tostring_any", toString<Any>);
		methods.add("av2/tostring_int8", toString<int8>);
		methods.add("av2/tostring_int16", toString<int16>);
		methods.add("av2/tostring_int32", toString<int32>);
		methods.add("av2/tostring_int64", toString<int64>);
		methods.add("av2/tostring_uint8", toString<uint8>);
		methods.add("av2/tostring_uint16", toString<uint16>);
		methods.add("av2/tostring_uint32", toString<uint32>);
		methods.add("av2/tostring_uint64", toString<uint64>);
		methods.add("av2/tostring_float32", toString<float32>);
		methods.add("av2/tostring_float64", toString<float64>);
		methods.add("av2/tostring_float128", toString<float128>);
		methods.add("av2/tostring_char", toString<UTF8Char>);
		methods.add("av2/tostring_vector", toString<Vector4>);
	}
};

AV2_Library(ToStringLib);
