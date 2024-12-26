#ifndef MAKAILIB_FILE_JSON_H
#define MAKAILIB_FILE_JSON_H

#define JSON_NO_IO
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#include "_lib/nlohmann/json.hpp"
#pragma GCC diagnostic pop
#include "../compat/ctl.hpp"
#include "get.hpp"

/// @brief JSON-related facilities.
namespace Makai::JSON {
	/// @brief Third-party API facilites.
	namespace Extern {
		/// @brief Base JSON library.
		using Nlohmann = nlohmann::json;
		/// @brief Underlying JSON structure type.
		using JSONData = nlohmann::ordered_json;
	}

	/// @brief Underlying JSON structure type.
	using JSONType = Extern::JSONData;
	
	struct JSONValue;

	/// @brief JSON view.
	class JSONView: public View<Extern::JSONData> {
	public:
		/// @brief Constructs the JSON view.
		/// @param data Reference to JSON value to view.
		/// @param name JSON value name. By default, it is `"<anonymous>"`.
		JSONView(Extern::JSONData& data, String const& name = "<anonymous>");
		/// @brief Constructs the JSON view.
		/// @param data Const reference to JSON value to view.
		/// @param name JSON value name. By default, it is `"<anonymous>"`.
		JSONView(Extern::JSONData const& data, String const& name = "<anonymous>");
		/// @brief Copy constructor.
		/// @param other `JSONView` to copy from.
		JSONView(JSONView const& other);
		/// @brief Move constructor.
		/// @param other `JSONView` to move.
		JSONView(JSONView&& other);

		/// @brief Returns a copy of the underlying JSON value.
		Extern::JSONData json() const;

		/// @brief Returns the current value stored in the view.
		/// @tparam T Value type.
		/// @return Stored value.
		/// @throw Error::FailedAction if stored value is not of the given type.
		template<class T>
		inline T get() const {
			T result;
			if (!tryGet<T>(result))
				throw Error::FailedAction(
					"Parameter '" + name + "' is not of type '"
					+ TypeInfo<T>::name() + "'!",
					err,
					CTL_CPP_PRETTY_SOURCE
				);
			return result;
		}

		/// @brief Returns the current value stored in the view, or the fallback value.
		/// @tparam T Value type.
		/// @param fallback Fallback value.
		/// @return Stored value, or fallback.
		template<class T>
		inline T get(T const& fallback) const {
			T result;
			if (!tryGet<T>(result))
				return fallback;
			return result;
		}

		/// @brief JSON structure member access operator.
		/// @param key Member to access.
		/// @return View to member.
		/// @throw Error::InvalidAction if data is not a JSON object.
		JSONView operator[](String const& key);
		/// @brief JSON structure member access operator.
		/// @param key Member to access.
		/// @return Constant view to member.
		/// @throw Error::InvalidAction if data is not a JSON object.
		const JSONView operator[](String const& key) const;

		/// @brief JSON structure indexing operator.
		/// @param key Index to access.
		/// @return View to index.
		/// @throw Error::InvalidAction if data is not a JSON array.
		JSONView operator[](usize const index);
		/// @brief JSON structure indexing operator.
		/// @param key Index to access.
		/// @return Constant view to index.
		/// @throw Error::InvalidAction if data is not a JSON array.
		const JSONView operator[](usize const index) const;
		
		/// @brief Returns the member/element count of the underlying data.
		/// @return Member/element count.
		inline usize size() const {return view().size();}

		/// @brief JSON assignment operator.
		/// @param v Value to assign to underlying data.
		/// @return Reference to self.
		JSONView& operator=(JSONView const& v);

		/// @brief Value assignment operator.
		/// @tparam T Value type.
		/// @param value Value to assign to underlying data.
		/// @return Reference to self.
		template<class T>
		inline JSONView& operator=(T const& value)
		requires (!Makai::Type::Convertible<T, JSONView>) {
			view() = value;
			return (*this);
		}

		/// @brief Returns the current value stored in the view.
		/// @tparam T Value type.
		template<typename T> operator T() const {return get<T>();}

		/// @brief Returns the JSON value's name.
		/// @return Structure name.
		String getName() const;

		/// @brief Returns the underlying JSON value as a string.
		/// @param indent Indentation to apply. `-1` for no line breaks. By default, it is `-1`.
		/// @param ch Indent character. By default, it is `'\t'`.
		/// @return Structure as string.
		String toString(int const indent = -1, char const ch = '\t') const;

		/// @brief Returns whether the underlying JSON value has a given member.
		/// @param key Member to check for.
		/// @return Whether member exists.
		inline bool has(String const& key) const {return view().contains(key.stdView()); }

