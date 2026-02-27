#ifndef MAKAILIB_ANIMA_V2_RUNTIME_BASICTYPES_H
#define MAKAILIB_ANIMA_V2_RUNTIME_BASICTYPES_H

#include "interfaces.hpp"

namespace Makai::Anima::V2::Runtime {
	namespace Type {
		template <uint64 FLAGS>
		struct ABasic: Runtime::AType {
			ABasic(): AType(FLAGS | AType::FLAG_BASIC) {}
		};

		struct Void: ABasic<0> {
			UTF8String	name() const override {return UTF8String("void");}
		};

		struct UInt: ABasic<AType::FLAG_UNSIGNED | AType::FLAG_INTEGER | AType::FLAG_NUMBER> {
			UTF8String name() const override {return UTF8String("uint");}
		};

		struct Int: ABasic<AType::FLAG_INTEGER | AType::FLAG_NUMBER> {
			UTF8String name() const override {return UTF8String("int");}
		};

		struct Real: ABasic<AType::FLAG_NUMBER> {
			UTF8String name() const override {return UTF8String("real");}
		};

		struct String: ABasic<AType::FLAG_STRING> {
			UTF8String name() const override {return UTF8String("string");}
		};

		struct Bytes: ABasic<AType::FLAG_BYTES> {
			UTF8String	name() const override {return UTF8String("bytes");}
		};

		struct Array: ABasic<AType::FLAG_ARRAY> {
			UTF8String	name() const override {return UTF8String("array");}

			constexpr static Instance<Array> of(Instance<AType> const& type) {
				auto const arr = new Array();
				arr->base = type;
				return arr;
			}
		};

	}
}

#endif
