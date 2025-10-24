#ifndef CTL_EX_DATA_VALUE_H
#define CTL_EX_DATA_VALUE_H

#include "../../ctl/container/container.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Data exchange format facilities.
namespace Data {
	/// @brief Dynamic value.
	struct Value;
}

/// @brief Data-specific type constraints.
namespace Type::Data {
	/// @brief Type must be a serializable type.
	template <class T>
	concept Serializable = requires (T v) {
			{v.serialize()} -> Type::Convertible<::CTL::Data::Value>;
	};

	/// @brief Type must be a deserializable type.
	template <class T>
	concept Deserializable = requires (T v) {
			{T::deserialize(declval<::CTL::Data::Value>())} -> Type::Convertible<T>;
	};
}

namespace Data {
	struct Value: Ordered {
		/// @brief Integer type.
		using IntegerType	= ssize;
		/// @brief Real number type.
		using RealType		= double;
		/// @brief String type.
		using StringType	= UTF8String;
		/// @brief Byte list type.
		using ByteListType	= List<byte>;
		/// @brief Array type.
		using ArrayType		= List<Value>;
		/// @brief Object type.
		using ObjectType	= ListMap<StringType, Value>;
		
		/// @brief Underlying value type.
		enum class Kind {
			DVT_UNDEFINED,
			DVT_NULL,
			DVT_NAN,
			DVT_BOOLEAN,
			DVT_SIGNED,
			DVT_UNSIGNED,
			DVT_REAL,
			DVT_STRING,
			DVT_ARRAY,
			DVT_BYTES,
			DVT_OBJECT
		};

		/// @brief Underlying storage.
		union Content {
			IntegerType			integer;
			RealType			real;
			StringType			string;
			ByteListType		bytes;
			Unique<ArrayType>	array;
			Unique<ObjectType>	object;

			constexpr Content() {}
			constexpr ~Content() {}
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
					if (node.validate([] (auto const& e) {return isNumberChar(static_cast<char>(e.value()));}))
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
		constexpr Value():			kind(Kind::DVT_UNDEFINED)	{}
		/// @brief Constructs a null value.
		constexpr Value(nulltype):	kind(Kind::DVT_NULL)		{}

		/// @brief Constructs a boolean value.
		constexpr Value(bool const value):			kind(Kind::DVT_BOOLEAN)		{content.integer = value;	}
		/// @brief Constructs a signed integer value.
		template <::CTL::Type::SignedInteger T>
		constexpr Value(T const value):				kind(Kind::DVT_SIGNED)		{content.integer = value;	}
		/// @brief Constructs an unsigned integer value.
		template <::CTL::Type::Unsigned T>
		constexpr Value(T const value):				kind(Kind::DVT_UNSIGNED)	{content.integer = value;	}
		/// @brief Constructs a real number value.
		template <::CTL::Type::Real T>
		constexpr Value(T const value):				kind(Kind::DVT_REAL)		{content.real = value;		}
		/// @brief Constructs a string value.
		constexpr Value(StringType const& value):	kind(Kind::DVT_STRING)		{content.string = value;	}
		/// @brief Constructs an array value.
		constexpr Value(ArrayType const value):		kind(Kind::DVT_ARRAY)		{makeFromArray(value);		}
		/// @brief Constructs a byte list value.
		constexpr Value(ByteListType const& value):	kind(Kind::DVT_BYTES)		{content.bytes = value;		}
		/// @brief Constructs an object value.
		constexpr Value(ObjectType const& value):	kind(Kind::DVT_OBJECT)		{makeFromObject(value);		}		

		/// @brief Constructs the value from a serializable value.
		template <::CTL::Type::Data::Serializable T>
		constexpr Value(T const& value): Value(value.serialize()) {}

		/// @brief Copy constructor.
		constexpr Value(Value const& other)	{operator=(other);}

		/// @brief Destructor.
		constexpr ~Value() {dump();}

