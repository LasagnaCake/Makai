#ifndef CTL_EX_DATA_VALUE_H
#define CTL_EX_DATA_VALUE_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/container/container.hpp"
#include "../../ctl/typetraits/enum.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Data exchange format facilities.
namespace Data {
	/// @brief Dynamic value.
	struct Value;
}

/// @brief Data-specific type constraints.
namespace Type::Ex::Data {
	/// @brief Type must be a serializable type.
	template <class T>
	concept Serializable = requires (T v) {
			{v.serialize()} -> ::CTL::Type::Convertible<::CTL::Ex::Data::Value>;
	};

	/// @brief Type must be a deserializable type.
	template <class T>
	concept Deserializable = requires (T v) {
			{T::deserialize(declval<::CTL::Ex::Data::Value>())} -> Type::Convertible<T>;
	};
}

namespace Data {
	struct Value: Ordered {
		/// @brief Integer type.
		using IntegerType	= ssize;
		/// @brief Real number type.
		using RealType		= double;
		/// @brief String type.
		using StringType	= String;
		/// @brief Byte list type.
		using ByteListType	= List<byte>;
		/// @brief Array type.
		using ArrayType		= List<Value>;
		/// @brief Object type.
		using ObjectType	= ListMap<StringType, Value>;
		
		/// @brief Underlying value type.
		enum class Kind {
			DVK_UNDEFINED,
			DVK_NULL,
			DVK_NAN,
			DVK_BOOLEAN,
			DVK_SIGNED,
			DVK_UNSIGNED,
			DVK_REAL,
			DVK_STRING,
			DVK_ARRAY,
			DVK_BYTES,
			DVK_OBJECT
		};

		/// @brief Underlying storage.
		union Content {
			IntegerType			integer;
			RealType			real;
			StringType			string;
			ByteListType		bytes;
			owner<ArrayType>	array;
			owner<ObjectType>	object;

			constexpr Content()		{integer = 0;}
			constexpr ~Content()	{}
		};

		/// @brief Compiled element path.
		struct CompiledPath {
			/// @brief Path breakdown.
			List<KeyValuePair<ssize, StringType>> nodes;
		};
		
		/// @brief Element path.
		struct Path {
			/// @brief Path.	
			StringType value;
			
			/// @brief Returns the path as a compiled path.
			/// @return Compiled path.
			constexpr CompiledPath compiled() const {
				constexpr auto EMPTY_NODE = [] (StringType const& elem) {return elem.empty();};
				auto rp = value.replaced(StringType::DataType{'\\'}, '/').split(StringType::DataType{'/'}).eraseIf(EMPTY_NODE);
				CompiledPath path;
				path.nodes.reserve(rp.size());
				for (auto node: rp)
					if (node.validate([] (auto const& e) {return isNumberChar(static_cast<char>(e));}))
						path.nodes.pushBack({toInt64(::CTL::toString(node)), ""});
					else path.nodes.pushBack({Limit::MAX<ssize>, node});
				return path;
			}

			/// @brief Returns the path as a compiled path.
			constexpr operator CompiledPath() const {
				return compiled();
			}
		};

		/// @brief Empty constructor.
		constexpr Value():			kind(Kind::DVK_UNDEFINED)	{}
		/// @brief Constructs a null value.
		constexpr Value(nulltype):	kind(Kind::DVK_NULL)		{}

		/// @brief Constructs a boolean value.
		template <::CTL::Type::Equal<bool> T>
		constexpr Value(T const value):				kind(Kind::DVK_BOOLEAN)			{content.integer = value;					}
		/// @brief Constructs a signed integer value.
		template <::CTL::Type::SignedInteger T>
		constexpr Value(T const value)
		requires (Type::Different<T, bool>):		kind(Kind::DVK_SIGNED)			{content.integer = value;					}
		/// @brief Constructs an unsigned integer value.
		template <::CTL::Type::UnsignedInteger T>
		constexpr Value(T const value)
		requires (Type::Different<T, bool>):		kind(Kind::DVK_UNSIGNED)		{content.integer = value;					}
		/// @brief Constructs an unsigned integer value.
		template <::CTL::Type::Enumerator T>
		constexpr Value(T const value):				Value(enumcast(value))			{											}
		/// @brief Constructs a real number value.
		template <::CTL::Type::Real T>
		constexpr Value(T const value):				kind(Kind::DVK_REAL)			{content.real = value;						}
		/// @brief Constructs a string value.
		template <::CTL::Type::CanBecome<StringType> T>
		constexpr Value(T const& value):			kind(Kind::DVK_STRING)			{MX::construct(&content.string, value);		}
		/// @brief Constructs a byte list value.
		constexpr Value(ByteListType const& value):	kind(Kind::DVK_BYTES)			{MX::construct(&content.bytes, value);		}
		/// @brief Constructs an array value.
		constexpr Value(ArrayType const value):		kind(Kind::DVK_ARRAY)			{content.array = new ArrayType(value);		}
		/// @brief Constructs an object value.
		constexpr Value(ObjectType const& value):	kind(Kind::DVK_OBJECT)			{content.object = new ObjectType(value);	}		

