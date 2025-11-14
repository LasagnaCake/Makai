#ifndef MAKAILIB_FILE_FLOW_H
#define MAKAILIB_FILE_FLOW_H

#include "../compat/ctl.hpp"
#include "get.hpp"

/// @brief FLOW-related facilities.
namespace Makai::FLOW {
	/// @brief Underlying FLOW structure type.
	using Value = Data::Value;

	/// @brief FLOW object type.
	using Object	= Value::ObjectType;
	/// @brief FLOW array type.
	using Array		= Value::ArrayType;
	/// @brief FLOW byte list type.
	using ByteList	= Value::ByteListType;
	/// @brief FLOW object entry type.
	using Entry		= Value::ObjectType::PairType;

	/// @brief Parses a FLOW string.
	/// @param data String to parse.
	/// @return String as FLOW value.
	/// @throw Error::FailedAction at FLOW parsing failures.
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

namespace MkFLOW = Makai::FLOW;

namespace Makai::File {
	/// @brief Loads a JSON file from disk.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	/// @throw Error::FailedAction on JSON parsing failures.
	inline FLOW::Value loadFLOW(String const& path)	{return FLOW::loadFile(path);	}
	/// @brief Loads a JSON file. Will try to load from attached archive. If that fails, tries to load from disk.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	/// @throw Error::FailedAction on JSON parsing failures.
	inline FLOW::Value getFLOW(String const& path)	{return FLOW::getFile(path);	}
}

#endif // MAKAILIB_FILE_JSON_H