		/// @brief Returns a copy of the underlying JSON value.
		inline operator Extern::JSONData() {return view();}
		
		/// @brief Returns whether the JSON value is null.
		/// @return Whether it is null.
		bool isNull() const;
		/// @brief Returns whether the JSON value is an integer.
		/// @return Whether it is an integer.
		bool isInt() const;
		/// @brief Returns whether the JSON value is a floating point number.
		/// @return Whether it is a float.
		bool isFloat() const;
		/// @brief Returns whether the JSON value is a number.
		/// @return Whether it is a number.
		bool isNumber() const;
		/// @brief Returns whether the JSON value is a JSON object.
		/// @return Whether it is a JSON object.
		bool isObject() const;
		/// @brief Returns whether the JSON value is an unsigned integer.
		/// @return Whether it is an unsigned integer.
		bool isUnsigned() const;
		/// @brief Returns whether the JSON value is a JSON array.
		/// @return Whether it is a JSON array.
		bool isArray() const;
		/// @brief Returns whether the JSON value is a boolean value.
		/// @return Whether it is a boolean.
		bool isBool() const;
		/// @brief Returns whether the JSON value is a string.
		/// @return Whether it is a string.
		bool isString() const;
		/// @brief Returns whether the JSON value is a primitive valie.
		/// @return Whether it is primitive.
		bool isPrimitive() const;
		/// @brief Returns whether the JSON value is a structured value.
		/// @return Whether it is structured.
		bool isStructured() const;
		/// @brief Returns whether the JSON value is a discarded value.
		/// @return Whether it is discarded.
		bool isDiscarded() const;

		/// @brief Tries to get a value from the underlying JSON value.
		/// @tparam T Value type.
		/// @param out Output of the value.
		/// @return Whether the value was successfully acquired.
		template<Makai::Type::Primitive T>
		bool tryGet(T& out) const try {
			out = view().get<T>();
			err = "";
			return true;
		} catch (Extern::Nlohmann::exception const& e) {
			err = e.what();
			return false;
		}

		/// @brief Tries to get a value from the underlying JSON value.
		/// @tparam T Value type.
		/// @param out Output of the value.
		/// @return Whether the value was successfully acquired.
		template<Makai::Type::Enumerator T>
		bool tryGet(T& out) const try {
			out = view().get<T>();
			err = "";
			return true;
		} catch (Extern::Nlohmann::exception const& e) {
			err = e.what();
			return false;
		}

		/// @brief Tries to get a value from the underlying JSON value.
		/// @tparam T Value type.
		/// @param out Output of the value.
		/// @return Whether the value was successfully acquired.
		template <Makai::Type::Equal<String> T>
		bool tryGet(T& out) const try {
			out = view().get<std::string>();
			err = "";
			return true;
		} catch (Extern::Nlohmann::exception const& e) {
			err = e.what();
			return false;
		}

		/// @brief Tries to get a value from the underlying JSON value.
		/// @tparam T Value type.
		/// @param out Output of the value.
		/// @return Whether the value was successfully acquired.
		template <Makai::Type::Container::List T>
		bool tryGet(T& out) const
		requires (
			Makai::Type::Different<typename T::DataType, String>
		&&	Makai::Type::Different<typename T::DataType, JSONView>
		&&	Makai::Type::Different<typename T::DataType, JSONValue>
		) try {
			out = T(view().get<std::vector<typename T::DataType>>());
			err = "";
			return true;
		} catch (Extern::Nlohmann::exception const& e) {
			err = e.what();
			return false;
		}

		/// @brief Tries to get a value from the underlying JSON value.
		/// @tparam T Value type.
		/// @param out Output of the value.
		/// @return Whether the value was successfully acquired.
		template <Makai::Type::Container::List T>
		bool tryGet(T& out) const
		requires Makai::Type::Equal<typename T::DataType, String>
		try {
			out = T(view().get<std::vector<std::string>>());
			err = "";
			return true;
		} catch (Extern::Nlohmann::exception const& e) {
			err = e.what();
			return false;
		}

		/// @brief Returns the error that happened when `tryGet` was executed.
		/// @return Error.
		String error() {return err;}

	private:
		/// @brief Constant reference to JSON data.
		Extern::JSONData const&	cdata;
		/// @brief Dummy reference. Used if reference to JSON value is constant.
		Extern::JSONData		dummy;

		/// @brief Error that happened when `tryGet` was executed.
		mutable String err = "";

		/// @brief JSON value name.
		String const name;

		friend class JSONValue;
	};

	/// @brief JSON value.
	struct JSONValue: public JSONView {
		/// @brief Constructs the JSON value.
		/// @param name Value name. By default, it is `"<anonymous>"`.
		JSONValue(String const& name = "<anonymous>");