		/// @brief Constructs the value from a serializable value.
		template <Type::Ex::Data::Serializable T>
		constexpr Value(T const& value): Value(value.serialize()) {}

		/// @brief Copy constructor.
		constexpr Value(Value const& other)	{operator=(other);}

		/// @brief Destructor.
		constexpr ~Value() {dump();}

		/// @brief Copy assignment operator.
		constexpr Value& operator=(Value const& other) {
			dump();
			switch (kind = other.kind) {
				case Kind::DVK_BOOLEAN:
				case Kind::DVK_UNSIGNED:
				case Kind::DVK_SIGNED:		content.integer	= other.content.integer;					break;
				case Kind::DVK_REAL:		content.real	= other.content.real;						break;
				case Kind::DVK_STRING:		MX::construct(&content.string, other.content.string);		break;
				case Kind::DVK_BYTES:		MX::construct(&content.bytes, other.content.bytes);			break;
				case Kind::DVK_ARRAY:		content.array	= new ArrayType(*other.content.array);		break;
				case Kind::DVK_OBJECT:		content.object	= new ObjectType(*other.content.object);	break;
				default: break;
			}
			return *this;
		}

		/// @brief Returns whether the value is undefined.
		constexpr bool isUndefined() const	{return kind == Kind::DVK_UNDEFINED;	}
		/// @brief Returns whether the value is null.
		constexpr bool isNull() const		{return kind == Kind::DVK_NULL;			}
		/// @brief Returns whether the value is not a number.
		constexpr bool isNaN() const		{return kind == Kind::DVK_NAN;			}
		/// @brief Returns whether the value is a boolean.
		constexpr bool isBoolean() const	{return kind == Kind::DVK_BOOLEAN;		}
		/// @brief Returns whether the value is a signed integer.
		constexpr bool isSigned() const		{return kind == Kind::DVK_SIGNED;		}
		/// @brief Returns whether the value is an unsigned integer.
		constexpr bool isUnsigned() const	{return kind == Kind::DVK_UNSIGNED;		}
		/// @brief Returns whether the value is a real number.
		constexpr bool isReal() const		{return kind == Kind::DVK_REAL;			}
		/// @brief Returns whether the value is a string.
		constexpr bool isString() const		{return kind == Kind::DVK_STRING;		}
		/// @brief Returns whether the value is an array.
		constexpr bool isArray() const		{return kind == Kind::DVK_ARRAY;		}
		/// @brief Returns whether the value is a byte list.
		constexpr bool isBytes() const		{return kind == Kind::DVK_BYTES;		}
		/// @brief Returns whether the value is an object.
		constexpr bool isObject() const		{return kind == Kind::DVK_OBJECT;		}

		/// @brief Returns whether the value is an integer.
		constexpr bool isInteger() const	{return isSigned() || isUnsigned();							}
		/// @brief Returns whether the value is a number.
		constexpr bool isNumber() const		{return isInteger() || isReal();							}
		/// @brief Returns whether the value is a scalar type.
		constexpr bool isScalar() const		{return isNumber() || isBoolean(); 							}
		/// @brief Returns whether the value is a data primitive.
		constexpr bool isPrimitive() const	{return isScalar() || isString() || isNull() || isBytes();	}
		/// @brief Returns whether the value is a structured type (array or object).
		constexpr bool isStructured() const {return isArray() || isObject();							}

		/// @brief Returns whether the value is a "falsy" value.
		/// @note Falsy values are: `undefined`, `null`, and empty strings.
		constexpr bool isFalsy() const		{return isNull() || isUndefined() || (isString() && empty());	}
		/// @brief Returns whether the value is a "truthy" value.
		/// @note Truthy values are: Arrays, objects & non-empty strings.
		constexpr bool isTruthy() const		{return isArray() || isObject() || (isString() && size());		}
		/// @brief Returns whether the type can be "coerced" to be a boolean.
		/// @brief Only type that currently cannot be coerced is byte list.
		constexpr bool isVerifiable() const	{return isTruthy() || isFalsy() || isPrimitive();				}

