#ifndef MAKAILIB_FILE_JSON_H
#define MAKAILIB_FILE_JSON_H

#include "../compat/ctl.hpp"
#include "get.hpp"

/// @brief JSON-related facilities.
namespace Makai::JSON {
	/// @brief Underlying JSON structure type.
	using Value = Data::Value;

	/// @brief JSON object type.
	using Object	= Value::ObjectType;
	/// @brief JSON array type.
	using Array		= Value::ArrayType;
	/// @brief JSON byte list type.
	using ByteList	= Value::ByteListType;
	/// @brief JSON object entry type.
	using Entry		= Value::ObjectType::PairType;

	/// @brief Parses a JSON string.
	/// @param data String to parse.
	/// @return String as JSON value.
	/// @throw Error::FailedAction at JSON parsing failures.
	Value parse(String const& data);
	/// @brief Loads a JSON file from disk.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	/// @throw Error::FailedAction at JSON parsing failures.
	Value loadFile(String const& path);
	/// @brief Loads a JSON file. Will try to load from attached archive. If that fails, tries to load from disk.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	/// @throw Error::FailedAction at JSON parsing failures.
	Value getFile(String const& path);
}

namespace MkJSON = Makai::JSON;

namespace Makai::File {
	/// @brief Loads a JSON file from disk.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	/// @throw Error::FailedAction on JSON parsing failures.
	inline JSON::Value loadJSON(String const& path)	{return JSON::loadFile(path);	}
	/// @brief Loads a JSON file. Will try to load from attached archive. If that fails, tries to load from disk.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	/// @throw Error::FailedAction on JSON parsing failures.
	inline JSON::Value getJSON(String const& path)	{return JSON::getFile(path);	}
}

#endif // MAKAILIB_FILE_JSON_H