		/// @brief Constructs the JSON value.
		/// @param name Value name.
		/// @param data Value contents.
		JSONValue(String const& name, Extern::JSONData const& data);

		/// @brief Constructs the JSON value.
		/// @param data Value contents.
		JSONValue(Extern::JSONData const& data);

		/// @brief Constructs the JSON value.
		/// @param view Value contents.
		JSONValue(JSONView const& view);

		/// @brief Copy constructor.
		/// @param other `JSONValue` to copy from.
		JSONValue(JSONValue const& other);

		/// @brief Tries to get a value from the underlying JSON value.
		/// @tparam T Value type.
		/// @param out Output of the value.
		/// @return Whether the value was successfully acquired.
		template <Makai::Type::Container::List T>
		bool tryGet(T& out) const
		requires Makai::Type::Equal<typename T::DataType, JSONValue>
		try {
			out = T(view().get<std::vector<Extern::JSONData>>());
			err = "";
			return true;
		} catch (Extern::Nlohmann::exception const& e) {
			err = e.what();
			return false;
		}

		/// @brief Tries to get a value from the underlying JSON value.
		/// @tparam T Value type.
		/// @param out Output of the value.
		/// @return Whether the value was successfully acquired.
		template <Makai::Type::Container::SimpleMap T>
		bool tryGet(T& out) const
		requires (
			Makai::Type::Equal<typename T::KeyType, String>
		&&	Makai::Type::Equal<typename T::ValueType, JSONValue>
		) try {
			T res;
			for (auto [k, v]: view().items())
				res[k] = v;
			err = "";
			return true;
		} catch (Extern::Nlohmann::exception const& e) {
			err = e.what();
			return false;
		}

		/// @brief Tries to get a value from the underlying JSON value.
		/// @tparam T Value type.
		/// @param out Output of the value.
		/// @return Whether the value was successfully acquired.
		template <Makai::Type::Container::SimpleMap T>
		bool tryGet(T& out) const
		requires (
			Makai::Type::Equal<typename T::KeyType, String>
		&&	Makai::Type::Different<typename T::ValueType, JSONValue>
		) try {
			T res;
			for (auto [k, v]: view().items())
				res[k] = v.template get<typename T::ValueType>();
			err = "";
			return true;
		} catch (Extern::Nlohmann::exception const& e) {
			err = e.what();
			return false;
		}

		/// @brief Returns the current value stored in the view.
		/// @tparam T Value type.
		/// @return Stored value.
		/// @throw Error::FailedAction if stored value is not of the given type.
		template<class T> T get() const						{return JSONView::get<T>();			}
		/// @brief Returns the current value stored in the view, or the fallback value.
		/// @tparam T Value type.
		/// @param fallback Fallback value.
		/// @return Stored value, or fallback.
		template<class T> T get(T const& fallback) const	{return JSONView::get<T>(fallback);	}

		/// @brief Empties the underlying JSON value.
		/// @return Reference to self.
		JSONValue& clear();

		//inline operator JSONView() {return JSONView(data, getName());}

	private:
		/// @brief Underlying JSON value.
		Extern::JSONData data;
	};

	/// @brief Creates a JSON object.
	/// @param name Value name.
	/// @return JSON object.
	JSONValue object(String const& name = "<anonymous>");

	/// @brief Creates a JSON array.
	/// @param name Value name.
	/// @return JSON array.
	JSONValue array(String const& name = "<anonymous>");

	/// @brief Compatibility type analog.
	using JSONData = JSONValue;

	/// @brief Parses a JSON string.
	/// @param data String to parse.
	/// @return String as JSON value.
	/// @throw Error::FailedAction at JSON parsing failures.
	JSONData parse(String const& data);
	/// @brief Loads a JSON file from disk.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	/// @throw Error::FailedAction at JSON parsing failures.
	JSONData loadFile(String const& path);
	/// @brief Loads a JSON file. Will try to load from attached archive. If that fails, tries to load from disk.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	/// @throw Error::FailedAction at JSON parsing failures.
	JSONData getFile(String const& path);
}

namespace MkJSON = Makai::JSON;

namespace Makai::File {
	/// @brief Loads a JSON file from disk.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	/// @throw Error::FailedAction on JSON parsing failures.
	inline JSON::JSONData loadJSON(String const& path)	{return JSON::loadFile(path);	}
	/// @brief Loads a JSON file. Will try to load from attached archive. If that fails, tries to load from disk.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	/// @throw Error::FailedAction on JSON parsing failures.
	inline JSON::JSONData getJSON(String const& path)	{return JSON::getFile(path);	}
}

#endif // MAKAILIB_FILE_JSON_H