		/// @brief Returns whether the value is an integer.
		constexpr bool isInt() const		{return isInteger();	}
		/// @brief Returns whether the value is a real number.
		constexpr bool isFloat() const		{return isReal();		}
		/// @brief Returns whether the value is a boolean.
		constexpr bool isBool() const		{return isBoolean();	}
		/// @brief Returns whether the value is undefined.
		constexpr bool isDiscarded() const	{return isUndefined();	}

		/// @brief Tries to get the value as a given type.
		/// @tparam T value type.
		/// @param out Output.
		/// @return Whether value was successfully acquired.
		template <::CTL::Type::Equal<bool> T>
		constexpr bool tryGet(T& out) const {
			if (!isVerifiable()) return false;
			if (isFalsy())							out = false;
			else if (isTruthy())					out = true;
			else if (isInteger() || isBoolean())	out = content.integer;
			else if (isReal())						out = content.real;
			return true;
		}

		/// @brief Tries to get the value as a given type.
		/// @tparam T value type.
		/// @param out Output.
		/// @return Whether value was successfully acquired.
		template <::CTL::Type::Number T>
		constexpr bool tryGet(T& out) const 
		requires ::CTL::Type::Different<T, bool> {
			if (isInteger() || isBoolean())	out = content.integer;
			else if (isReal())				out = content.real;
			else return false;
			return true;
		}
		
		/// @brief Tries to get the value as a given type.
		/// @tparam T value type.
		/// @param out Output.
		/// @return Whether value was successfully acquired.
		template <::CTL::Type::Enumerator T>
		constexpr bool tryGet(T& out) const  {
			if (isInteger() || isBoolean())	out = static_cast<T>(content.integer);
			else if (isReal())				out = static_cast<T>(content.real);
			else return false;
			return true;
		}

		/// @brief Tries to get the value as a given type.
		/// @tparam T value type.
		/// @param out Output.
		/// @return Whether value was successfully acquired.
		template <::CTL::Type::OneOf<String, UTF8String> T>
		constexpr bool tryGet(T& out) const {
			if (isString()) out = content.string;
			else return false;
			return true;
		}

		/// @brief Tries to get the value as a given type.
		/// @tparam T value type.
		/// @param out Output.
		/// @return Whether value was successfully acquired.
		template <::CTL::Type::Equal<ByteListType> T>
		constexpr bool tryGet(T& out) const {
			if (isBytes()) out = content.bytes;
			else return false;
			return true;
		}

		/// @brief Tries to get the value as a given type.
		/// @tparam T value type.
		/// @param out Output.
		/// @return Whether value was successfully acquired.
		template <::CTL::Type::Equal<ArrayType> T>
		constexpr bool tryGet(T& out) const {
			if (isArray()) out = *content.array;
			else return false;
			return true;
		}

		/// @brief Tries to get the value as a given type.
		/// @tparam T value type.
		/// @param out Output.
		/// @return Whether value was successfully acquired.
		template <::CTL::Type::Equal<ObjectType> T>
		constexpr bool tryGet(T& out) const {
			if (isObject()) out = *content.object;
			else return false;
			return true;
		}

		/// @brief Tries to get the value as a given type.
		/// @tparam T value type.
		/// @param out Output.
		/// @return Whether value was successfully acquired.
		template <Type::Container::List T>
		constexpr bool tryGet(T& out) const
		requires (
			::CTL::Type::Different<typename T::DataType, Value>
		&&	::CTL::Type::Different<typename T::DataType, typename ByteListType::DataType>
		) {
			using ElementType = typename T::DataType;
			if (!isArray()) return false;
			out.clear().reserve(size());
			for (Value const& v: *content.array) {
				ElementType temp;
				if (!v.template tryGet<ElementType>(temp)) {
					out.clear();
					return false;
				}
				out.pushBack(temp);
			}
			return true;
		}

		/// @brief Tries to get the value as a given type.
		/// @tparam T value type.
		/// @param out Output.
		/// @return Whether value was successfully acquired.
		template <Type::Container::Map T>
		constexpr bool tryGet(T& out) const
		requires (::CTL::Type::Different<typename T::ValueType, Value>) {
			using ElementType = typename T::ValueType;
			if (!isObject()) return false;
			out.clear().reserve(size());
			for (auto const& [k, v]: *content.object) {
				ElementType temp;
				if (!v.template tryGet<ElementType>(temp)) {
					out.clear();
					return false;
				}
				v.tryGet(temp);
				out[k] = temp;
			}
			return true;
		}