		/// @brief Copy assignment operator.
		constexpr Value& operator=(Value const& other) {
			dump();
			kind = other.kind;
			switch (kind) {
				case Kind::DVT_BOOLEAN:
				case Kind::DVT_UNSIGNED:
				case Kind::DVT_SIGNED:		content.integer	= other.content.integer;
				case Kind::DVT_REAL:		content.real	= other.content.real;
				case Kind::DVT_STRING:		MX::construct(&content.string,		other.content.string	);
				case Kind::DVT_ARRAY:		makeFromArray(*other.content.array);
				case Kind::DVT_BYTES:		MX::construct(&content.bytes,		other.content.bytes		);
				case Kind::DVT_OBJECT:		makeFromObjectRef(other.content.object);
				default: break;
			}
			return *this;
		}

		/// @brief Returns whether the value is undefined.
		constexpr bool isUndefined() const	{return kind == Kind::DVT_UNDEFINED;	}
		/// @brief Returns whether the value is null.
		constexpr bool isNull() const		{return kind == Kind::DVT_NULL;			}
		/// @brief Returns whether the value is not a number.
		constexpr bool isNaN() const		{return kind == Kind::DVT_NAN;			}
		/// @brief Returns whether the value is a boolean.
		constexpr bool isBoolean() const	{return kind == Kind::DVT_BOOLEAN;		}
		/// @brief Returns whether the value is a signed integer.
		constexpr bool isSigned() const		{return kind == Kind::DVT_SIGNED;		}
		/// @brief Returns whether the value is an unsigned integer.
		constexpr bool isUnsigned() const	{return kind == Kind::DVT_UNSIGNED;		}
		/// @brief Returns whether the value is a real number.
		constexpr bool isReal() const		{return kind == Kind::DVT_REAL;			}
		/// @brief Returns whether the value is a string.
		constexpr bool isString() const		{return kind == Kind::DVT_STRING;		}
		/// @brief Returns whether the value is an array.
		constexpr bool isArray() const		{return kind == Kind::DVT_ARRAY;		}
		/// @brief Returns whether the value is a byte list.
		constexpr bool isBytes() const		{return kind == Kind::DVT_BYTES;		}
		/// @brief Returns whether the value is an object.
		constexpr bool isObject() const		{return kind == Kind::DVT_OBJECT;		}

		/// @brief Returns whether the value is an integer.
		constexpr bool isInteger() const	{return isSigned() || !isUnsigned();										}
		/// @brief Returns whether the value is a number.
		constexpr bool isNumber() const		{return isInteger() || isReal();											}
		/// @brief Returns whether the value is a scalar type.
		constexpr bool isScalar() const		{return isNumber() || isBoolean(); 											}
		/// @brief Returns whether the value is a data primitive.
		constexpr bool isPrimitive() const	{return isScalar() || isString() || isNull() || isBytes();					}
		/// @brief Returns whether the value is a structured type (array or object).
		constexpr bool isStructured() const {return isArray() || isObject();											}

		/// @brief Returns whether the value is a "falsy" value.
		/// @note Falsy values are: `undefined`, `null`, and empty strings.
		constexpr bool isFalsy() const		{return isNull() || isUndefined() || (isString() && empty());	}
		/// @brief Returns whether the value is a "truthy" value.
		/// @note Truthy values are: Arrays, objects & non-empty strings.
		constexpr bool isTruthy() const		{return isArray() || isObject() || (isString() && size());		}
		/// @brief Returns whether the type can be "coerced" to be a boolean.
		/// @brief Only type that currently cannot be coerced is byte list.
		constexpr bool isVerifiable() const	{return isTruthy() || isFalsy() || isPrimitive();				}

