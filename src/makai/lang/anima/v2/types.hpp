#ifndef MAKAILIB_ANIMA_V2_TYPES_H
#define MAKAILIB_ANIMA_V2_TYPES_H

#include "class.hpp"

namespace Makai::Anima::V2::Types {
	namespace Functions {
		inline Class::Function::Instance toString();
		inline Class::Function::Instance nameof();
		inline Class::Function::Instance baseof();
	}
	
	namespace Methods {
		inline Class::Method::Instance toString();
		inline Class::Method::Instance typeName();
		inline Class::Method::Instance baseName();
	}

	inline Class::Type String() {
		static Class::Type t = [] {
			auto t = new Class();
			t->underlying = Data::Value::Kind::DVK_STRING;
			return t;
		} ();
		return t;
	}
	
	inline Class::Type Any() {
		static Class::Type t = [] {
			auto t = new Class();
			t->isAny = true;
			return t;
		} ();
		return t;
	}
	
	inline Class::Type Value() {
		static Class::Type t = [] {
			auto t = new Class();
			t->base = Any()->id;
			t->methods.pushBack(Methods::toString());
			t->methods.pushBack(Methods::typeName());
			t->methods.pushBack(Methods::baseName());
			return t;
		} ();
		return t;
	}

	inline Class::Type Identifier() {
		static Class::Type t = [] {
			auto t = new Class();
			t->base = Value()->id;
			t->underlying = Data::Value::Kind::DVK_IDENTIFIER;
			return t;
		} ();
		return t;
	}

	inline Class::Type Boolean() {
		static Class::Type t = [] {
			auto t = new Class();
			t->base = Value()->id;
			t->underlying = Data::Value::Kind::DVK_BOOLEAN;
			return t;
		} ();
		return t;
	}
	
	inline Class::Type Signed() {
		static Class::Type t = [] {
			auto t = new Class();
			t->base = Value()->id;
			t->underlying = Data::Value::Kind::DVK_SIGNED;
			return t;
		} ();
		return t;
	}
	
	inline Class::Type Unsigned() {
		static Class::Type t = [] {
			auto t = new Class();
			t->base = Value()->id;
			t->underlying = Data::Value::Kind::DVK_UNSIGNED;
			return t;
		} ();
		return t;
	}
	
	inline Class::Type Real() {
		static Class::Type t = [] {
			auto t = new Class();
			t->base = Value()->id;
			t->underlying = Data::Value::Kind::DVK_REAL;
			return t;
		} ();
		return t;
	}

	inline Class::Type Array() {
		static Class::Type t = [] {
			auto t = new Class();
			t->base = Value()->id;
			t->underlying = Data::Value::Kind::DVK_ARRAY;
			return t;
		} ();
		return t;
	}

	inline Class::Type Bytes() {
		static Class::Type t = [] {
			auto t = new Class();
			t->base = Value()->id;
			t->underlying = Data::Value::Kind::DVK_BYTES;
			return t;
		} ();
		return t;
	}
	
	inline Class::Type Object() {
		static Class::Type t = [] {
			auto t = new Class();
			t->base = Value()->id;
			t->underlying = Data::Value::Kind::DVK_OBJECT;
			return t;
		} ();
		return t;
	}
	
	inline Class::Type Undefined() {
		static Class::Type t = [] {
			auto t = new Class();
			t->base = Value()->id;
			t->underlying = Data::Value::Kind::DVK_UNDEFINED;
			return t;
		} ();
		return t;
	}
	
	inline Class::Type Null() {
		static Class::Type t = [] {
			auto t = new Class();
			t->base = Value()->id;
			t->underlying = Data::Value::Kind::DVK_NULL;
			return t;
		} ();
		return t;
	}
	
	inline Class::Type NotANumber() {
		static Class::Type t = [] {
			auto t = new Class();
			t->base = Value()->id;
			t->underlying = Data::Value::Kind::DVK_NAN;
			return t;
		} ();
		return t;
	}
}

namespace Makai::Anima::V2::Types::Functions {
	inline Class::Function::Instance toString() {
		static Class::Function::Instance f = [] {
			auto f = new Class::Function();
			f->result = String()->id;
			f->args.pushBack(Any()->id);
			return f;
		} ();
		return f;
	}
	inline Class::Function::Instance nameof() {
		static Class::Function::Instance f = [] {
			auto f = new Class::Function();
			f->result = String()->id;
			f->args.pushBack(Any()->id);
			return f;
		} ();
		return f;
	}
	inline Class::Function::Instance baseof() {
		static Class::Function::Instance f = [] {
			auto f = new Class::Function();
			f->result = String()->id;
			f->args.pushBack(Any()->id);
			return f;
		} ();
		return f;
	}
}

namespace Makai::Anima::V2::Types::Methods {
	inline Class::Method::Instance toString() {
		static Class::Method::Instance m = [] {
			auto m = new Class::Method();
			m->id = Functions::toString()->id;
			return m;
		} ();
		return m;
	}
	inline Class::Method::Instance typeName() {
		static Class::Method::Instance m = [] {
			auto m = new Class::Method();
			m->id = Functions::nameof()->id;
			return m;
		} ();
		return m;
	}
	inline Class::Method::Instance baseName() {
		static Class::Method::Instance m = [] {
			auto m = new Class::Method();
			m->id = Functions::baseof()->id;
			return m;
		} ();
		return m;
	}
}

#endif