		/// @brief Tries to get the value as a given type.
		/// @tparam T value type.
		/// @param out Output.
		/// @return Whether value was successfully acquired.
		template <::CTL::Type::Equal<Value> T>
		constexpr bool tryGet(T& out) const {
			out = *this;
			return true;
		}

		/// @brief Tries to get the value as a given type.
		/// @tparam T value type.
		/// @param out Output.
		/// @return Whether value was successfully acquired.
		template <Type::Ex::Data::Deserializable T>
		constexpr bool tryGet(T& out) const {
			out = T::deserialize(*this);
			return true;
		}

		/// @brief Returns whether the value is of a given type.
		/// @tparam T Checked type.
		/// @return Whether value is of given type.
		template <class T>
		constexpr bool is() const {
			if constexpr (Type::Equal<T, bool>)						return isBoolean();
			else if constexpr (Type::Equal<T, nulltype>)			return isNull();
			else if constexpr (Type::Equal<T, void>)				return isUndefined();
			else if constexpr (Type::Unsigned<T>)					return isUnsigned();
			else if constexpr (Type::Signed<T>)						return isSigned();
			else if constexpr (Type::Real<T>)						return isReal();
			else if constexpr (Type::OneOf<T, String, UTF8String>)	return isString();
			else if constexpr (Type::Equal<T, ArrayType>)			return isArray();
			else if constexpr (Type::Equal<T, ByteListType>)		return isBytes();
			else if constexpr (Type::Equal<T, ObjectType>)			return isObject();
			else if constexpr (Type::Equal<T, Value>)				return true;
			else return false;
		}

		/// @brief Tries to get a sub-element of a given type.
		/// @tparam T value type.
		/// @param out Output.
		/// @param index Element index.
		/// @return Whether value was successfully acquired.
		template <class T>
		constexpr bool tryFetch(T& out, ssize const index) const {
			if (!(isArray() && index < size()))
				return false;
			if constexpr (Type::Different<T, Value>)
				return (*content.array)[index].tryGet<T>(out);
			else {
				out = (*content.array)[index];
				return true;
			}
		}

		/// @brief Tries to get a sub-element of a given type.
		/// @tparam T value type.
		/// @param out Output.
		/// @param key Element key.
		/// @return Whether value was successfully acquired.
		template <class T>
		constexpr bool tryFetch(T& out, StringType const key) const {
			if (!(isObject() && contains(key)))
				return false;
			if constexpr (Type::Different<T, Value>)
				return read(key).tryGet<T>(out);
			else {
				out = read(key);
				return true;
			}
		}

		/// @brief Tries to get a sub-element of a given type.
		/// @tparam T value type.
		/// @param out Output.
		/// @param key Element path.
		/// @return Whether value was successfully acquired.
		template <class T>
		bool tryFetch(T& out, CompiledPath path) const {
			if (path.nodes.empty()) {
				if constexpr (Type::Different<T, Value>)
					return tryGet<T>(out);
				else {
					out = *this;
					return true;
				}
			}
			if (!isStructured()) return false;
			auto const key = path.nodes.front();
			path.nodes.remove(0);
			if (isArray()) {
				if (key.value.size()) return false;
				return operator[](key.key).tryFetch(out, path);
			} else {
				if (key.value.empty()) return false;
				return operator[](key.value).tryFetch(out, path);
			}
		}

		/// @brief Returns the value as a given type.
		/// @tparam T value type.
		/// @return Value as `T`.
		/// @throw Error::InvalidType On type mismatches.
		template <class T>
		constexpr T get() const {
			T out;
			if (!tryGet<T>(out))
				typeMismatchError(asKindName<T>());
			return out;
		}

		/// @brief Returns the value as a given type.
		/// @tparam T value type.
		/// @param fallback Fallback value.
		/// @return Value as `T`, or fallback.
		template <class T>
		constexpr T get(T const& fallback) const {
			T out;
			if (!tryGet<T>(out))
				return fallback;
			return out;
		}

		/// @brief Returns the value as a given type (Implicit conversion).
		template <class T>
		constexpr operator T() const {return get<T>();}

		/// @brief Array element access operator.
		/// @param index Element index.
		/// @return Element at given index.
		/// @note If index does not exist, array is grown until index does.
		/// @throw Error::InvalidType If value is not an array.
		template <Type::Integer T>
		constexpr Value& operator[](T const index) {
			if (isFalsy()) operator=(array());
			if (!isArray()) typeMismatchError("array");
			extendArray(index);
			return (*content.array)[index];
		}
		