		/// @brief Tries to get the value as a given type.
		/// @tparam T value type.
		/// @param out Output.
		/// @return Whether value was successfully acquired.
		template <::CTL::Type::Equal<bool> T>
		constexpr bool tryGet(T& out) const {
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
		template <::CTL::Type::Equal<ArrayType> T>
		constexpr bool tryGet(T& out) const {
			if (isArray()) out = content.array;
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
		template <::CTL::Type::Equal<ObjectType> T>
		constexpr bool tryGet(T& out) const {
			if (isObject()) out = content.object;
			else return false;
			return true;
		}

		/// @brief Tries to get the value as a given type.
		/// @tparam T value type.
		/// @param out Output.
		/// @return Whether value was successfully acquired.
		template <class T>
		constexpr bool tryGet(List<T>& out) const
		requires (::CTL::Type::Different<T, Value> && ::CTL::Type::Different<T, typename ByteListType::DataType>) {
			out.clear().reserve(size());
			for (Value const& v: *content.array) {
				T temp;
				if (!v.template is<T>()) {
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
		template <::CTL::Type::Equal<Value> T>
		constexpr bool tryGet(T& out) const {
			out = *this;
			return true;
		}

		/// @brief Tries to get the value as a given type.
		/// @tparam T value type.
		/// @param out Output.
		/// @return Whether value was successfully acquired.
		template <::CTL::Type::Data::Deserializable T>
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
		constexpr Value& operator[](ssize const index) {
			if (isFalsy()) *this = array();
			if (!isArray()) typeMismatchError("array");
			extendArray(index);
			return (*content.array)[index];
		}
		
		/// @brief Object element access operator.
		/// @param key Element key.
		/// @return Element at given key.
		/// @note If key does not exist, it is created.
		/// @throw Error::InvalidType If value is not an object.
		constexpr Value& operator[](StringType const& key) {
			if (isFalsy()) *this = object();
			if (!isObject()) typeMismatchError("object");
			if (!contains(key)) read(key) = Value::undefined();
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
		constexpr Value operator[](ssize const index) const {
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
		constexpr Value operator[](StringType const& key) const {
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
		constexpr bool empty() const {return size() > 0;}

		/// @brief Threeway comparison operator.
		constexpr OrderType operator<=>(Value const& other) const {
			if (isFalsy() || other.isFalsy())		return isFalsy() <=> other.isFalsy();
			if (isBoolean() && other.isBoolean())	return (get<bool>() <=> other.template get<bool>());
			if (isInteger() && other.isInteger())	return (get<ssize>() <=> other.template get<ssize>());
			if (isReal() && other.isReal())			return (get<double>() <=> other.template get<double>());
			if (isString() && other.isString())		return (content.string <=> other.content.string);
			if (isArray() && other.isArray())		return (content.array <=> other.content.array);
			if (isBytes() && other.isBytes())		return (content.bytes <=> other.content.bytes);
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
		using Padding = Nullable<StringType>;

		/// @brief Converts the value to a JSON (JavaScript Object Notation) string.
		/// @param pad Padding to use. By default, it is `nullptr`.
		/// @return Value as JSON string.
		/// @note If padding is set to a string, newlines are added for each element.
		constexpr StringType toJSONString(Padding const& pad = nullptr) const {
			if (isFalsy()) return "null";
			if (isString())
				return escape(content.string);
			if (isBoolean() || isInteger())
				return ::CTL::toString(content.integer);
			if (isReal())
				return ::CTL::toString(content.real);
			if (isBytes())
				// TODO: This
				return "[]";
			StringType const lhs = pad ? StringType("") : StringType("\n") + pad.value();
			Padding const next = pad ? Padding{nullptr} : Padding{pad.value() + pad.value()};
			if (isArray()) {
				StringType result = "[";
				for (auto const& v: *content.array)
					result += lhs + v.toJSONString(next) + ", ";
				return result.substring(0, -3) + lhs + StringType("]");
			}
			if (isObject()) {
				StringType result = "{";
				for (auto const& [k, v]: items())
					result +=  lhs + escape(k) + ": " + v.toJSONString(next) + ", ";
				return result.substring(0, -3) + lhs + StringType("}");
			}
			return "";
		}

		/// @brief Converts the value to a FLOW (Fast Lazy Object Writing) string.
		/// @param pad Padding to use. By default, it is `nullptr`.
		/// @return Value as FLOW string.
		/// @note If padding is set to a string, newlines are added for each element.
		constexpr StringType toFLOWString(Padding const& pad = nullptr) const {
			if (isFalsy()) return "null";
			if (isString())
				return escape(content.string);
			if (isBoolean() || isInteger())
				return ::CTL::toString(content.integer);
			if (isReal())
				return ::CTL::toString(content.real);
			if (isBytes())
				// TODO: This
				return "[]";
			StringType const lhs = pad ? StringType("") : StringType("\n") + pad.value();
			Padding const next = pad ? Padding{nullptr} : Padding{pad.value() + pad.value()};
			if (isArray()) {
				StringType result = "[";
				for (auto const& v: *content.array)
					result += lhs + v.toFLOWString(next) + " ";
				return result.substring(0, -3) + lhs + StringType("]");
			}
			if (isObject()) {
				StringType result = "{";
				for (auto const& [k, v]: items())
					result +=  lhs + escape(k) + " " + v.toFLOWString(next) + " ";
				return result.substring(0, -3) + lhs + StringType("}");
			}
			return "";
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
		using ObjectItemListType = List<KeyValuePair<StringType, Value>>;

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
		requires (... && Type::Convertible<Types, Value>) {
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

		constexpr void makeFromArray(ArrayType const& array);
		constexpr void extendArray(ssize const sz);

		constexpr void makeFromObject(ObjectType const& object);
		constexpr void makeFromObjectRef(Unique<ObjectType> const& object);

		constexpr Value& read(StringType const& key);
		constexpr Value read(StringType const& key) const;
		constexpr Value& read(ssize const index);
		constexpr Value read(ssize const index) const;

		constexpr OrderType compareWithRef(Unique<ObjectType> const& object) const;

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
				case Kind::DVT_UNDEFINED:	return "undefined";
				case Kind::DVT_NULL:		return "null";
				case Kind::DVT_NAN:			return "NaN";
				case Kind::DVT_BOOLEAN:		return "boolean";
				case Kind::DVT_SIGNED:		return "signed";
				case Kind::DVT_UNSIGNED:	return "unsigned";
				case Kind::DVT_REAL:		return "real";
				case Kind::DVT_STRING:		return "string";
				case Kind::DVT_BYTES:		return "bytes";
				case Kind::DVT_ARRAY:		return "array";
				case Kind::DVT_OBJECT:		return "object";
			}
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

		constexpr void dump();
	};

	constexpr void Value::dump() {
		switch (kind) {
			case Kind::DVT_STRING:	MX::destruct(&content.string);
			case Kind::DVT_ARRAY:	MX::destruct(content.array.raw());
			case Kind::DVT_BYTES:	MX::destruct(&content.bytes);
			case Kind::DVT_OBJECT:	MX::destruct(content.object.raw());
			default: break;
		}
		kind = Kind::DVT_UNDEFINED;
	}

	constexpr void Value::makeFromArray(Value::ArrayType const& value) {
		content.array.bind(new ArrayType({value}));
	}

	constexpr void Value::makeFromObject(Value::ObjectType const& value) {
		content.object.bind(new ObjectType(copy(value)));
	}

	constexpr void Value::makeFromObjectRef(Unique<Value::ObjectType> const& value) {
		makeFromObject(*value);
	}

	constexpr Value::OrderType Value::compareWithRef(Unique<Value::ObjectType> const& value) const {
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
		if (isFalsy())	return 0;
		if (isScalar())	return 1;
		if (isString())	return content.string.size();
		if (isArray())	return content.array->size();
		if (isBytes())	return content.bytes.size();
		if (isObject())	return content.object->size();
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
	}

	constexpr Value Value::info()  {
		Value result = object();
		#if defined(__GNUG__) && !defined(__clang__)
		result[{"compiler"}][{"name"}]		= "gcc";
		result[{"compiler"}][{"version"}]	= ::CTL::toString(__GNUC__, ".", __GNUC_MINOR__);
		#elif defined(__clang__)
		result[{"compiler"}][{"name"}]		= "clang";
		result[{"compiler"}][{"version"}]	= __clang_version__;
		#else
		#error "Unsupported compiler!"
		#endif
		result[{"cpp"}] = UTF8String(::CTL::toString(__cplusplus));
		#if defined(_WIN32) || defined(_WIN64)
		result[{"os"}]	= "windows";
		#elif defined(__linux__)
		result[{"os"}]	= "linux";
		#elif defined(__APPLE__) || defined(__MACH__)
		result[{"os"}]	= "apple";
		#elif defined(__unix__)
		result[{"os"}]	= "unix";
		#endif
		#if defined(__x86_64__) || defined(_M_X64)
		result[{"arch"}]	= "x64";
		#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
		result[{"arch"}]	= "x86";
		#elif defined(__aarch64__) || defined(_M_ARM64)
		result[{"arch"}]	= "arm64";
		#else 
		result[{"arch"}]	= "unknown";
		#endif 
		return result;
	}

	constexpr bool Value::contains(Value::StringType const& key) const  {
		if (!isObject()) return false;
		return content.object->contains(key);
	}
}

CTL_NAMESPACE_END

#endif