		/// @brief Object element access operator.
		/// @param key Element key.
		/// @return Element at given key.
		/// @note If key does not exist, it is created.
		/// @throw Error::InvalidType If value is not an object.
		template <Type::CanBecome<StringType> T>
		constexpr Value& operator[](T const& key) {
			if (isFalsy()) operator=(object());
			if (!isObject()) typeMismatchError("object");
			if (!contains(key)) content.object->insert({key, Value::undefined()});
			return read(key);
		}

		/// @brief Object element access operator.
		/// @param path Element path.
		/// @return Element at given path.
		/// @note If path does not exist, it is created.
		/// @throw Error::InvalidType If a node in the path is not the correct type.
		constexpr Value& operator[](CompiledPath path) {
			if (path.nodes.empty()) return *this;
			if (!(isStructured() || isFalsy())) typeMismatchError("array or object");
			auto const key = path.nodes.front();
			path.nodes.remove(0);
			if (isArray()) {
				if (key.value.size()) typeMismatchError("array");
				return operator[](key.key).operator[](path);
			} else {
				if (key.value.empty()) typeMismatchError("object");
				return operator[](key.value).operator[](path);
			}
		}
		
		/// @brief Object element access operator.
		/// @param index Element index.
		/// @return Element at given index.
		/// @throw Error::InvalidType If value is not an array.
		/// @throw Error::OutOfBounds If index is out of bounds.
		template <Type::Integer T>
		constexpr Value operator[](T const index) const {
			if (!isArray()) typeMismatchError("array");
			else if (index >= static_cast<ssize>(size()))
				outOfBoundsError(index);
			return read(index);
		}

		/// @brief Object element access operator.
		/// @param key Element key.
		/// @return Element at given key.
		/// @throw Error::InvalidType If value is not an object.
		/// @throw Error::NonexistentValue If key does not exist.
		template <Type::CanBecome<StringType> T>
		constexpr Value operator[](T const& key) const {
			if (!isObject()) typeMismatchError("object");
			else if (!contains(key))
				missingKeyError(key);
			return read(key);
		}

		/// @brief Object element access operator.
		/// @param path Element path.
		/// @return Element at given path.
		/// @throw Error::InvalidType If a node in the path is not the correct type.
		/// @throw Error::InvalidType If value is not an object.
		/// @throw Error::NonexistentValue If key does not exist.
		constexpr Value const& operator[](CompiledPath path) const {
			if (path.nodes.empty()) return *this;
			if (!isStructured()) typeMismatchError("array or object");
			auto const key = path.nodes.front();
			path.nodes.remove(0);
			if (isArray()) {
				if (key.value.size()) typeMismatchError("array");
				return operator[](key.key).operator[](path);
			} else {
				if (key.value.empty()) typeMismatchError("object");
				if (!contains(key.value)) missingKeyError(key.value);
				return operator[](key.value).operator[](path);
			}
		}

		/// @brief Returns the size of the value.
		/// @return Value size. For falsy types, it is 0. For scalar types, it is 1. For any other type, it is its amout of elements.
		constexpr usize size() const;

		/// @brief Returns whether the value is empty.
		constexpr bool empty() const {return size() == 0;}

		/// @brief Threeway comparison operator.
		constexpr OrderType operator<=>(Value const& other) const {
			if (isFalsy() || other.isFalsy())		return isFalsy() <=> other.isFalsy();
			if (isBoolean() && other.isBoolean())	return (get<bool>() <=> other.template get<bool>());
			if (isInteger() && other.isInteger())	return (get<ssize>() <=> other.template get<ssize>());
			if (isReal() && other.isReal())			return (get<double>() <=> other.template get<double>());
			if (isString() && other.isString())		return (content.string <=> other.content.string);
			if (isBytes() && other.isBytes())		return (content.bytes <=> other.content.bytes);
			if (isArray() && other.isArray())		return (*content.array <=> *other.content.array);
			if (isObject() && other.isObject())		return compareWithRef(other.content.object);
			// Le JavaScript
			return (size() <=> other.size());
		}

		/// @brief Equality comparison operator.
		constexpr bool operator==(Value const& other) const {return operator<=>(other) == Order::EQUAL;	}
		/// @brief Inequality comparison operator.
		constexpr bool operator!=(Value const& other) const {return !operator==(other);					}

		/// @brief Swap implementation.
		constexpr friend void swap(Value& a, Value& b);

		/// @brief String padding.
		struct Padding {
			constexpr Padding(nulltype = nullptr): padding(false) {}
			template <Type::CanBecome <StringType> T>
			constexpr Padding(T const& pad): pad(""), followup(pad)												{}
			constexpr Padding(StringType const& pad, StringType const& followup): pad(pad), followup(followup)	{}

			constexpr Padding next() const {if (*this) return {pad + followup, followup}; return{};}

			constexpr operator bool() const			{return exists();	}
			constexpr operator StringType() const	{return toString();	}

			constexpr StringType toString() const	{return pad + followup;	}
			constexpr bool exists() const			{return padding;		}

			constexpr StringType base() const	{return pad;	}

		private:
			bool	padding = true;
			String	pad;
			String	followup;
		};

		/// @brief Converts the value to a JSON (JavaScript Object Notation) string.
		/// @param pad Padding to use. By default, it is `nullptr`.
		/// @return Value as JSON string.
		/// @note If padding is set to a string, newlines are added for each element.
		constexpr StringType toJSONString(Padding const& pad = nullptr) const {
			if (isFalsy()) return "null";
			if (isString())
				return escape(content.string);
			if (isBoolean())
				return content.integer ? "true" : "false";
			if (isInteger())
				return ::CTL::toString(content.integer);
			if (isReal())
				return ::CTL::toString(content.real);
			StringType const NEWLINE = StringType("\n");
			StringType const lhs = pad ? (NEWLINE + pad.toString()) : StringType("");
			if (isBytes()) {
				if (empty()) return "[]";
				StringType result = "[";
				for (auto const& v: content.bytes)
					result += lhs + ::CTL::toString(v) + ", ";
				return result.sliced(0, -3) + lhs + StringType("]");
			}
			if (isArray()) {
				if (empty()) return "[]";
				StringType result = "[";
				for (auto const& v: *content.array)
					result += lhs + v.toJSONString(pad.next()) + ", ";
				return result.sliced(0, -3) + (NEWLINE + pad.base()) + StringType("]");
			}
			if (isObject()) {
				if (empty()) return "{}";
				StringType result = "{";
				for (auto const& [k, v]: items())
					result +=  lhs + escape(k) + ": " + v.toJSONString(pad.next()) + ", ";
				return result.sliced(0, -3) + (NEWLINE + pad.base()) + StringType("}");
			}
			return StringType();
		}

		/// @brief Converts the value to a FLOW (Fast Lazy Object Writing) string.
		/// @param pad Padding to use. By default, it is `nullptr`.
		/// @return Value as FLOW string.
		/// @note If padding is set to a string, newlines are added for each element.
		constexpr StringType toFLOWString(Padding const& pad = nullptr) const {
			if (isFalsy()) return "null";
			if (isBoolean())
				return content.integer ? "true" : "false";
			if (isInteger())
				return ::CTL::toString(content.integer);
			if (isReal())
				return ::CTL::toString(content.real);
			if (isBytes()) {
				StringType result = "#32'";
				for (auto& b: content.bytes) {
					auto bs = String::fromNumber<byte>(b, 32).substring(2);
					if (bs.size() < 2)
						bs = "0" + bs;
					result += bs;
				}
				return result + "'";
			}
			StringType const NEWLINE = StringType("\n");
			StringType const lhs = pad ? (NEWLINE + pad.toString()) : StringType("");
			if (isArray()) {
				if (empty()) return "[]";
				StringType result = "[";
				for (auto const& v: *content.array)
					result += lhs + v.toFLOWString(pad.next()) + " ";
				return result.sliced(0, -2) + (NEWLINE + pad.base()) + StringType("]");
			}
			if (isObject()) {
				if (empty()) return "{}";
				StringType result = "{";
				for (auto [k, v]: items())
					result +=  lhs + escape(k) + " " + v.toFLOWString(pad.next()) + " ";
				return result.sliced(0, -2) + (NEWLINE + pad.base()) + StringType("}");
			}
			return StringType();
		}

		/// @brief String format type.
		enum class Format {
			DVF_JSON,
			DVF_FLOW
		};

		/// @brief Converts the value to a data string.
		/// @param pad Padding to use. By default, it is `nullptr`.
		/// @param format String format to use. By default, it is `Format::DVF_JSON`.
		/// @return Value as string.
		/// @note If padding is set to a string, newlines are added for each element.
		constexpr StringType toString(Padding const& pad = nullptr, Format const format = Format::DVF_JSON) {
			switch (format) {
				case Format::DVF_JSON: return toJSONString(pad);
				case Format::DVF_FLOW: return toFLOWString(pad);
			}
			return "";
		}

		/// @brief Creates an undefined value.
		constexpr static Value undefined()		{return Value();		}
		/// @brief Creates a null value.
		constexpr static Value null()			{return nullptr;		}
		/// @brief Creates a boolean value.
		constexpr static Value boolean()		{return false;			}
		/// @brief Creates a signed integer value.
		constexpr static Value signedInt()		{return 0;				}
		/// @brief Creates an unsigned integer value.
		constexpr static Value unsignedInt()	{return 0u;				}
		/// @brief Creates a real number value.
		constexpr static Value real()			{return 0d;				}
		/// @brief Creates a string value.
		constexpr static Value string()			{return StringType();	}
		/// @brief Creates an array value.
		constexpr static Value array()			{return ArrayType();	}
		/// @brief Creates a byte list value.
		constexpr static Value bytes()			{return ByteListType();	}
		/// @brief Creates an object value.
		constexpr static Value object();

		/// @brief Empties the value.
		/// @return Reference to self.
		constexpr Value& clear() {
			dump();
			return *this;
		}

		/// @brief Object key-value pair list type.
		using ObjectItemListType = List<KeyValuePair<StringType const, Value>>;

		/// @brief Returns the object's contents as a list of key-value pairs.
		/// @return Object contents, or empty list if not an object.
		constexpr ObjectItemListType items() const;
	
		/// @brief Object key list type.
		using ObjectKeyListType = List<StringType>;

		/// @brief Returns the object's keys.
		/// @return Object keys, or empty list if not an object.
		constexpr ObjectKeyListType keys() const;

		/// @brief Appends another value's contents into this one.
		/// @param other Value to append.
		/// @return Reference to self.
		constexpr Value& append(Value const& other);

		/// @brief Appends a series of values into this one.
		/// @param values Values to append.
		/// @return Reference to self.
		constexpr Value& append(List<Value> const& values) {
			for (auto const& v: values)
				append(v);
			return *this;			
		}

		/// @brief Appends a series of values into this one.
		/// @param values... Values to append.
		/// @return Reference to self.
		template <class... Types>
		constexpr Value& append(Types const... values)
		requires (sizeof...(Types) > 2) {
			(..., append(values));
			return *this;
		}

		/// @brief Merges a series of values into a single value.
		/// @param first First value to merge.
		/// @param rest... Other values to merge.
		/// @return Reference to self.
		template <class... Types>
		constexpr Value merge(Value first, Types const&... rest)
		requires (... && Type::CanBecome<Types, Value>) {
			return first.append(rest...);
		}
		
		/// @brief Returns compiler, OS and architecture information as a value.
		/// @return Information.
		constexpr static Value info();

		/// @brief Returns whether the value contains a given key.
		constexpr bool contains(StringType const& key) const;

		/// @brief Returns whether the value contains a given path.
		constexpr bool contains(CompiledPath path) const {
			if (path.nodes.empty()) return isFalsy();
			if (!isStructured()) return false;
			auto const key = path.nodes.front();
			path.nodes.remove(0);
			if (isArray()) {
				if (key.value.size()) return false;
				return operator[](key.key).contains(path);
			} else {
				if (key.value.empty()) return false;
				if (!contains(key.value)) return false;
				return operator[](key.value).contains(path);
			}
		}

	private:
		/// @brief Value type.
		Kind	kind;
		/// @brief Value content.
		Content	content;

		constexpr void extendArray(ssize const sz);

		constexpr Value& read(StringType const& key);
		constexpr Value read(StringType const& key) const;
		constexpr Value& read(ssize const index);
		constexpr Value read(ssize const index) const;

		constexpr OrderType compareWithRef(ref<ObjectType> const& object) const;

		[[noreturn]] void typeMismatchError(String const& expectedType) const {
			throw Error::InvalidType(
				"Type mismatch!",
				"Value type is [" + enumAsString(kind) + "],"
				"\nExpected type is [" + expectedType + "]"
			);
		}

		[[noreturn]] void outOfBoundsError(ssize const index) const {
			throw Error::OutOfBounds(
				"Index [" + ::CTL::toString(index) + "] is out of bounds!",
				"Array size is [" + ::CTL::toString(size()) + "]"
			);
		}

		[[noreturn]] void missingKeyError(StringType const& key) const {
			throw Error::NonexistentValue("Object does not contain key \"" + key.toString() + "\"!");
		}

		template <class T>
		constexpr static String asKindName() {
			return "T";
		}

		constexpr static String enumAsString(Kind const& kind) {
			switch (kind) {
				case Kind::DVK_UNDEFINED:	return "undefined";
				case Kind::DVK_NULL:		return "null";
				case Kind::DVK_NAN:			return "NaN";
				case Kind::DVK_BOOLEAN:		return "boolean";
				case Kind::DVK_SIGNED:		return "signed";
				case Kind::DVK_UNSIGNED:	return "unsigned";
				case Kind::DVK_REAL:		return "real";
				case Kind::DVK_STRING:		return "string";
				case Kind::DVK_BYTES:		return "bytes";
				case Kind::DVK_ARRAY:		return "array";
				case Kind::DVK_OBJECT:		return "object";
			}
			return "none";
		}

		constexpr static StringType escape(StringType const& str) {
			StringType result = "\"";
			for (auto const& c: str) {
				if (c == StringType::DataType{'\''})		result += "\\'";
				else if (c == StringType::DataType{'\\'})	result += "\\\\";
				else if (c == StringType::DataType{'\n'})	result += "\\n";
				else if (c == StringType::DataType{'\t'})	result += "\\t";
				else if (c == StringType::DataType{'\"'})	result += "\\\"";
				else result.pushBack(c);
			}
			return result + StringType("\"");
		}

		constexpr void dump() {
			switch (kind) {
				default: break;
				case Kind::DVK_STRING:	MX::destruct(&content.string);						break;
				case Kind::DVK_BYTES:	MX::destruct(&content.bytes);						break;
				case Kind::DVK_ARRAY:	delete content.array; content.array = nullptr;		break;
				case Kind::DVK_OBJECT:	delete content.object; content.object = nullptr;	break;
			}
			kind = Kind::DVK_UNDEFINED;
		}
	};

	constexpr Value::OrderType Value::compareWithRef(ref<Value::ObjectType> const& value) const {
		return Value::Order::EQUAL;
	}

	constexpr void Value::extendArray(ssize const count) {
		while (count >= static_cast<ssize>(size()))
			content.array->pushBack(nullptr);
	}
	
	constexpr Value& Value::read(ssize const index) {
		return content.array->operator[](index);
	}
	
	constexpr Value Value::read(ssize const index) const {
		return content.array->operator[](index);
	}

	constexpr Value& Value::read(Value::StringType const& key) {
		return content.object->operator[](key);
	}
	
	constexpr Value Value::read(Value::StringType const& key) const {
		return content.object->operator[](key);
	}

	constexpr usize Value::size() const {
		if (isString())	return content.string.size();
		if (isBytes())	return content.bytes.size();
		if (isArray())	return content.array->size();
		if (isObject())	return content.object->size();
		if (isScalar())	return 1;
		return 0;
	}

	constexpr void swap(Value& a, Value& b) {
		Value tmp = a;
		a = b;
		b = tmp;
	}

	constexpr Value Value::object() {
		return Value::ObjectType();
	}

	constexpr Value& Value::append(Value const& other)  {
		if (!(isObject() || other.isObject())) return *this;
		if (!other.isObject()) return *this;
		if (!isObject()) *this = object();
		for (auto [k, v]: other.items()) {
			if (v.isNull()) operator[](k) = nullptr;
			else operator[](k).append(v);
		}
		return *this;
	}

	constexpr Value Value::info()  {
		Value result = object();
		#if defined(__GNUG__) && !defined(__clang__)
		result["compiler"]["name"]		= "gcc";
		result["compiler"]["version"]	= StringType(::CTL::toString(__GNUC__, ".", __GNUC_MINOR__));
		#elif defined(__clang__)
		result["compiler"]["name"]		= "clang";
		result["compiler"]["name"]		= __clang_version__;
		#else
		#error "Unsupported compiler!"
		#endif
		result["cpp"] = Value::StringType(::CTL::toString(__cplusplus));
		#if defined(_WIN32) || defined(_WIN64)
		result["os"]	= "windows";
		#elif defined(__linux__)
		result["os"]	= "linux";
		#elif defined(__APPLE__) || defined(__MACH__)
		result["os"]	= "apple";
		#elif defined(__unix__)
		result["os"]	= "unix";
		#endif
		#if defined(__x86_64__) || defined(_M_X64)
		result["arch"]	= "x64";
		#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
		result["arch"]	= "x86";
		#elif defined(__aarch64__) || defined(_M_ARM64)
		result["arch"]	= "arm64";
		#else 
		result["arch"]	= "unknown";
		#endif 
		return result;
	}

	constexpr bool Value::contains(Value::StringType const& key) const  {
		if (!isObject()) return false;
		return content.object->contains(key);
	}

	constexpr Value::ObjectItemListType Value::items() const  {
		if (!isObject()) return {};
		return content.object->items();
	}

	constexpr Value::ObjectKeyListType Value::keys() const  {
		if (!isObject()) return {};
		return content.object->keys();
	}
}

CTL_EX_NAMESPACE_END

#endif